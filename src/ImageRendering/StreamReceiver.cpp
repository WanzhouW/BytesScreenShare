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

	m_watchingTimer = new QTimer(this);
	m_watchingTimer->setInterval(12000);
	//800毫秒内没有接收到新数据，认为断开
	connect(m_watchingTimer,&QTimer::timeout,this,&StreamReceiver::onStreamTimeout);
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
	if (!m_codecCtx) {
		qCritical() << "Codec context not found!";
		return false;
	}
	// 设置解码器参数
	m_codecCtx->thread_count = 4;  // 多线程解码

	//打开解码器！
	int ret = avcodec_open2(m_codecCtx, m_codec, nullptr);
	if (ret < 0) {
		char errbuf[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(ret, errbuf, sizeof(errbuf));
		qCritical() << "Failed to open codec:" << errbuf;
		avcodec_free_context(&m_codecCtx);
		return false;
	}
	//初始化Parser（可选使用）
	m_parser = av_parser_init(codecId);//根据编码数据格式处理粘包/拆包问题
	if (!m_parser) {
		qWarning() << "Parser not available, will decode directly";
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
	if (m_watchingTimer) {
		m_watchingTimer->start();
	}
	if (!m_isDecoderInited) {
		qWarning() << "Decoder not initialized!";
		return;
	}
	//1.将std::byte转换为uint8_t
	const uint8_t* nalData = reinterpret_cast<const uint8_t*>(data.data());//转换为指向data头部的uint8_t指针
	int nalSize = static_cast<int>(data.size());//获取数据大小
	if (nalSize <= 0) {
		qWarning() << "Empty NALU!";
		emit stateChange("error");
		return;
	}
	// 【新增】分析 NALU 类型
	uint8_t naluType = nalData[0] & 0x1F;
	const char* naluTypeName = "Unknown";
	bool isConfigFrame = false;

	switch (naluType) {
	case 1: naluTypeName = "Non-IDR Picture"; break;
	case 5: naluTypeName = "IDR Picture"; break;
	case 6: naluTypeName = "SEI"; break;
	case 7: naluTypeName = "SPS"; isConfigFrame = true; break;
	case 8: naluTypeName = "PPS"; isConfigFrame = true; break;
	case 9: naluTypeName = "Access Unit Delimiter"; break;
	default: naluTypeName = QString("Type_%1").arg(naluType).toLocal8Bit().data(); break;
	}

	qDebug() << "Processing NALU:" << naluTypeName << "type=" << naluType
		<< "size=" << nalSize << "isConfig=" << isConfigFrame;

	// 1. 准备 Annex-B 起始码
	std::vector<uint8_t> packetData;
	// 关键帧(5)或配置帧(7,8)用 4字节头，其他用 3字节头 (这其实是兼容性写法，统一用4字节 00 00 00 01 也是完全合法的)
	if (naluType == 5 || naluType == 7 || naluType == 8) {
		packetData = { 0x00, 0x00, 0x00, 0x01 };
	}
	else {
		packetData = { 0x00, 0x00, 0x01 };
	}

	// 2. 拼接数据
	packetData.insert(packetData.end(), nalData, nalData + nalSize);

	// 3. 直接填充 AVPacket (跳过 av_parser_parse2)
	// 注意：av_packet_alloc 已经在构造函数里做了，这里要重置一下或者确保它是干净的
	av_packet_unref(m_packet);

	// 必须分配内存并拷贝，因为 packetData 是局部变量，函数结束会被销毁
	if (av_new_packet(m_packet, packetData.size()) < 0) {
		qCritical() << "Failed to allocate packet";
		return;
	}
	memcpy(m_packet->data, packetData.data(), packetData.size());
	emit stateChange("Connected");
	// 4. 解码
	decodePacket();
	
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

void StreamReceiver::processEncodedData(const QByteArray& naluData) {
	qDebug() << "process EncodedData";
	if (!m_isDecoderInited) {
		if (!initDecoder(AV_CODEC_ID_H264)) {
			qWarning() << "Failed to init decoder";
			return;
		}
	}

	//将QByteArray转换为std::vector<std::byte>
	std::vector<std::byte> data;
	data.reserve(naluData.size());

	const char* rawData = naluData.constData();
	for (int i = 0; i < naluData.size(); ++i) {
		data.push_back(static_cast<std::byte>(rawData[i]));
	}
	//调用现有的处理函数
	onTrackData(data);
}


//提取解码逻辑为独立方法
void StreamReceiver::decodePacket() {
	qDebug() << "Sending packet to decoder, size:" << m_packet->size;

	int ret = avcodec_send_packet(m_codecCtx, m_packet);
	if (ret < 0) {
		if (ret == AVERROR(EAGAIN)) {
			qDebug() << "Decoder buffer full, need to read frames first";
		}
		else if (ret != AVERROR_INVALIDDATA) {
			char errbuf[AV_ERROR_MAX_STRING_SIZE];
			av_strerror(ret, errbuf, sizeof(errbuf));
			qWarning() << "Error sending packet:" << errbuf;
		}
		return;
	}

	qDebug() << "Packet sent to decoder successfully";

	// 尝试获取所有可用的解码帧
	while (true) {
		ret = avcodec_receive_frame(m_codecCtx, m_frame);

		if (ret == AVERROR(EAGAIN)) {
			qDebug() << "Need more data (EAGAIN)";
			break;
		}
		else if (ret == AVERROR_EOF) {
			qDebug() << "End of stream (EOF)";
			break;
		}
		else if (ret < 0) {
			char errbuf[AV_ERROR_MAX_STRING_SIZE];
			av_strerror(ret, errbuf, sizeof(errbuf));
			qCritical() << "Error during decoding:" << errbuf;
			break;
		}

		qDebug() << "Successfully decoded frame:"
			<< "format:" << m_frame->format
			<< "size:" << m_frame->width << "x" << m_frame->height
			<< "YUV420P:" << (m_frame->format == AV_PIX_FMT_YUV420P);

		// 【新增/替换】为以下代码：

		int width = m_frame->width;
		int height = m_frame->height;
		int uvWidth = width / 2;
		int uvHeight = height / 2;

		// 1. 调整缓冲区大小
		if (m_bufferY.size() != width * height) {
			m_bufferY.resize(width * height);
			m_bufferU.resize(uvWidth * uvHeight);
			m_bufferV.resize(uvWidth * uvHeight);
		}

		// 2. 逐行拷贝 Y 数据 (去除 FFmpeg 的 stride 填充)
		uint8_t* srcY = m_frame->data[0];
		uint8_t* dstY = reinterpret_cast<uint8_t*>(m_bufferY.data());
		for (int i = 0; i < height; i++) {
			// 关键点：src 使用 linesize[0] 跳过填充，dst 使用 width 紧凑排列
			memcpy(dstY + i * width, srcY + i * m_frame->linesize[0], width);
		}

		// 3. 逐行拷贝 U 数据
		uint8_t* srcU = m_frame->data[1];
		uint8_t* dstU = reinterpret_cast<uint8_t*>(m_bufferU.data());
		for (int i = 0; i < uvHeight; i++) {
			memcpy(dstU + i * uvWidth, srcU + i * m_frame->linesize[1], uvWidth);
		}

		// 4. 逐行拷贝 V 数据
		uint8_t* srcV = m_frame->data[2];
		uint8_t* dstV = reinterpret_cast<uint8_t*>(m_bufferV.data());
		for (int i = 0; i < uvHeight; i++) {
			memcpy(dstV + i * uvWidth, srcV + i * m_frame->linesize[2], uvWidth);
		}

		// 5. 发送信号
		// 【重点】这里我们传入 width 作为 stride 参数！
		// 因为我们的 m_buffer 已经是紧凑排列的了，没有 padding
		emit frameReady(
			reinterpret_cast<uchar*>(m_bufferY.data()),
			reinterpret_cast<uchar*>(m_bufferU.data()),
			reinterpret_cast<uchar*>(m_bufferV.data()),
			width,      // yStride 欺骗接收端说 stride 就是 width
			uvWidth,    // uStride
			uvWidth,    // vStride
			width,
			height
		);

		// 6. 释放 FFmpeg 帧
		av_frame_unref(m_frame);

		
	}
}

//实现超时处理函数
void StreamReceiver::onStreamTimeout() {
	qDebug() << "Stream watchingdog timeout! Assuming remote disconnected.";

	m_watchingTimer->stop();

	emit stateChange("disconnected");
	CleanupDecoder();

}