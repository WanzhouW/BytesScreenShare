#include "PeerConnectionManager.hpp"
#include "../signaling/WsSignalingClient.hpp"
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

PeerConnectionManager::PeerConnectionManager(QObject* parent)
    : QObject(parent)
    , m_ws(nullptr)
    , m_pc(nullptr)
    , m_videoChannel(nullptr)
    , m_isCaller(false)
{}

PeerConnectionManager::~PeerConnectionManager()
{}

void PeerConnectionManager::start(const QString& targetId)
{
    m_targetPeerId = targetId;
    m_isCaller = true;
    createPeerConnection();
    setupDataChannel(); // the user who send the offer must establish DataChannel
}

void PeerConnectionManager::createPeerConnection()
{
    rtc::Configuration config;

    m_pc = std::make_shared<rtc::PeerConnection>(config);

    // 1. Status monitoring
    m_pc->onStateChange([this](rtc::PeerConnection::State state) {
        QMetaObject::invokeMethod(this, [this, state]() {
            if (state == rtc::PeerConnection::State::Connected) {
                INFO() << "P2P handshaking successfully!";
            }
            else if (state == rtc::PeerConnection::State::Disconnected ||
                state == rtc::PeerConnection::State::Failed) {
                emit p2pDisconnected();
            }
            });
        });

    // 2. ICE Exchange
    m_pc->onLocalCandidate([this](rtc::Candidate cand) {
        QJsonObject data;
        data["candidate"] = QString::fromStdString(cand.candidate());
        data["sdpMid"] = QString::fromStdString(cand.mid());
        sendSignalingMessage(stype_to_string(SignalingType::ICE), m_targetPeerId, data);
        });

    // 3. SDP Exchange (Generate Offer/Answer)
    m_pc->onLocalDescription([this](rtc::Description desc) {
        QJsonObject data;
        data["sdp"] = QString::fromStdString(desc);
        QString type = (desc.type() == rtc::Description::Type::Offer) ? 
            stype_to_string(SignalingType::OFFER) : stype_to_string(SignalingType::ANSWER);
        sendSignalingMessage(type, m_targetPeerId, data);
        });

    // 4. Peer bind DataChannel
    m_pc->onDataChannel([this](std::shared_ptr<rtc::DataChannel> dc) {
        // Recv the established channel
        if (dc->label() == "video-stream") {
            bindDataChannel(dc);
        }
        });
}

void PeerConnectionManager::setupDataChannel()
{
    if (!m_pc) {
        WARNING() << "Datachannel has already builed!";
    }

    rtc::DataChannelInit initConf;
    initConf.reliability.type = rtc::Reliability::Type::Rexmit;
    initConf.reliability.rexmit = 0;
    initConf.reliability.unordered = false;

    auto dc = m_pc->createDataChannel("video-stream", initConf);
    bindDataChannel(dc);
}

void PeerConnectionManager::bindDataChannel(std::shared_ptr<rtc::DataChannel> dc)
{
    m_videoChannel = dc;

    m_videoChannel->onOpen([this]() {
        QMetaObject::invokeMethod(this, [this]() {
            emit p2pConnected();
            });
        });

    m_videoChannel->onMessage([this](std::variant<rtc::binary, rtc::string> data) {
        if (std::holds_alternative<rtc::binary>(data)) {
            auto& binData = std::get<rtc::binary>(data);

            QByteArray qData(reinterpret_cast<const char*>(binData.data()), binData.size());

            QMetaObject::invokeMethod(this, [this, qData]() {
                emit encodedFrameReceived(qData);
                });
        }
        });
}

void PeerConnectionManager::handleSignalingMessage(const QJsonObject& json)
{
    SignalingType type = string_to_stype(json["type"].toString());
    QString from = json["from"].toString();
    QJsonObject data = json["data"].toObject();

    // make sure to process in main thread
    QMetaObject::invokeMethod(this, [=]() {
        if (type == SignalingType::REGISTER_SUCCESS) {
            m_myId = data["peerId"].toString();
            qDebug() << "My ID:" << m_myId;
            emit peersList(data["peers"].toArray());
        }
        else if (type == SignalingType::PEER_JOINED) {
            emit peerJoined(data["id"].toString());
        }
        else if (type == SignalingType::OFFER) {
            m_targetPeerId = from;
            m_isCaller = false;

            createPeerConnection(); // init PC

            // set remote Offer
            std::string sdp = data["sdp"].toString().toStdString();
            m_pc->setRemoteDescription(rtc::Description(sdp, rtc::Description::Type::Offer));

            // automatically generate Answer (libdatachannel when recv setRemoteDescription)
            // setLocalDescription will callback onLocalDescription to send Answer
            // Peer will recv DataChannel in signaling
        }
        else if (type == SignalingType::ANSWER) {
            std::string sdp = data["sdp"].toString().toStdString();
            m_pc->setRemoteDescription(rtc::Description(sdp, rtc::Description::Type::Answer));
        }
        else if (type == SignalingType::ICE) {
            std::string cand = data["candidate"].toString().toStdString();
            std::string mid = data["sdpMid"].toString().toStdString();
            m_pc->addRemoteCandidate(rtc::Candidate(cand, mid));
        }
        });
}

