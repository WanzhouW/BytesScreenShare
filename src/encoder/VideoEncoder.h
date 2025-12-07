#pragma once
#include <QObject>
#include <QVideoFrame>
#include <functional>

// FFmpeg 是 C 语言库
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class VideoEncoder : public QObject
{
    Q_OBJECT
public:
    explicit VideoEncoder(QObject* parent = nullptr);
    ~VideoEncoder();

    // 初始化编码器 (编码目标为 1080p， 在ScreenCapture中写了)
    bool init(int width, int height, int fps, int bitrate);

    // 编码一帧 Qt 的画面
    void encode(const QVideoFrame& frame);

    // 回调函数：编码好的 H.264 数据通过这里传出去
    std::function<void(const std::vector<uint8_t>&, uint32_t)> onEncodedData;

private:
    // 资源释放
    void cleanup();

    AVCodecContext* m_codecCtx = nullptr;
    AVFrame* m_frameYUV = nullptr;     // 存放转换后的 YUV 数据
    SwsContext* m_swsCtx = nullptr;    // 用于图像缩放和格式转换
    AVPacket* m_pkt = nullptr;

    int m_targetW = 1920; // 统一为1080p的分辨率，减少网络压力和延时
    int m_targetH = 1080;
    int m_frameCount = 0;

    int m_lastSrcW = -1;// 记录上一次输入的源分辨率，用于检测变化
    int m_lastSrcH = -1;
};