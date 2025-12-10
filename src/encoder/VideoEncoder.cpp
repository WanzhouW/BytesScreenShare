#include "VideoEncoder.h"
#include <QDebug>
#include <libavutil/frame.h>

VideoEncoder::VideoEncoder(QObject* parent) : QObject(parent) {
    m_pkt = av_packet_alloc();
}

VideoEncoder::~VideoEncoder() {
    cleanup();
}

bool VideoEncoder::init(int width, int height, int fps, int bitrate) {
    m_targetW = width;
    m_targetH = height;

    // 1. 查找 H.264 编码器
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        qDebug() << "H.264 Encoder not found!";
        return false;
    }

    // 2. 配置上下文
    m_codecCtx = avcodec_alloc_context3(codec);
    m_codecCtx->bit_rate = bitrate;
    m_codecCtx->width = width;
    m_codecCtx->height = height;
    m_codecCtx->time_base = { 1, fps };
    m_codecCtx->framerate = { fps, 1 };

    m_codecCtx->gop_size = 0.5*fps; // 关键帧间隔
    m_codecCtx->max_b_frames = 0; // 实时流建议 0 B帧，降低延迟
    m_codecCtx->pix_fmt = AV_PIX_FMT_YUV420P; // H.264 标准输入格式

    // 3. 打开编码器 (preset=ultrafast 牺牲压缩率换取速度)
    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "preset", "ultrafast", 0);
    av_dict_set(&opts, "tune", "zerolatency", 0);

    // m_codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (avcodec_open2(m_codecCtx, codec, &opts) < 0) {
        qDebug() << "Could not open codec";
        return false;
    }

    // 4. 分配 YUV 帧内存
    m_frameYUV = av_frame_alloc();
    m_frameYUV->format = m_codecCtx->pix_fmt;
    m_frameYUV->width = m_codecCtx->width;
    m_frameYUV->height = m_codecCtx->height;
    av_frame_get_buffer(m_frameYUV, 32);

    return true;
}

