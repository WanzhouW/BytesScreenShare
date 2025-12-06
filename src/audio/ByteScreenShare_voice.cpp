#pragma execution_character_set("utf-8")
#include "ByteScreenShare_voice.h"
#include <QJsonDocument>
#include <QJsonObject>

VoiceChat::VoiceChat(QObject* parent) : QObject(parent)
{
    // 配置 WebRTC 日志（可选）
    rtc::InitLogger(rtc::LogLevel::Debug);

}

VoiceChat::~VoiceChat()
{
    if (audioInput) audioInput->stop();
    if (audioOutput) audioOutput->stop();
    if (dc) dc->close();
    if (pc) pc->close();
}

// 调试用：包装一下 setLocalDescription，看是谁在调用
static void LogAndSetLocalDescription(rtc::PeerConnection& pc, const char* who)
{
    qDebug() << "[DEBUG] setLocalDescription called by" << who;
    pc.setLocalDescription();
}

//void VoiceChat::initAudio()
//{
//    // 1. 先打印一下默认输入设备
//    auto inputDev = QMediaDevices::defaultAudioInput();
//    qDebug() << "Input Device:" << inputDev.description();
//
//    QAudioFormat wanted;
//    // 先设置一个常见的格式
//    wanted.setSampleRate(44100);
//    wanted.setChannelCount(1);
//    wanted.setSampleFormat(QAudioFormat::Int16);
//
//    // 2. 让 Qt 检查是否支持，不支持就退而求其次用设备默认格式
//    QAudioFormat formatToUse = wanted;
//    if (!inputDev.isFormatSupported(wanted)) {
//        qWarning() << "Wanted format not supported, using nearest/default format.";
//        formatToUse = inputDev.preferredFormat(); // 设备推荐格式
//    }
//
//    format = formatToUse; // 保存下来，输出也用同样的格式
//
//    // 3. 初始化输入 (麦克风)
//    audioInput = new QAudioSource(inputDev, format, this);
//    inputDevice = audioInput->start();
//    connect(inputDevice, &QIODevice::readyRead, this, &VoiceChat::onAudioRecorded);
//
//    // 4. 初始化输出 (扬声器)
//    auto outputDev = QMediaDevices::defaultAudioOutput();
//    audioOutput = new QAudioSink(outputDev, format, this);
//    audioOutput->setBufferSize(format.sampleRate() * format.bytesPerSample()
//        * format.channelCount() * 0.2); // 200ms 缓冲
//    outputDevice = audioOutput->start();
//
//    qDebug() << "Audio initialized. sampleRate =" << format.sampleRate()
//        << "channels =" << format.channelCount()
//        << "bytesPerSample =" << format.bytesPerSample();
//}
void VoiceChat::initAudio()
{
    auto inputDev = QMediaDevices::defaultAudioInput();
    qDebug() << "Input Device:" << inputDev.description();

    // 目标统一格式：48k, mono, Int16
    QAudioFormat wanted;
    wanted.setSampleRate(48000);
    wanted.setChannelCount(2);
    wanted.setSampleFormat(QAudioFormat::Int16);

    if (!inputDev.isFormatSupported(wanted)) {
        qWarning() << "Input device does NOT support 48k/mono/Int16";
        auto pref = inputDev.preferredFormat();
        qDebug() << "preferred input format: rate=" << pref.sampleRate()
            << "channels=" << pref.channelCount()
            << "bytesPerSample=" << pref.bytesPerSample()
            << "sampleFormat=" << int(pref.sampleFormat());
        return;
    }

    format = wanted;   // 采集和播放都用这个

    // 输入
    audioInput = new QAudioSource(inputDev, format, this);
    inputDevice = audioInput->start();
    connect(inputDevice, &QIODevice::readyRead, this, &VoiceChat::onAudioRecorded);

    // 输出（同样 48k/mono/Int16）
    auto outputDev = QMediaDevices::defaultAudioOutput();
    if (!outputDev.isFormatSupported(format)) {
        qWarning() << "Output device does NOT support 48k/mono/Int16";
        auto pref = outputDev.preferredFormat();
        qDebug() << "preferred output format: rate=" << pref.sampleRate()
            << "channels=" << pref.channelCount()
            << "bytesPerSample=" << pref.bytesPerSample()
            << "sampleFormat=" << int(pref.sampleFormat());
        return;
    }

    audioOutput = new QAudioSink(outputDev, format, this);
    audioOutput->setBufferSize(format.sampleRate() * format.bytesPerSample()
        * format.channelCount() * 0.2);
    outputDevice = audioOutput->start();

    qDebug() << "Audio initialized. sampleRate =" << format.sampleRate()
        << "channels =" << format.channelCount()
        << "bytesPerSample =" << format.bytesPerSample()
        << "sampleFormat =" << int(format.sampleFormat());
}


