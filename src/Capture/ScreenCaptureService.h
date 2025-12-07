#pragma once

#include <QObject>
#include <QVideoSink>
#include <QMediaCaptureSession>
#include <QScreenCapture>
#include <QVideoWidget>
#include <memory>  // for std::unique_ptr
#include "../encoder/VideoEncoder.h"
#include "../network/RtcRtpSender.h" 

class RtcRtpSender;

// 继承 QObject 是为了能使用信号槽机制
class ScreenCaptureService : public QObject
{
    Q_OBJECT // Qt 宏

public:
    explicit ScreenCaptureService(QObject* parent = nullptr);
    ~ScreenCaptureService();

    // 供 UI 调用的接口
    void startCapture();
    void stopCapture();
    void initEncoder(const QString& targetIp);  // 初始化编码器和 WebRTC 发送端

    // WebRTC 信令：设置对端返回的 SDP Answer
    /*bool setRemoteSdp(const QString& answerSdp);*/

    // UI窗口指针
    QVideoWidget* getVideoPreviewWidget();

signals:
    // 告诉外界：捕获状态变了 (可选)
    void captureStateChanged(bool isRunning);

    void videoDataReady(const std::vector<uint8_t>& data);

private:
    void init();

    QMediaCaptureSession* m_session = nullptr;
    QScreenCapture* m_screenCapture = nullptr;
    QVideoWidget* m_previewWidget = nullptr; // 这是一个用来预览的小窗口
    QVideoSink* m_videoSink = nullptr; // 用于提取帧
    VideoEncoder* m_encoder = nullptr; // 用于压缩帧

    // WebRTC RTP 发送器
    // 使用智能指针 (unique_ptr) 管理内存，避免手动 delete
    std::unique_ptr<RtcRtpSender> m_rtcSender;
};