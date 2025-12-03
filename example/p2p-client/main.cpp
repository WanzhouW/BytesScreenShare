#include <QtWidgets/QApplication>
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QDebug>
#include <QObject>
#include <QThread>

#include <rtc/rtc.hpp>

#include <string>
#inlcude <memory>

using namespace std;

// --- 信令类型定义 ---
const QString TYPE_REGISTER_REQ = "REGISTER_REQUEST";
const QString TYPE_REGISTER_SUC = "REGISTER_SUCCESS";
const QString TYPE_OFFER = "OFFER";
const QString TYPE_ANSWER = "ANSWER";
const QString TYPE_ICE = "ICE";
const QString TYPE_PEER_JOINED = "PEER_JOINED";

class P2PClient : public QWidget {
    Q_OBJECT

public:
    P2PClient(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        setupSignaling();
    }

    ~P2PClient() {
        if (ws) ws->close();
        pc.reset(); // 释放 PeerConnection
    }

private:
    // --- UI 组件 ---
    QTextEdit* logView;
    QLineEdit* serverAddr;
    QPushButton* btnConnect;
    QComboBox* peerList;
    QPushButton* btnStartP2P;
    QPushButton* btnSendMessage;

    // --- 核心成员 ---
    QWebSocket* ws;
    QString myId;
    bool isConnected = false;

    // LibDataChannel 智能指针
    shared_ptr<rtc::PeerConnection> pc;
    shared_ptr<rtc::DataChannel> dc;

    // --- 1. UI 初始化 ---
    void setupUI() {
        QVBoxLayout* layout = new QVBoxLayout(this);

        // 服务器连接区
        QHBoxLayout* h1 = new QHBoxLayout();
        serverAddr = new QLineEdit("ws://127.0.0.1:11290", this);
        btnConnect = new QPushButton("连接信令服务器", this);
        h1->addWidget(new QLabel("Server:"));
        h1->addWidget(serverAddr);
        h1->addWidget(btnConnect);
        layout->addLayout(h1);

        // P2P 操作区
        QHBoxLayout* h2 = new QHBoxLayout();
        peerList = new QComboBox(this);
        btnStartP2P = new QPushButton("发起 Offer (Start P2P)", this);
        btnStartP2P->setEnabled(false);
        h2->addWidget(new QLabel("在线 Peer:"));
        h2->addWidget(peerList, 1);
        h2->addWidget(btnStartP2P);
        layout->addLayout(h2);

        // DataChannel 测试区
        btnSendMessage = new QPushButton("发送 DataChannel 消息", this);
        btnSendMessage->setEnabled(false);
        layout->addWidget(btnSendMessage);

        // 日志区
        logView = new QTextEdit(this);
        logView->setReadOnly(true);
        layout->addWidget(logView);

        resize(500, 650);
        setWindowTitle("WebRTC P2P Client (Fixed)");

        // 绑定 UI 信号
        connect(btnConnect, &QPushButton::clicked, this, &P2PClient::connectSignaling);
        connect(btnStartP2P, &QPushButton::clicked, this, &P2PClient::startP2P);

        // 发送数据通道消息
        connect(btnSendMessage, &QPushButton::clicked, [this]() {
            if (dc && dc->isOpen()) {
                QString txt = "Hello from " + myId + " at " + QTime::currentTime().toString();
                dc->send(txt.toStdString());
                log("[DataChannel] >> Sent: " + txt);
            }
            else {
                log("[Error] DataChannel not open.");
            }
            });
    }

    // --- 2. 线程安全的辅助函数 ---

    // 线程安全的日志打印
    void log(const QString& msg) {
        // 如果当前不在主线程，转发给主线程
        if (QThread::currentThread() != this->thread()) {
            QMetaObject::invokeMethod(this, [this, msg]() { log(msg); }, Qt::QueuedConnection);
            return;
        }
        logView->append(msg);
    }

