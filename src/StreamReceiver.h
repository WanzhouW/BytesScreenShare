#pragma once
#ifndef STREAMRECEIVER_H
#define STREAMRECEIVER_H

#include <QObject>
#include<QByteArray>
#include<QMutex>
#include<memory>
#include<vector>
#include<string>

#include<rtc/rtc.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include<libavutil/frame.h>
#include<libavutil/imgutils.h>
#include<libswscale/swscale.h>
}

//StreamReceiver类别用于：
//	1.作为接收端建立WebRTC连接。
//	2.接受远端的视屏流数据。
//	3.使用FFmpeg对于视频数据解码。
//	4.将解码后的YUV数据通过信号抛给UI层
class StreamReceiver : public QObject {
	Q_OBJECT

public:

	explicit StreamReceiver(QObject* parent = nullptr);
	~StreamReceiver();
	//启动接收流程：signalingUrl:信令服务器地址  myId:本机ID  peerId:发送端的ID(用于请求流)
	void start(const std::string& signalingUrl,const std::string& myId,const std::string& peerId);
	//停止接收并断开连接
	void stop();

signals:
	//视频帧解码成功信号（将解码后的YUV420P数据发给UI线程）
	//y,u,v:YUV420P数据；yStride,uStride,vStride:YUV420P数据的平面跨度；width,height:视频宽高
	void frameReady(const uchar* y,const uchar* u,const uchar* v,int yStride,int uStride,int vStride,int width,int height);

	//通知UI连接状态（"Connecting","Connected","Disconnected","Error"）
	void stateChange(const QString& state);

private:
	//WebRTC核心对象
	std::shared_ptr<rtc::PeerConnection> m_pc;
	std::shared_ptr<rtc::WebSocket> m_ws;

	//FFmpeg解码核心对象
	AVCodecContext* m_codecCtx=nullptr;//解码器的上下文
	AVFrame* m_frame = nullptr;//解码后的原始帧
	AVPacket* m_packet = nullptr;//接收到的编码包
	const AVCodec* m_codec = nullptr;//指定编解码器
	//解析器上下文（用于处理H264的粘包/拆包问题，也就是对于NALU单元进行拆分或者合并得到完整的一个NALU数据）
	AVCodecParserContext* m_parser = nullptr;

	//内部状态
	bool m_isDecoderInited = false;
	std::string m_myId;
	std::string m_peerId;

	//初始化FFmpeg解码器
	bool initDecoder(AVCodecID id);
	//释放FFmpeg解码器
	void CleanupDecoder();
	//处理从WebRTC接收到的二进制数据
	void onTrackData(const std::vector<std::byte>& data);
	//设置信令WebSocket
	void setupSignaling(const std::string& url);
};

#endif //STREAMRECEIVER_H