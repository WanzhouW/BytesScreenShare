#include "ScreenCaptureService.h"
#include "../encoder/VideoEncoder.h" 
#include "../network/RtcRtpSender.h" 
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>

ScreenCaptureService::ScreenCaptureService(QObject* parent)
    : QObject(parent)
{
    init();
}

ScreenCaptureService::~ScreenCaptureService()
{
    stopCapture();
    // Qt 的对象树机制(parent)会自动清理内存，
    // 但为了保险，手动停止一下更好
}

void ScreenCaptureService::init()
{
    // 1. 创建核心对象
    m_session = new QMediaCaptureSession(this);
    m_screenCapture = new QScreenCapture(this);
    m_previewWidget = new QVideoWidget(); // 注意：这个组件稍后要给 UI 组布局用

    // 2. 连接管道
    m_session->setScreenCapture(m_screenCapture);
    m_session->setVideoOutput(m_previewWidget);

    // 3. 默认选择主屏幕
    m_screenCapture->setScreen(QGuiApplication::primaryScreen());


    m_videoSink = new QVideoSink(this);
    m_session->setVideoOutput(m_videoSink); // 这会导致界面变黑，但数据有了

    // 连接信号：每当屏幕刷新，frameChanged 触发
    connect(m_videoSink, &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame& frame) {
        if (m_encoder && frame.isValid()) {
            // 把这一帧丢给编码器
            m_encoder->encode(frame);
        }
    });
}

void ScreenCaptureService::startCapture()
{
    // 如果编码器没准备好，打印警告
    if (!m_encoder) {
        qDebug() << "Warning: Encoder not initialized yet. Frames will be dropped.";
    }

    if (m_screenCapture) {
        m_screenCapture->start();
        qDebug() << "Screen Capture Started!";
        emit captureStateChanged(true);
    }
}

void ScreenCaptureService::stopCapture()
{
    if (m_screenCapture) {
        m_screenCapture->stop();
        qDebug() << "Screen Capture Stopped!";
        emit captureStateChanged(false);
    }
}

void ScreenCaptureService::initEncoder(const QString& targetIp)
{
    qDebug() << "Initializing Encoder for Target:" << targetIp;

    // 1. 初始化 WebRTC 发送端
    if (!m_rtcSender) {
        m_rtcSender = std::make_unique<RtcRtpSender>(this);

        // 关键：连接信号获取 SDP Offer
        //connect(m_rtcSender.get(), &RtcRtpSender::onLocalSdpReady, this, [](const QString& sdp) {
        //    qDebug() << "========================================";
        //    qDebug() << "Copy this SDP Offer to the Receiver:";
        //    qDebug().noquote() << sdp;
        //    qDebug() << "========================================";
        //    });

        //connect(m_rtcSender.get(), &RtcRtpSender::onIceCandidate, this, [](const QString& cand, const QString& mid) {
        //   qDebug() << "ICE Candidate:" << mid << cand;
            // 实际项目中这里应该通过信令发送给对端
    //        });

    //    // 开始 WebRTC 流程 (创建 PC -> 创建 DC -> CreateOffer)
    //    m_rtcSender->initConnection();

        //【新增】 连接 RTP 打包信号->本类的对外信号
        connect(m_rtcSender.get(), &RtcRtpSender::rtpPacketReady,
            this, &ScreenCaptureService::videoDataReady);
    }

    // 2. 初始化编码器
    if (!m_encoder) {
        m_encoder = new VideoEncoder(this);
        // 1080p, 30fps, 4Mbps (提高码率以保证画质)
        if (m_encoder->init(1920, 1080, 30, 4000000)) {
            qDebug() << "Video Encoder Initialized!";
        }
        else {
            qDebug() << "Encoder Init Failed!";
            return;
        }

        // 3. 绑定数据流：Encoder -> Sender
        m_encoder->onEncodedData = [this](const std::vector<uint8_t>& data, uint32_t ts) {
            if (m_rtcSender) {
                // 传入 NAL 数据和时间戳
                m_rtcSender->sendH264(data, ts);
            }
            };
    }
}

//bool ScreenCaptureService::setRemoteSdp(const QString& answerSdp)
//{
//    if (!m_rtcSender) {
//        qDebug() << "RtcRtpSender 未初始化，无法设置远端 SDP";
//        return false;
//    }
//    bool ok = m_rtcSender->setRemoteDescription(answerSdp.toStdString());
//    if (ok) {
//        qDebug() << "Remote SDP set (answer) 成功";
//    }
//    else {
//        qDebug() << QStringLiteral("Remote SDP set(answer) 失败 ");
//    }
//    return ok;
//}

QVideoWidget* ScreenCaptureService::getVideoPreviewWidget()
{
    return m_previewWidget;
}