void VideoEncoder::encode(const QVideoFrame& inputFrame) {
    if (!m_codecCtx) return;

    // A. 映射 Qt 帧到内存
    QVideoFrame cloneFrame = inputFrame;
    if (!cloneFrame.map(QVideoFrame::ReadOnly)) {
        qDebug() << "Map frame failed";
        return;
    }

    // B. 分辨率/格式转换 (SWS Scale)
    // 即使输入是 2K/4K，都会被这里强行缩放到 1920x1080 YUV420P
    if (!m_swsCtx || cloneFrame.width() != m_lastSrcW ||
        cloneFrame.height() != m_lastSrcH) {
        qDebug() << "Source resolution changed to" << cloneFrame.width() << "x" << cloneFrame.height() << "- Recreating SwsContext";

        // 如果旧的存在，先释放
        if (m_swsCtx) {
            sws_freeContext(m_swsCtx);
            m_swsCtx = nullptr;
        }

        // 更新记录
        m_lastSrcW = cloneFrame.width();
        m_lastSrcH = cloneFrame.height();

        
        // 这里简化假设输入是 RGB32 (AV_PIX_FMT_BGRA 或 RGBA)
        m_swsCtx = sws_getContext(
            cloneFrame.width(), cloneFrame.height(), AV_PIX_FMT_BGRA, // 输入
            m_targetW, m_targetH, AV_PIX_FMT_YUV420P,               // 输出
            SWS_BICUBIC, nullptr, nullptr, nullptr
        );
    }

    // 安全检查：如果上下文创建失败，不要继续，否则 sws_scale 会崩溃
    if (!m_swsCtx) {
        cloneFrame.unmap();
        return;
    }

    av_frame_make_writable(m_frameYUV);

    // 执行转换
    const uint8_t* srcData[4] = { cloneFrame.bits(0) };
    int srcLinesize[4] = { cloneFrame.bytesPerLine(0) };

    sws_scale(m_swsCtx, srcData, srcLinesize, 0, cloneFrame.height(),
        m_frameYUV->data, m_frameYUV->linesize);

    cloneFrame.unmap();

    // C. 发送给编码器
    m_frameYUV->pts = m_frameCount++; // 设置时间戳
    int ret = avcodec_send_frame(m_codecCtx, m_frameYUV);

    // D. 接收编码后的包
    while (ret >= 0) {
        ret = avcodec_receive_packet(m_codecCtx, m_pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
        else if (ret < 0) break;

        if (onEncodedData) {
            uint8_t* data = m_pkt->data;
            int size = m_pkt->size;

            // 解析 Annex-B 格式，提取 NALU
            // 我们需要找到 00 00 01 或 00 00 00 01 分隔符
            int curPos = 0;
            while (curPos < size) {
                // 寻找 start code
                int nalStart = -1;
                int prefixLen = 0;

                // 简单的 start code 查找逻辑
                // 注意：这里假设 FFmpeg 输出是标准的 Annex-B
                // 实际上 avcodec_receive_packet 出来的通常开头就是 Start Code

                // 如果这是包的开始，通常直接就是 Start Code
                if (curPos + 4 <= size && data[curPos] == 0 && data[curPos + 1] == 0 && data[curPos + 2] == 0 && data[curPos + 3] == 1) {
                    nalStart = curPos + 4;
                    prefixLen = 4;
                }
                else if (curPos + 3 <= size && data[curPos] == 0 && data[curPos + 1] == 0 && data[curPos + 2] == 1) {
                    nalStart = curPos + 3;
                    prefixLen = 3;
                }

                if (nalStart == -1) {
                    // 找不到 start code，可能这本身就是裸数据（不太可能），或者解析结束
                    break;
                }

                // 寻找下一个 start code 来确定当前 NALU 结束位置
                int nextNalStart = size; // 默认为包尾
                for (int i = nalStart; i < size - 3; ++i) {
                    if (data[i] == 0 && data[i + 1] == 0 && data[i + 2] == 1) {
                        nextNalStart = i; // 00 00 01
                        // 如果前面还有个 0，那就是 00 00 00 01，回退一位
                        if (i > 0 && data[i - 1] == 0) nextNalStart = i - 1;
                        break;
                    }
                }

                int nalSize = nextNalStart - nalStart;
                if (nalSize > 0) {
                    std::vector<uint8_t> nalBuffer(data + nalStart, data + nalStart + nalSize);

                    // 计算 PTS (Presentation Time Stamp) 对应的 90kHz 时间戳
                    // ffmpeg 的 pts 通常基于 time_base (我们设的是 1/30)
                    // RTP 需要 90000Hz。
                    
                    uint32_t rtpTimestamp = 0;
                    if (m_pkt->pts != AV_NOPTS_VALUE) {
                        // 设置 time_base = {1, fps}
                        // 所以 pts 就是帧数 0, 1, 2...
                        // 90kHz 下每帧间隔 = 90000 / fps
                        // 假设 fps=30 -> 3000
                        rtpTimestamp = static_cast<uint32_t>(m_pkt->pts * (90000 / 30));
                    }

                    // 回调出去：裸 NALU 数据 + 时间戳
                    onEncodedData(nalBuffer, rtpTimestamp);
                }

                curPos = nextNalStart;
            }
        }
        av_packet_unref(m_pkt);
    }
}

void VideoEncoder::cleanup() {
    if (m_codecCtx) {
        avcodec_free_context(&m_codecCtx);
        m_codecCtx = nullptr;
    }
    // 正确的释放帧内存方式
    if (m_frameYUV) {
        av_frame_free(&m_frameYUV); // 传入二级指针
        m_frameYUV = nullptr;
    }
    if (m_swsCtx) {
        sws_freeContext(m_swsCtx);
        m_swsCtx = nullptr;
    }
    if (m_pkt) {
        av_packet_free(&m_pkt);
        m_pkt = nullptr;
    }
}