void VoiceChat::setupPeerConnection()
{
    if (pc) {
        qDebug() << "[setupPeerConnection] pc already exists, skip creating new one.";
        return;
    }

    rtc::Configuration config;

    // 加一个公共 STUN（先用最常见的）
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");

    pc = std::make_shared<rtc::PeerConnection>(config);
    qDebug() << "[setupPeerConnection] this=" << this << " pc=" << pc.get();

    // 1) 本地 SDP（Offer 或 Answer）
    pc->onLocalDescription([this](rtc::Description description) {
        QString sdp = QString::fromStdString(std::string(description));
        int     type = static_cast<int>(description.type());  // 先拷一份出来

        QMetaObject::invokeMethod(
            this,
            [this, sdp, type]() {
                qDebug() << "[LocalDescription] type =" << type
                    << "len =" << sdp.size();
                emit localDescriptionGenerated(sdp, type);
            },
            Qt::QueuedConnection
        );
        });

    // 2) 本地 ICE Candidate
    pc->onLocalCandidate([this](auto candidate) {
        QString cand = QString::fromStdString(candidate.candidate());
        QString mid = QString::fromStdString(candidate.mid());

        qDebug() << "[LOCAL CANDIDATE]"
            << "cand =" << cand
            << "mid  =" << mid;

        // 打包成 JSON，包含 candidate + mid
        QJsonObject obj;
        obj["candidate"] = cand;
        obj["mid"] = mid;
        QJsonDocument doc(obj);
        QString json = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

        QMetaObject::invokeMethod(
            this,
            [this, json]() { emit localCandidateGenerated(json); },
            Qt::QueuedConnection
        );
        });

    // 3) 连接状态
    pc->onStateChange([this](rtc::PeerConnection::State state) {
        qDebug() << "PeerConnection State:" << int(state);
        if (state == rtc::PeerConnection::State::Connected) {
            QMetaObject::invokeMethod(
                this,
                [this]() { emit connectionStateChanged("Connected"); },
                Qt::QueuedConnection
            );
        }
        });

    // 4) DataChannel（被叫侧）
    pc->onDataChannel([this](std::shared_ptr<rtc::DataChannel> incomingDc) {
        dc = incomingDc;
        QMetaObject::invokeMethod(
            this,
            [this]() { setupDataChannel(); },
            Qt::QueuedConnection
        );
        });
}

void VoiceChat::setupDataChannel()
{
    if (!dc) return;

    dc->onOpen([this]() {
        qDebug() << "DataChannel OPEN";
        emit connectionStateChanged("DataChannel Open");
        });

    dc->onMessage([this](auto data) {
        // 接收到对方发来的音频数据 (二进制)
        if (std::holds_alternative<rtc::binary>(data)) {
            auto bytes = std::get<rtc::binary>(data);
            //qDebug() << "Received Bytes:" << bytes.size();
            if (outputDevice && audioOutput->state() == QAudio::ActiveState) {
                // 将接收到的 PCM 数据写入音频输出设备
                outputDevice->write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
            }
            else {
                qDebug() << "[AudioRecv] output not ready, state="
                    << (audioOutput ? audioOutput->state() : -1);
            }
        }
        });
}

