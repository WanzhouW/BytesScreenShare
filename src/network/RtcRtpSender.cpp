#include "RtcRtpSender.h"
#include <QDebug>
#include <random>
#include <cstring> // for memcpy

RtcRtpSender::RtcRtpSender(QObject* parent)
    : QObject(parent)
{
    // 生成随机 SSRC
    std::random_device rd;
    ssrc_ = rd();
}

RtcRtpSender::~RtcRtpSender()
{
    //if (dc_) dc_->close();
    //if (pc_) pc_->close();  
	// 现在无需要手动释放智能指针
}

/*因此处不需要考虑信令，以下不再需要*/
//void RtcRtpSender::initConnection()
//{
//    // 触发创建流程
//    ensurePeerConnection();
//    ensureDataChannel();
//
//    // 触发 Offer 创建 (libdatachannel 异步回调 onLocalDescription)
//    if (pc_) {
//        try {
//            pc_->setLocalDescription();
//        }
//        catch (const std::exception& e) {
//            qDebug() << "setLocalDescription failed:" << e.what();
//        }
//    }
//}
//
//void RtcRtpSender::ensurePeerConnection()
//{
//    if (pc_) return;
//
//    rtc::Configuration config;
//    config.iceServers.emplace_back("stun:stun.l.google.com:19302"); // Google STUN
//
//    pc_ = std::make_shared<rtc::PeerConnection>(config);
//
//    pc_->onStateChange([](rtc::PeerConnection::State state) {
//        qDebug() << "PC State:" << static_cast<int>(state);
//        });
//
//    pc_->onLocalCandidate([this](rtc::Candidate candidate) {
//        emit onIceCandidate(
//            QString::fromStdString(candidate.candidate()),
//            QString::fromStdString(candidate.mid())
//        );
//        });
//
//    pc_->onLocalDescription([this](rtc::Description description) {
//        std::string sdp = std::string(description);
//        qDebug() << "Local SDP Generated (Offer).";
//        emit onLocalSdpReady(QString::fromStdString(sdp));
//        });
//}
//
//void RtcRtpSender::ensureDataChannel()
//{
//    if (dc_) return;
//    ensurePeerConnection();
//
//    // 视频传输不需要重传，也不需要有序（因为有 RTP 序号），这样能降低延迟
//    rtc::DataChannelInit config;
//    config.reliability.type = rtc::Reliability::Type::Rexmit; // 限制重传
//    config.reliability.unordered = true; // 允许乱序 (我们有 RTP 序号，自己能排)
//    config.reliability.maxRetransmits = 0; // 视频流通常不重传，丢了就请求关键帧
//    // config.reliability.maxRetransmits = 2;
//
//    dc_ = pc_->createDataChannel("video-stream", config);
//
//    dc_->onOpen([this]() {
//        qDebug() << "DataChannel OPEN!";
//        emit onDataChannelOpen();
//        });
//
//    dc_->onClosed([this]() { emit onDataChannelClosed(); });
//}

/*既然此处不需要考虑信令，这些也不再需要*/

// -------------------------------------------------------------------------
//  RFC 6184 H.264 RTP 封装
// -------------------------------------------------------------------------
void RtcRtpSender::sendH264(const std::vector<uint8_t>& nalData, uint32_t timestamp)
{

    currentTimestamp_ = timestamp; // 更新当前帧的时间戳

    // H.264 NALU Header (1 byte)
    // [F|NRI|Type]
    uint8_t nalHeader = nalData[0];
    uint8_t nalType = nalHeader & 0x1F;

    // 情况 1: 单个 NAL 单元 (Single NAL Unit Packet)
    // 如果数据足够小，直接发送
    if (nalData.size() <= MAX_RTP_PAYLOAD_SIZE) {
        RtcRtpSender::sendRtpPacket(nalData, true); // Marker = 1 (一帧结束)
        return;
    }

    // 情况 2: 分片单元 (FU-A)
    // 如果数据太大，必须拆分
    // RFC 6184 Section 5.8

    // 跳过 NAL Header，只切分 Payload
    const uint8_t* payloadData = nalData.data() + 1;
    size_t payloadSize = nalData.size() - 1;
    size_t offset = 0;

    while (offset < payloadSize) {
        // 计算当前分片大小
        size_t chunkSize = std::min(MAX_RTP_PAYLOAD_SIZE - 2, payloadSize - offset);
        // -2 是因为 FU-A 需要 2 个字节的头 (Indicator + Header)

        bool isFirstPacket = (offset == 0);
        bool isLastPacket = (offset + chunkSize == payloadSize);

        std::vector<uint8_t> fuaPacket;
        fuaPacket.reserve(chunkSize + 2);

        // 1. FU Indicator Byte
        // 格式: [F|NRI|Type]
        // F, NRI 来自原始 NAL Header
        // Type = 28 (FU-A)
        uint8_t fuIndicator = (nalHeader & 0xE0) | 28;
        fuaPacket.push_back(fuIndicator);

        // 2. FU Header Byte
        // 格式: [S|E|R|Type]
        // S: Start bit, E: End bit, R: 0, Type: 原始 NAL Type
        uint8_t fuHeader = nalType;
        if (isFirstPacket) fuHeader |= 0x80; // Set S bit
        if (isLastPacket)  fuHeader |= 0x40; // Set E bit
        fuaPacket.push_back(fuHeader);

        // 3. Payload
        fuaPacket.insert(fuaPacket.end(), payloadData + offset, payloadData + offset + chunkSize);

        // 发送分片
        // 只有最后一个分片的 Marker 位才置 1
        sendRtpPacket(fuaPacket, isLastPacket);

        offset += chunkSize;
    }
}

void RtcRtpSender::sendRtpPacket(const std::vector<uint8_t>& payload, bool marker)
{
    // RTP Header 固定 12 字节
    std::vector<uint8_t> packet;
    packet.resize(12 + payload.size());

    uint8_t* header = reinterpret_cast<uint8_t*>(packet.data());

    // Byte 0: V=2, P=0, X=0, CC=0 -> 0x80
    header[0] = 0x80;

    // Byte 1: M (Marker), PT (Payload Type)
    header[1] = (marker ? 0x80 : 0x00) | (payloadType_ & 0x7F);

    // Byte 2-3: Sequence Number (Big Endian)
    header[2] = (sequenceNumber_ >> 8) & 0xFF;
    header[3] = sequenceNumber_ & 0xFF;
    sequenceNumber_++;

    // Byte 4-7: Timestamp (Big Endian)
    header[4] = (currentTimestamp_ >> 24) & 0xFF;
    header[5] = (currentTimestamp_ >> 16) & 0xFF;
    header[6] = (currentTimestamp_ >> 8) & 0xFF;
    header[7] = currentTimestamp_ & 0xFF;

    // Byte 8-11: SSRC (Big Endian)
    header[8] = (ssrc_ >> 24) & 0xFF;
    header[9] = (ssrc_ >> 16) & 0xFF;
    header[10] = (ssrc_ >> 8) & 0xFF;
    header[11] = ssrc_ & 0xFF;

    // 拷贝 Payload
    std::memcpy(reinterpret_cast<uint8_t*>(packet.data()) + 12, payload.data(), payload.size());

    // 通过 DataChannel 发送二进制
    //if (dc_) {
    //    try {
    //        dc_->send(packet); // libdatachannel 接受 std::vector<std::byte>
    //    }
    //    catch (...) {
    //        // 忽略发送错误，防止崩溃
    //    }
    //}
    emit rtpPacketReady(packet);
}