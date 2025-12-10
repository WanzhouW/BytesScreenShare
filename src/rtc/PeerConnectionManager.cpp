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
    // sendtest();
}

void PeerConnectionManager::createPeerConnection()
{
    rtc::Configuration config;

    m_pc = std::make_shared<rtc::PeerConnection>(config);

    // 1. Status monitoring
    m_pc->onStateChange([this](rtc::PeerConnection::State state) {
        QMetaObject::invokeMethod(this, [this, state]() {
            if (state == rtc::PeerConnection::State::Connected) {
                qDebug() << "P2P handshaking successfully!";
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
        qDebug("datachannel open successfully!");
        if(m_isCaller) sendtest();    
        QMetaObject::invokeMethod(this, [this]() {
            emit p2pConnected();
            if(m_isCaller) emit dataChannelOpened();
            });
        });

    m_videoChannel->onMessage([this](std::variant<rtc::binary, rtc::string> data) {
        // 文本消息
        if (std::holds_alternative<rtc::string>(data)) {
            auto &str = std::get<rtc::string>(data);
            qDebug() << "Callee received text:" << QString::fromStdString(str);

            // 比如这里你可?? emit 信号，通知 UI 显示消息
            return;
        }

        if (std::holds_alternative<rtc::binary>(data)) {
            auto& binData = std::get<rtc::binary>(data);

            QByteArray qData(reinterpret_cast<const char*>(binData.data()), binData.size());

            qDebug() << "received qData:" << qData.toHex(' ');

            // QMetaObject::invokeMethod(this, [this, qData]() {
            //     emit encodedFrameReceived(qData);
            //     });
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

            if (!m_pc) {   // ?? 只在没有 PC 时才创建一??
                createPeerConnection();
            }

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
    // m_ws->open(url.toStdString());

    // 在调用 open 的地方
    try {
        std::string targetUrl = url.toStdString();
        // 或者 targetUrl = url.toStdString();

        qDebug() << "Attempting to connect to:" << QString::fromStdString(targetUrl);

        m_ws->open(targetUrl);
    }
    catch (const std::invalid_argument& e) {
        // 【关键】看看这里打印了什么！
        qDebug() << "CRITICAL ERROR (Invalid Argument):" << e.what();
    }
    catch (const std::exception& e) {
        qDebug() << "CRITICAL ERROR (Exception):" << e.what();
    }
    catch (...) {
        qDebug() << "CRITICAL ERROR: Unknown exception occurred.";
    }
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
void PeerConnectionManager::sendtest(){
    if (m_videoChannel && m_videoChannel->isOpen()){
        qDebug("message send!");
        m_videoChannel->send("Hello from sender!");
    }
}

#include <vector>
#include <cstring> // for memcpy
#include <algorithm> // for std::min
#include <cstddef>   // for std::byte

// ... (前面的代码)

void PeerConnectionManager::sendEncodedFrame(const QByteArray& encodedData, uint32_t timestamp)
{
    // 1. 通道检查
    if (m_videoChannel && m_videoChannel->isOpen()) {

        // 2. 准备原始数据
        const uint8_t* nalData = reinterpret_cast<const uint8_t*>(encodedData.constData());
        size_t totalSize = encodedData.size();
        if (totalSize == 0) return;

        // H.264 头信息
        uint8_t nalHeader = nalData[0];
        uint8_t nalType = nalHeader & 0x1F;

        // 【更新】------------------------
        // RTP 切片逻辑

        //  情况 A: NALU单个包小，可以直接传 
        if (totalSize <= MAX_RTP_PAYLOAD_SIZE) {
            // 直接分配 libdatachannel 需要的 std::vector<std::byte>
            std::vector<std::byte> packet(12 + totalSize);

            // 但是赋值要uint8_t* 需要一个指针指向这段内存
            uint8_t* header = reinterpret_cast<uint8_t*>(packet.data());

            // RTP Header (12 bytes)
            header[0] = 0x80;
            header[1] = 0x80 | (payloadType_ & 0x7F); // Marker = 1
            header[2] = (m_sequenceNumber >> 8) & 0xFF;
            header[3] = m_sequenceNumber & 0xFF;
            m_sequenceNumber++;

            header[4] = (currentTimestamp_ >> 24) & 0xFF;
            header[5] = (currentTimestamp_ >> 16) & 0xFF;
            header[6] = (currentTimestamp_ >> 8) & 0xFF;
            header[7] = currentTimestamp_ & 0xFF;
            header[8] = (m_ssrc >> 24) & 0xFF;
            header[9] = (m_ssrc >> 16) & 0xFF;
            header[10] = (m_ssrc >> 8) & 0xFF;
            header[11] = m_ssrc & 0xFF;

            // Copy Payload
            std::memcpy(header + 12, nalData, totalSize);

            // 1. 转成 QByteArray (为了方便打印)
            QByteArray debugHex(reinterpret_cast<const char*>(packet.data()), packet.size());

            // 2. 打印日志 (Type, Size, Hex)     
            qDebug() << "original binData :" << debugHex.toHex(' ');
            qDebug("video data send!");

            // 【发送】
            try {
                m_videoChannel->send(packet);
            }
            catch (...) {
                qDebug() << "Send frame failed. Channel might be busy or closed.";
            }

            return;
        }

        // === 情况 B: NALU太大，切片 (FU-A) ===
        const uint8_t* payloadData = nalData + 1;
        size_t payloadSize = totalSize - 1;
        size_t offset = 0;

        while (offset < payloadSize) {
            size_t chunkSize = std::min(MAX_RTP_PAYLOAD_SIZE - 2, payloadSize - offset);
            bool isFirst = (offset == 0);
            bool isLast = (offset + chunkSize == payloadSize);

            // 直接分配 std::vector<std::byte>
            std::vector<std::byte> packet(12 + 2 + chunkSize);

            // 获取可写的 uint8_t 指针
            uint8_t* header = reinterpret_cast<uint8_t*>(packet.data());

            // RTP Header
            header[0] = 0x80;
            // 只有最后一片 Marker=1，其他为0
            header[1] = (isLast ? 0x80 : 0x00) | (payloadType_ & 0x7F);
            header[2] = (m_sequenceNumber >> 8) & 0xFF;
            header[3] = m_sequenceNumber & 0xFF;
            m_sequenceNumber++;
            header[4] = (currentTimestamp_ >> 24) & 0xFF;
            header[5] = (currentTimestamp_ >> 16) & 0xFF;
            header[6] = (currentTimestamp_ >> 8) & 0xFF;
            header[7] = currentTimestamp_ & 0xFF;

            header[8] = (m_ssrc >> 24) & 0xFF;
            header[9] = (m_ssrc >> 16) & 0xFF;
            header[10] = (m_ssrc >> 8) & 0xFF;
            header[11] = m_ssrc & 0xFF;

            // FU Indicator 
            header[12] = (nalHeader & 0xE0) | 28;

            // FU Header
            header[13] = nalType;
            if (isFirst) header[13] |= 0x80; // S bit
            if (isLast)  header[13] |= 0x40; // E bit

            // Copy Payload Chunk
            std::memcpy(header + 14, payloadData + offset, chunkSize);

            // 1. 转成 QByteArray (为了方便打印)
            QByteArray debugHex(reinterpret_cast<const char*>(packet.data()), packet.size());

            // 2. 打印日志 (Type, Size, Hex)     
            qDebug() << "original binData :" << debugHex.toHex(' ');
            qDebug("video data send!");

            // 【发送】
            try {
                m_videoChannel->send(packet);
            }
            catch (...) {
                qDebug() << "Send frame failed. Channel might be busy or closed.";
            }

            offset += chunkSize;
        }
    }else {
        // 如果 DataChannel 还没打开或已关闭，则丢弃数据
		qDebug() << "DataChannel not open. Dropping encoded frame.";
    }
        
}

void PeerConnectionManager::stop()
{
    // 1. 关闭 DataChannel
    if (m_videoChannel) {
        // 如果 DataChannel 仍处于打开状态，尝试关闭它
        if (m_videoChannel->isOpen()) {
            try {
                m_videoChannel->close();
                qDebug() << "DataChannel closed successfully.";
            } catch (const std::exception& e) {
                qWarning() << "Error closing DataChannel:" << e.what();
            }
        }
        // 清除共享指针引用
        m_videoChannel.reset();
    }
    
    // 2. 关闭 PeerConnection
    if (m_pc) {
        // 断开底层的 WebRTC 连接。
        // 这将触发 ICE 连接关闭，并在远端触发连接状态变化。
        try {
            m_pc->close(); 
            qDebug() << "PeerConnection closed successfully.";
        } catch (const std::exception& e) {
            qWarning() << "Error closing PeerConnection:" << e.what();
        }
        // 清除共享指针引用。由于 m_pc 是 shared_ptr，这会减少引用计数。
        // 如果没有其他引用，对象将被销毁，释放所有 WebRTC 内部资源。
        m_pc.reset();
    }
    
    // 3. 重置管理器状态
    m_targetPeerId.clear();
    m_isCaller = false;
    
    // 4. 清理 SignalingClient（可选）
    // 如果您希望在停止 P2P 后仍保持信令连接（以便再次呼叫或被呼叫），则可以跳过此步。
    // 如果您想完全退出会议，应该断开信令连接。
    /*
    if (m_ws) {
        m_ws->close();
        m_ws.reset();
        m_myId.clear();
    }
    */

    qDebug() << "PeerConnectionManager stopped and resources released.";
    
    // 5. 通知 UI P2P 已终止
    // 注意：您已经在 onStateChange 中处理了 Disconnected 状态的信号，
    // 但是手动调用 stop() 应该也发信号以确保 UI 响应。
    // emit p2pDisconnected(); // 如果您想区分是远端断开还是本地主动断开，可以定义一个新信号
}

//void PeerConnectionManager::sendH264(const std::vector<uint8_t>& nalData, uint32_t timestamp)
//{
//
//    currentTimestamp_ = timestamp; // 更新当前帧的时间戳
//
//    // H.264 NALU Header (1 byte)
//    // [F|NRI|Type]
//    uint8_t nalHeader = nalData[0];
//    uint8_t nalType = nalHeader & 0x1F;
//
//    // 情况 1: 单个 NAL 单元 (Single NAL Unit Packet)
//    // 如果数据足够小，直接发送
//    if (nalData.size() <= MAX_RTP_PAYLOAD_SIZE) {
//        sendRtpPacket(nalData, true); // Marker = 1 (一帧结束)
//        return;
//    }
//
//    // 情况 2: 分片单元 (FU-A)
//    // 如果数据太大，必须拆分
//    // RFC 6184 Section 5.8
//
//    // 跳过 NAL Header，只切分 Payload
//    const uint8_t* payloadData = nalData.data() + 1;
//    size_t payloadSize = nalData.size() - 1;
//    size_t offset = 0;
//
//    while (offset < payloadSize) {
//        // 计算当前分片大小
//        size_t chunkSize = std::min(MAX_RTP_PAYLOAD_SIZE - 2, payloadSize - offset);
//        // -2 是因为 FU-A 需要 2 个字节的头 (Indicator + Header)
//
//        bool isFirstPacket = (offset == 0);
//        bool isLastPacket = (offset + chunkSize == payloadSize);
//
//        std::vector<uint8_t> fuaPacket;
//        fuaPacket.reserve(chunkSize + 2);
//
//        // 1. FU Indicator Byte
//        // 格式: [F|NRI|Type]
//        // F, NRI 来自原始 NAL Header
//        // Type = 28 (FU-A)
//        uint8_t fuIndicator = (nalHeader & 0xE0) | 28;
//        fuaPacket.push_back(fuIndicator);
//
//        // 2. FU Header Byte
//        // 格式: [S|E|R|Type]
//        // S: Start bit, E: End bit, R: 0, Type: 原始 NAL Type
//        uint8_t fuHeader = nalType;
//        if (isFirstPacket) fuHeader |= 0x80; // Set S bit
//        if (isLastPacket)  fuHeader |= 0x40; // Set E bit
//        fuaPacket.push_back(fuHeader);
//
//        // 3. Payload
//        fuaPacket.insert(fuaPacket.end(), payloadData + offset, payloadData + offset + chunkSize);
//
//        // 发送分片
//        // 只有最后一个分片的 Marker 位才置 1
//        sendRtpPacket(fuaPacket, isLastPacket);
//
//        offset += chunkSize;
//    }
//}
//
//void PeerConnectionManager::sendRtpPacket(const std::vector<uint8_t>& payload, bool marker)
//{
//    // RTP Header 固定 12 字节
//    std::vector<uint8_t> packet;
//    packet.resize(12 + payload.size());
//
//    uint8_t* header = reinterpret_cast<uint8_t*>(packet.data());
//
//    // Byte 0: V=2, P=0, X=0, CC=0 -> 0x80
//    header[0] = 0x80;
//
//    // Byte 1: M (Marker), PT (Payload Type)
//    header[1] = (marker ? 0x80 : 0x00) | (payloadType_ & 0x7F);
//
//    // Byte 2-3: Sequence Number (Big Endian)
//    header[2] = (sequenceNumber_ >> 8) & 0xFF;
//    header[3] = sequenceNumber_ & 0xFF;
//    sequenceNumber_++;
//
//    // Byte 4-7: Timestamp (Big Endian)
//    header[4] = (currentTimestamp_ >> 24) & 0xFF;
//    header[5] = (currentTimestamp_ >> 16) & 0xFF;
//    header[6] = (currentTimestamp_ >> 8) & 0xFF;
//    header[7] = currentTimestamp_ & 0xFF;
//
//    // Byte 8-11: SSRC (Big Endian)
//    header[8] = (ssrc_ >> 24) & 0xFF;
//    header[9] = (ssrc_ >> 16) & 0xFF;
//    header[10] = (ssrc_ >> 8) & 0xFF;
//    header[11] = ssrc_ & 0xFF;
//
//    // 拷贝 Payload
//    std::memcpy(reinterpret_cast<uint8_t*>(packet.data()) + 12, payload.data(), payload.size());
//
//    // 通过 DataChannel 发送二进制
//    //if (dc_) {
//    //    try {
//    //        dc_->send(packet); // libdatachannel 接受 std::vector<std::byte>
//    //    }
//    //    catch (...) {
//    //        // 忽略发送错误，防止崩溃
//    //    }
//    //}
//    emit rtpPacketReady(packet);
//}

QString PeerConnectionManager::id() const
{
    return m_myId;
}

QString PeerConnectionManager::target() const
{
    return m_targetPeerId;
}
