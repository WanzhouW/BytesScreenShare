# include "StreamReceiver.h"
#include<QDebug>
#include<QJsonDocument>
#include<QJsonObject>

//辅助宏:输出FFMPEG返回的错误信息
#define FFMPEG_CHECK(ret,msg)\
	if (ret < 0) {\
		char errbuf[AV_ERROR_MAX_STRING_SIZE];\
		av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);\
		qCritical() << msg << ":" << errbuf;\
		return;\
	}

StreamReceiver::StreamReceiver(QObject * parent):QObject(parent) {
	//预先分配AVPacket以及AVFrame
	m_packet = av_packet_alloc();
	m_frame = av_frame_alloc();
}

StreamReceiver::~StreamReceiver() {
	stop();
	if (m_packet) av_packet_free(&m_packet);
    if (m_frame) av_frame_free(&m_frame);
}

void StreamReceiver::start(const std::string& signalUrl,const std::string& myId,const std::string& peerId) {
	this->m_myId = myId;
	this->m_peerId=peerId;

	//1.配置WebRTC
	rtc::Configuration config;
	//添加Google的STUN服务器用于NAT穿透
	config.iceServers.emplace_back("stun:stun.l.google.com:19302");
	//用于存储设置的WebRTC对象
	m_pc = std::make_shared<rtc::PeerConnection>(config);

	//2.监听Track事件
	m_pc->onTrack([this](std::shared_ptr<rtc::Track> track) {
		if (track->description().type() == "video") {
			qDebug() << "Received Video Track!";
			//初始化解码器
			if (!initDecoder(AV_CODEC_ID_H264)) {
				emit stateChange("error");
				return;
			}
			//当接受到一帧新数据使用定义的回调函数处理得到的信息
			track->onMessage([this](rtc::message_variant data) {
				if (std::holds_alternative<rtc::binary>(data)) {//确定数据是否含有message_variant类型，并将其中的二进制拿出
					const auto& bin = std::get<rtc::binary>(data);
					onTrackData(bin);//处理接受到的二进制数据
				}
			});
		}

		});
	//3.设置ICE Candidate回调(向信令服务器传输候选地址信息)
	m_pc->onLocalCandidate([this](rtc::Candidate candidate) {
		//将Candidate发送给信令服务器
		QJsonObject json;
		json["type"] = "ICE";
		json["from"] = QString::fromStdString(m_myId);//添加from字段
		json["to"] = QString::fromStdString(m_peerId);
		QJsonObject data;
		data["candidate"] = QString::fromStdString(candidate.candidate());
		data["sdpMid"] = QString::fromStdString(candidate.mid());
		data["sdpMLineIndex"] = candidate.port().value();
		json["data"] = data;

		QJsonDocument doc(json);
		if (m_ws && m_ws->isOpen()) {
			m_ws->send(doc.toJson(QJsonDocument::Compact).toStdString());//发送信息
		}
	});
	m_pc->onStateChange([this](rtc::PeerConnection::State state) {
		qDebug() << "PeerConnection State" << (int) state;
		if (state == rtc::PeerConnection::State::Connected) {
			emit stateChange("Connected");
		}
	});
	//启动信令连接
	setupSignaling(signalUrl);
}
void StreamReceiver::stop() {
	CleanupDecoder();
	if (m_ws) {
		m_ws->close();
		m_ws = nullptr;
	}
	emit stateChange("stopped");
}

bool StreamReceiver::initDecoder(AVCodecID codecId) {
	if (m_isDecoderInited)
		return true;
	m_codec = avcodec_find_decoder(codecId);
	if (!m_codec) {
		qCritical() << "Could not found";
		return false;
	}
	m_codecCtx = avcodec_alloc_context3(m_codec);
	if (!m_codecCtx)
		return false;
    //初始化Parser（可选使用）
	m_parser = av_parser_init(codecId);//根据编码数据格式处理粘包/拆包问题
	if (!m_parser) {
		qCritical() << "Parser not found!";
	}
	m_isDecoderInited = true;
	qDebug() << "Decoder initialized successfully";
	return true;
}

void StreamReceiver::CleanupDecoder() {
	if (m_codecCtx) {
		avcodec_free_context(&m_codecCtx);
		m_codecCtx = nullptr;
	}
	if (m_parser) {
		av_parser_close(m_parser);
		m_parser = nullptr;
	}
	//分配的编码器是由FFmpeg进行管理的，所以不需要手动释放
	m_isDecoderInited = false;
}