    void sendJson(const QJsonObject& json) {
        // LibDataChannel 的回调在后台线程，必须使用 invokeMethod 切换到主线程发送 WebSocket
        QMetaObject::invokeMethod(this, [this, json]() {
            if (!ws || !ws->isValid()) {
                log("[Error] WebSocket not connected, cannot send signaling.");
                return;
            }
            QJsonDocument doc(json);
            QString str = doc.toJson(QJsonDocument::Compact);
            ws->sendTextMessage(str);
            // 仅打印关键信令，避免刷屏
            QString type = json["type"].toString();
            if (type != TYPE_ICE) log("[Sig] >> SEND: " + type);
            }, Qt::QueuedConnection);
    }

    // --- 3. 信令 WebSocket 逻辑 ---
    void setupSignaling() {
        ws = new QWebSocket();
        connect(ws, &QWebSocket::connected, this, &P2PClient::onConnected);
        connect(ws, &QWebSocket::textMessageReceived, this, &P2PClient::onMessage);
        connect(ws, &QWebSocket::disconnected, [this]() {
            log("[WebSocket] Disconnected");
            isConnected = false;
            btnConnect->setEnabled(true);
            btnStartP2P->setEnabled(false);
            peerList->clear();
            });
    }

    void connectSignaling() {
        if (ws->state() == QAbstractSocket::ConnectedState) return;
        ws->open(QUrl(serverAddr->text()));
        log("Connecting to Signaling Server...");
    }

    void onConnected() {
        isConnected = true;
        log("[WebSocket] Connected!");
        btnConnect->setEnabled(false);

        QJsonObject json;
        json["type"] = TYPE_REGISTER_REQ;
        json["to"] = "Server";
        sendJson(json);
    }

    void onMessage(const QString& message) {
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (!doc.isObject()) return;
        QJsonObject json = doc.object();
        QString type = json["type"].toString();

        if (type == TYPE_REGISTER_SUC) {
            handleRegisterSuccess(json);
        }
        else if (type == TYPE_PEER_JOINED) {
            QString newId = json["data"].toObject()["id"].toString();
            if (!newId.isEmpty() && newId != myId) {
                peerList->addItem(newId);
                log("[Sig] Peer Joined: " + newId);
            }
        }
        else if (type == TYPE_OFFER) {
            handleOffer(json);
        }
        else if (type == TYPE_ANSWER) {
            handleAnswer(json);
        }
        else if (type == TYPE_ICE) {
            handleIce(json);
        }
    }

    void handleRegisterSuccess(const QJsonObject& json) {
        QJsonObject data = json["data"].toObject();
        myId = data["peerId"].toString();
        setWindowTitle("Client: " + myId);
        log("[Sig] Registered. My ID: " + myId);

        peerList->clear();
        QJsonArray peers = data["peers"].toArray();
        for (auto p : peers) {
            if (p.toString() != myId) peerList->addItem(p.toString());
        }
        btnStartP2P->setEnabled(true);
    }

    // --- 4. WebRTC 核心逻辑 ---
    // 初始化 PeerConnection 并绑定回调
    void createPeerConnection(const QString& targetId) {
        rtc::Configuration config;
        config.iceServers.emplace_back("stun:stun.l.google.com:19302"); // 公网 STUN

        pc = std::make_shared<rtc::PeerConnection>(config);

        // [回调 1] 本地 ICE 候选者生成 -> 发送给对端
        pc->onLocalCandidate([this, targetId](rtc::Candidate cand) {
            QJsonObject json;
            json["type"] = TYPE_ICE;
            json["from"] = myId;
            json["to"] = targetId;
            QJsonObject data;
            data["candidate"] = QString::fromStdString(cand.candidate());
            data["sdpMid"] = QString::fromStdString(cand.mid());
            data["sdpMLineIndex"] = cand.port().value();
            json["data"] = data;
            sendJson(json);
            });

        // [回调 2] 本地 SDP 生成 (Offer 或 Answer) -> 发送给对端
        // 这里的代码会在后台线程运行，sendJson 内部做了线程切换
        pc->onLocalDescription([this, targetId](rtc::Description desc) {
            QString sdpType = (desc.type() == rtc::Description::Type::Offer) ? TYPE_OFFER : TYPE_ANSWER;
            log("[RTC] Local Description Generated: " + sdpType);

            QJsonObject json;
            json["type"] = sdpType;
            json["from"] = myId;
            json["to"] = targetId;
            QJsonObject data;
            data["sdp"] = QString::fromStdString(std::string(desc));
            json["data"] = data;

            sendJson(json);
            });

        // [回调 3] DataChannel 状态变化 (被动接收方)
        pc->onDataChannel([this](shared_ptr<rtc::DataChannel> channel) {
            log("[RTC] Received Remote DataChannel: " + QString::fromStdString(channel->label()));
            setupDataChannel(channel);
            });

        // [回调 4] 连接状态监控
        pc->onStateChange([this](rtc::PeerConnection::State state) {
            log("[RTC] State Changed: " + QString::number((int)state));
            });
    }