void PeerConnectionManager::sendSignalingMessage(const QString& type, const QString& to, const QJsonObject& data)
{
    if (m_ws && m_ws->readyState() == rtc::WebSocket::State::Open) {
        QJsonObject msg;
        msg["type"] = type;
        msg["from"] = m_myId;
        msg["to"] = to;
        msg["data"] = data;
        QJsonDocument doc(msg);
        m_ws->send(doc.toJson(QJsonDocument::Compact).toStdString());
    }
}

void PeerConnectionManager::onConnectServer(const QString& url)
{
    m_serverUrl = url;

    rtc::WebSocket::Configuration config;
    m_ws = std::make_shared<rtc::WebSocket>(config);

    m_ws->onOpen([this]() {
        QMetaObject::invokeMethod(this, [this]() {
            emit signalingConnected();
            registerClient();
            });
        });

    m_ws->onError([this](std::string s) {
        QMetaObject::invokeMethod(this, [this, s]() {
            emit signalingError(QString::fromStdString(s));
            });
        });

    m_ws->onMessage([this](std::variant<rtc::binary, rtc::string> data) {
        if (std::holds_alternative<rtc::string>(data)) {
            QString msg = QString::fromStdString(std::get<rtc::string>(data));
            QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
            if (!doc.isNull() && doc.isObject()) {
                handleSignalingMessage(doc.object());
            }
        }
        });

    QObject::connect(this, &PeerConnectionManager::peerJoined, this, &PeerConnectionManager::onJoined);
    m_ws->open(url.toStdString());
}

void PeerConnectionManager::onSignalingMessage(const QJsonObject& obj)
{
    const QString type = obj.value("type").toString();

    if (type == "offer" || type == "answer") {
        std::string sdp = obj.value("sdp").toString().toStdString();
        std::string stype = type.toStdString();
        rtc::Description desc(sdp, stype);
        m_pc->setRemoteDescription(desc);

        // 
        if (!m_isCaller && type == "offer") {
            m_pc->setLocalDescription();  // 
        } 
    }else if (type == "candidate") {
        std::string candidate = obj.value("candidate").toString().toStdString();
        std::string mid = obj.value("mid").toString().toStdString();
        // int mlineindex = obj.value("mlineindex").toInt();

        rtc::Candidate cand(candidate, mid);
        m_pc->addRemoteCandidate(cand);
    }
}

void PeerConnectionManager::onJoined(const QString& peerId)
{
    m_targetPeerId = peerId;
}


void PeerConnectionManager::registerClient()
{
    QJsonObject data;
    sendSignalingMessage("REGISTER_REQUEST", "Server", data);
}

void PeerConnectionManager::sendEncodedVideoFrame(const QByteArray& encodedData)
{
    // 仅通过数据通道发送视频帧
    if (m_videoChannel && m_videoChannel->isOpen()) {
        // 转换为 libdatachannel 需要的 std::byte 格式
        // 注意：DataChannel 默认 MTU 限制（通常 64KB - 头部）。
        // 如果你的图片很大（例如 4K 截图），这里会失败，需要分片。
        // 但对于 1080p 的中等质量 JPEG (通常 < 100KB)，这可能勉强能行，建议分片。
        // *简单方案*：控制 JPEG 质量，确保每帧小于 64KB。

        try {
            std::vector<std::byte> binData(encodedData.size());
            std::memcpy(binData.data(), encodedData.constData(), encodedData.size());

            m_videoChannel->send(binData);
        }
        catch (...) {
            DEBUG() << "Send frame failed. Channel might be busy or closed.";
        }
    }
}

QString PeerConnectionManager::id() const
{
    return m_myId;
}

QString PeerConnectionManager::target() const
{
    return m_targetPeerId;
}