void StreamReceiver::onTrackData(const std::vector<std::byte>& data) {
	if (!m_isDecoderInited) return;
	//1.将std::byte转换为uint8_t
	const uint8_t* rawData = reinterpret_cast<const uint8_t*>(data.data());//转换为指向data头部的uint8_t指针
	int rawSize = static_cast<int>(data.size());//获取数据大小

	//2.RTP处理，到达时可能是乱序且一个RTP包通常只包含一个帧的部分，使用av_parser_parse2()函数处理
	uint8_t* parserChunk = nullptr;
	int parserSize = 0;

	//3.循环解析发，确保消耗掉所有的输入数据
	while (rawSize > 0) {
		//av_parser_parser：将rawData加入到缓存中，当凑齐一个NALU，parserSize不为0.并返回len代表消耗的字节大小
		int len = av_parser_parse2(m_parser, m_codecCtx, &parserChunk, &parserSize, rawData, rawSize, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
		rawData += len;//将数据指针后移
		rawSize -= len;//未处理数据减少

		if (parserSize > 0) {
			//完成的一帧数据进行存储
			m_packet->data = parserChunk;
			m_packet->size = parserSize;
			//4.发送给码器解码
			int ret = avcodec_send_packet(m_codecCtx,m_packet);
			if (ret < 0) {
				qWarning() << "Error Sending packet to decoder:" << ret;//错误是因为不包含关键帧，在流媒体发送初期很常见
				continue;
			}
			//得到解码后的原始画面（YUV）
			while (ret >= 0) {
				ret = avcodec_receive_frame(m_codecCtx,m_frame);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
					break;//需要更多的数据解出下一帧
				}
				else if (ret < 0) {
					qCritical() << "Error During Decoding";
					break;
				}
				
				emit frameReady(
					m_frame->data[0],m_frame->data[1],m_frame->data[2],
					m_frame->linesize[0],m_frame->linesize[1],m_frame->linesize[2],
					m_frame->width,m_frame->height);
			}
		}
	}
}

void StreamReceiver::setupSignaling(const std::string& url) {
	//1.在生成answer之后，触发onLocalDescription,发送给信令服务器接收端的SDP描述
	if (m_pc) {
		m_pc->onLocalDescription([this](rtc::Description desc) {
			qDebug() << "Local Description(Answer) Generated,sending ...";
			QJsonObject json;
			if (desc.typeString() == "answer") {
				json["type"] = "ANSWER";
			}
			else if (desc.typeString() == "offer") {
				json["type"] = "OFFER";
			}
			else {
				json["type"] = (QString::fromStdString(desc.typeString())).toUpper();
			}
			json["from"] = QString::fromStdString(m_myId);
			json["to"] = QString::fromStdString(m_peerId);

			QJsonObject data;
			data["sdp"] = QString::fromStdString(std::string(desc));
			json["data"] = data;

			QJsonDocument doc(json);
			if (m_ws && m_ws->isOpen()) {
				m_ws->send(doc.toJson(QJsonDocument::Compact).toStdString());
			}
		});
	}

	//2.配置WebSocket，告知信令服务器登录信息
	m_ws = std::make_shared<rtc::WebSocket>();
	m_ws->onOpen([this](){
		qDebug() << "WebSocket Connected!";
		//向信令服务器发送相关的连接信息（需要视情况修改）
		QJsonObject json;
		json["type"] = "REGISTER_REQUEST";
		json["to"] = "SERVER";
		QJsonDocument doc(json);
		m_ws->send(doc.toJson(QJsonDocument::Compact).toStdString());
	});

	//3.处理信令服务器转发的消息
	m_ws->onMessage([this](rtc::message_variant data) {
		if (!std::holds_alternative<std::string>(data)) return;
		if (std::holds_alternative<std::string>(data)) {
			std::string str = std::get<std::string>(data);
			//JSON 解析
			QJsonParseError parserError;
			QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(str), &parserError);
			if (parserError.error != QJsonParseError::NoError || !doc.isObject()) {
				qWarning() << "Ignored invalid JSON signaling message";
				return;
			}

			QJsonObject json = doc.object();
			QString type = json["type"].toString();
			//3.1.处理 SDP Offer
			if (type == "offer") {
				qDebug() << "Received Offer";
				std::string sdp = json["sdp"].toString().toStdString();
				//将远端传输的SDP设置在本地
				m_pc->setRemoteDescription(rtc::Description(sdp,"offer"));
				//触发Answer生成
				if(m_pc){
					m_pc->setLocalDescription();//生成本地的SDP描述
				}
			}
			//3.2.处理 ICE Candidate（收到远端的ICE候选地址）
			else if (type == "candidate") {
				if (!m_pc || !m_pc->remoteDescription().has_value()) return;

				std::string candidate = json["candidate"].toString().toStdString();
				std::string mid = json["sdpMid"].toString().toStdString();
				m_pc->addRemoteCandidate(rtc::Candidate(candidate, mid));
			}
		}
	});

	m_ws->onError([](std::string s) {
		qCritical() << "Web Socket Error:" << QString::fromStdString(s);
	});
	m_ws->onClosed([]() {
		qDebug() << "Web Socket Closed";
	});
	//3.发起连接
	m_ws->open(url);
}