    // 绑定 DataChannel 事件
    void setupDataChannel(shared_ptr<rtc::DataChannel> channel) {
        dc = channel;

        // 必须切换回主线程更新 UI
        QMetaObject::invokeMethod(this, [this]() {
            btnSendMessage->setEnabled(true);
            });

        dc->onOpen([this]() {
            log("[DataChannel] State: OPEN");
            dc->send("Hello from " + myId.toStdString());
            });

        dc->onMessage([this](variant<rtc::binary, string> message) {
            if (holds_alternative<string>(message)) {
                log("[DataChannel] << RECV: " + QString::fromStdString(get<string>(message)));
            }
            });
    }

    // --- 5. 业务流程 ---

    // [主动方] 点击按钮 -> 创建 PC -> 创建 DC -> 触发 Offer
    void startP2P() {
        QString targetId = peerList->currentText();
        if (targetId.isEmpty()) {
            log("[Error] No target selected.");
            return;
        }

        log("--- Starting P2P Handshake with " + targetId + " ---");

        // 1. 创建 PeerConnection
        createPeerConnection(targetId);

        // 2. [关键] 主动创建 DataChannel。
        // 在 libdatachannel 中，创建 Track 或 DataChannel 会自动触发 'negotiationneeded'，
        // 进而生成 Offer 并回调 onLocalDescription。
        auto channel = pc->createDataChannel("chat");
        log("[RTC] Created Local DataChannel 'chat'");

        setupDataChannel(channel);
    }

    // [被动方] 收到 Offer -> 创建 PC -> 设置 Remote SDP -> 自动生成 Answer
    void handleOffer(const QJsonObject& json) {
        QString fromId = json["from"].toString();
        log("--- Received OFFER from " + fromId + " ---");

        if (!pc) {
            createPeerConnection(fromId);
        }

        QString sdp = json["data"].toObject()["sdp"].toString();
        // 设置远端 Offer，库会自动生成 Answer 并触发 onLocalDescription
        pc->setRemoteDescription(rtc::Description(sdp.toStdString(), rtc::Description::Type::Offer));
    }

    // [主动方] 收到 Answer -> 设置 Remote SDP -> 连接建立
    void handleAnswer(const QJsonObject& json) {
        log("--- Received ANSWER ---");
        QString sdp = json["data"].toObject()["sdp"].toString();
        if (pc) {
            pc->setRemoteDescription(rtc::Description(sdp.toStdString(), rtc::Description::Type::Answer));
        }
    }

    // [双方] 收到 ICE Candidate -> 添加到 PC
    void handleIce(const QJsonObject& json) {
        QJsonObject data = json["data"].toObject();
        string cand = data["candidate"].toString().toStdString();
        string mid = data["sdpMid"].toString().toStdString();
        // log("[RTC] Adding Remote Candidate...");
        if (pc) {
            pc->addRemoteCandidate(rtc::Candidate(cand, mid));
        }
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    P2PClient w;
    w.show();
    return a.exec();
}

#include "main.moc"
