#pragma once

#include <QObject>
#include <memory>
#include <vector>
#include <rtc/rtc.hpp>

class RtcRtpSender : public QObject
{
    Q_OBJECT
public:
    explicit RtcRtpSender(QObject* parent = nullptr);
    ~RtcRtpSender();

    // 发送 H.264 NAL 单元 (内部会自动处理 RTP 封装和 FU-A 分片)
    // nalData: 不包含 StartCode (00 00 00 01) 的裸数据
    // timestamp: 90kHz 时钟的时间戳
    void sendH264(const std::vector<uint8_t>& nalData, uint32_t timestamp);

signals:
    //void onLocalSdpReady(const QString& sdp);
    //void onIceCandidate(const QString& candidate, const QString& mid);
    //void onDataChannelOpen(); //  明天一定要写完！
    //void onDataChannelClosed();
    void rtpPacketReady(const std::vector<uint8_t>& packet);

private:
    /*void ensurePeerConnection();
    void ensureDataChannel();*/

    // 底层 RTP 包发送函数
    void sendRtpPacket(const std::vector<uint8_t>& payload, bool marker);

private:
    std::shared_ptr<rtc::PeerConnection> pc_;
    std::shared_ptr<rtc::DataChannel> dc_;

    uint16_t sequenceNumber_ = 0;
    uint32_t ssrc_ = 0;
    uint32_t currentTimestamp_ = 0;

    const int payloadType_ = 96;
    const size_t MAX_RTP_PAYLOAD_SIZE = 1100; // 留出空间给 IP/UDP/RTP 头，保守设为 1100
};