void VoiceChat::onAudioRecorded()
{
    if (!inputDevice) return;

    QByteArray data = inputDevice->readAll();
    if (data.isEmpty()) return;

    // 调试
    //qDebug() << "Mic Data:" << data.size();
    // 本地回环测试
    /*if (outputDevice && audioOutput->state() == QAudio::ActiveState) {
        outputDevice->write(data);
    }*/

    // 拷贝一份到堆上，避免跨线程悬空
    auto sharedData = std::make_shared<QByteArray>(std::move(data));

    QMetaObject::invokeMethod(
        this,
        [this, sharedData]() {
            if (!dc || !dc->isOpen()) return;
            if (sharedData->isEmpty()) return;

            rtc::binary bin;
            bin.resize(sharedData->size());
            std::memcpy(bin.data(), sharedData->constData(), sharedData->size());
            dc->send(bin);
        },
        Qt::QueuedConnection
    );
}






// --- 信令流程 (P2P 握手) ---

void VoiceChat::startAsOfferer()
{
    setupPeerConnection();
    dc = pc->createDataChannel("audio_channel");
    QMetaObject::invokeMethod(
        this,
        [this]() { setupDataChannel(); },
        Qt::QueuedConnection
    );
}

void VoiceChat::startAsAnswerer()
{
    setupPeerConnection();
    // 接收方不需要 createDataChannel，等待 pc->onDataChannel 触发即可
}

void VoiceChat::setRemoteAnswer(const QString& sdp)
{
    if (!pc) {
        qDebug() << "[Alice] setRemoteAnswer: pc is null";
        return;
    }
    qDebug() << "[Alice] setRemoteAnswer: len=" << sdp.size();
    try {
        rtc::Description desc(sdp.toStdString(), "answer");
        qDebug() << "[Alice] setRemoteAnswer: created Description";
        pc->setRemoteDescription(desc);
        qDebug() << "[Alice] setRemoteAnswer: setRemoteDescription OK";
    }
    catch (const std::exception& e) {
        qDebug() << "[Alice] setRemoteAnswer exception:" << e.what();
    }
}

void VoiceChat::setRemoteOffer(const QString& sdp)
{
    if (!pc) {
        qDebug() << "[Bob] setRemoteOffer: pc is null";
        return;
    }
    qDebug() << "[Bob] setRemoteOffer: len=" << sdp.size();
    try {
        rtc::Description desc(sdp.toStdString(), "offer");
        qDebug() << "[Bob] setRemoteOffer: created Description";
        pc->setRemoteDescription(desc);
        qDebug() << "[Bob] setRemoteOffer: setRemoteDescription OK";
        pc->setLocalDescription();  // 生成 Answer
        qDebug() << "[Bob] setRemoteOffer: setLocalDescription OK";
    }
    catch (const std::exception& e) {
        qDebug() << "[Bob] setRemoteOffer exception:" << e.what();
    }
}


void VoiceChat::addRemoteCandidate(const QString& candidateJson)
{
    if (!pc) {
        qDebug() << "[WARN] addRemoteCandidate: pc == nullptr";
        return;
    }

    auto remoteDescOpt = pc->remoteDescription();
    if (!remoteDescOpt.has_value()) {
        qDebug() << "[WARN] addRemoteCandidate: remoteDescription not set yet";
        return;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(candidateJson.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qDebug() << "[ERROR] addRemoteCandidate: invalid JSON" << err.errorString();
        return;
    }

    QJsonObject obj = doc.object();
    QString cand = obj.value("candidate").toString();
    QString mid = obj.value("mid").toString();

    if (cand.isEmpty()) {
        qDebug() << "[ERROR] addRemoteCandidate: empty candidate string";
        return;
    }

    try {
        // 第二个参数是 mid，可以为空字符串
        rtc::Candidate c(cand.toStdString(), mid.toStdString());
        pc->addRemoteCandidate(c);
        qDebug() << "[INFO] addRemoteCandidate: added cand, mid =" << mid;
    }
    catch (const std::exception& e) {
        qDebug() << "[ERROR] addRemoteCandidate exception:" << e.what();
    }
}

