#pragma once

#include <QtWidgets/QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QGroupBox>

// 定义之前的信令类型字符串
namespace SignalingType {
    const QString REGISTER_REQUEST = "REGISTER_REQUEST";
    const QString REGISTER_SUCCESS = "REGISTER_SUCCESS";
    const QString OFFER = "OFFER";
    const QString ANSWER = "ANSWER";
    const QString ICE = "ICE";
    const QString PEER_JOINED = "PEER_JOINED";
}
#include "ui_TestClient.h"

class TestClient : public QWidget
{
    Q_OBJECT

public:
    TestClient(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        setupNetwork();
        setWindowTitle("WebRTC Signaling Test Client");
        resize(500, 700);
    }

    ~TestClient() {
        if (_socket) _socket->close();
    }

private slots:
    // 连接/断开服务器
    void onBtnConnectClicked() {
        if (_isConnected) {
            _socket->close();
        }
        else {
            QString urlStr = _editUrl->text();
            log("Connecting to " + urlStr + " ...");
            _socket->open(QUrl(urlStr));
        }
    }

    // WebSocket 事件
    void onConnected() {
        _isConnected = true;
        _btnConnect->setText("Disconnect");
        log("WebSocket Connected!", Qt::darkGreen);
        _btnRegister->setEnabled(true); // 连接成功后允许注册
    }

    void onDisconnected() {
        _isConnected = false;
        _btnConnect->setText("Connect");
        _myId = "";
        _labelMyId->setText("Unregistered");
        setButtonsEnabled(false);
        log("WebSocket Disconnected.", Qt::red);
    }

    void onTextMessageReceived(const QString& message) {
        // 1. 打印原始消息
        log("<< RECV: " + message, Qt::blue);

        // 2. 简单的自动解析逻辑
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (!doc.isObject()) return;

        QJsonObject json = doc.object();
        QString type = json["type"].toString();

        // 自动处理注册成功
        if (type == SignalingType::REGISTER_SUCCESS) {
            QJsonObject data = json["data"].toObject();
            if (data.contains("peerId")) {
                _myId = data["peerId"].toString();
                _labelMyId->setText("My ID: " + _myId);
                log("Received ID assignment: " + _myId, Qt::darkGreen);
                setButtonsEnabled(true); // 注册成功后允许发送信令
            }
        }
        // 自动处理新用户加入 (方便测试，自动填入 Target ID)
        else if (type == SignalingType::PEER_JOINED) {
            QJsonObject data = json["data"].toObject();
            QString newPeerId = data["id"].toString();
            if (!newPeerId.isEmpty()) {
                _editTargetId->setText(newPeerId);
                log("Auto-filled Target ID: " + newPeerId, Qt::darkYellow);
            }
        }
    }

    // --- 发送具体的信令 ---

    void onBtnRegisterClicked() {
        // 构造注册消息 (注意：Register不需要 from)
        QJsonObject json;
        json["type"] = SignalingType::REGISTER_REQUEST;
        json["to"] = "Server";
        QJsonObject data;
        data["device"] = "QtTestClient";
        json["data"] = data;

        sendJson(json);
    }

    void onBtnOfferClicked() {
        if (!checkTarget()) return;

        QJsonObject json;
        json["type"] = SignalingType::OFFER;
        json["from"] = _myId;  // 必须带上自己的ID
        json["to"] = _editTargetId->text().trimmed();

        QJsonObject data;
        data["sdp"] = "v=0\r\n(Mock SDP Offer Data)...";
        json["data"] = data;

        sendJson(json);
    }

    void onBtnAnswerClicked() {
        if (!checkTarget()) return;

        QJsonObject json;
        json["type"] = SignalingType::ANSWER;
        json["from"] = _myId;
        json["to"] = _editTargetId->text().trimmed();

        QJsonObject data;
        data["sdp"] = "v=0\r\n(Mock SDP Answer Data)...";
        json["data"] = data;

        sendJson(json);
    }

    void onBtnIceClicked() {
        if (!checkTarget()) return;

        QJsonObject json;
        json["type"] = SignalingType::ICE;
        json["from"] = _myId;
        json["to"] = _editTargetId->text().trimmed();

        QJsonObject data;
        data["candidate"] = "candidate:123 1 udp ...";
        data["sdpMid"] = "video";
        data["sdpMLineIndex"] = 0;
        json["data"] = data;

        sendJson(json);
    }

private:
    // 辅助函数：发送 JSON
    void sendJson(const QJsonObject& json) {
        QJsonDocument doc(json);
        QString strJson(doc.toJson(QJsonDocument::Compact));

        // 发送
        if (_socket->sendTextMessage(strJson) > 0) {
            log(">> SEND: " + strJson, Qt::black);
        }
        else {
            log("Error sending message!", Qt::red);
        }
    }

    // 辅助函数：日志显示
    void log(const QString& msg, const QColor& color = Qt::black) {
        _logView->setTextColor(color);
        _logView->append(QDateTime::currentDateTime().toString("[HH:mm:ss] ") + msg);
        _logView->moveCursor(QTextCursor::End);
    }

    // 辅助函数：检查是否有目标ID
    bool checkTarget() {
        if (_editTargetId->text().trimmed().isEmpty()) {
            log("Error: Target ID is empty! Wait for PEER_JOINED or enter manually.", Qt::red);
            return false;
        }
        return true;
    }

    // 辅助函数：控制按钮状态
    void setButtonsEnabled(bool enable) {
        _btnOffer->setEnabled(enable);
        _btnAnswer->setEnabled(enable);
        _btnIce->setEnabled(enable);
        _btnRegister->setEnabled(!enable && _isConnected); // 注册成功后禁用注册按钮
    }

    // --- 初始化 UI ---
    void setupUI() {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        // 1. 连接区域
        QHBoxLayout* connLayout = new QHBoxLayout();
        _editUrl = new QLineEdit("ws://127.0.0.1:11290", this);
        _btnConnect = new QPushButton("Connect", this);
        connLayout->addWidget(new QLabel("Server:"));
        connLayout->addWidget(_editUrl);
        connLayout->addWidget(_btnConnect);
        mainLayout->addLayout(connLayout);

        // 2. 状态显示
        _labelMyId = new QLabel("Unregistered", this);
        _labelMyId->setStyleSheet("font-weight: bold; font-size: 14px; color: blue;");
        _labelMyId->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(_labelMyId);

        // 3. 日志区域
        _logView = new QTextEdit(this);
        _logView->setReadOnly(true);
        mainLayout->addWidget(_logView);

        // 4. 操作区域
        QGroupBox* actionBox = new QGroupBox("Actions", this);
        QVBoxLayout* actionLayout = new QVBoxLayout(actionBox);

        // 注册按钮
        _btnRegister = new QPushButton("1. Send REGISTER_REQUEST", this);
        _btnRegister->setEnabled(false);
        actionLayout->addWidget(_btnRegister);

        // 目标 ID 输入
        QHBoxLayout* targetLayout = new QHBoxLayout();
        _editTargetId = new QLineEdit(this);
        _editTargetId->setPlaceholderText("Enter Target Peer ID here...");
        targetLayout->addWidget(new QLabel("Target ID:"));
        targetLayout->addWidget(_editTargetId);
        actionLayout->addLayout(targetLayout);

        // 信令按钮
        QHBoxLayout* sigLayout = new QHBoxLayout();
        _btnOffer = new QPushButton("Send OFFER", this);
        _btnAnswer = new QPushButton("Send ANSWER", this);
        _btnIce = new QPushButton("Send ICE", this);
        sigLayout->addWidget(_btnOffer);
        sigLayout->addWidget(_btnAnswer);
        sigLayout->addWidget(_btnIce);
        actionLayout->addLayout(sigLayout);

        mainLayout->addWidget(actionBox);

        setButtonsEnabled(false); // 初始禁用
    }

    void setupNetwork() {
        _socket = new QWebSocket();
        connect(_socket, &QWebSocket::connected, this, &TestClient::onConnected);
        connect(_socket, &QWebSocket::disconnected, this, &TestClient::onDisconnected);
        connect(_socket, &QWebSocket::textMessageReceived, this, &TestClient::onTextMessageReceived);

        connect(_btnConnect, &QPushButton::clicked, this, &TestClient::onBtnConnectClicked);
        connect(_btnRegister, &QPushButton::clicked, this, &TestClient::onBtnRegisterClicked);
        connect(_btnOffer, &QPushButton::clicked, this, &TestClient::onBtnOfferClicked);
        connect(_btnAnswer, &QPushButton::clicked, this, &TestClient::onBtnAnswerClicked);
        connect(_btnIce, &QPushButton::clicked, this, &TestClient::onBtnIceClicked);
    }

private:
    QWebSocket* _socket;
    bool _isConnected = false;
    QString _myId;

    // UI Components
    QLineEdit* _editUrl;
    QPushButton* _btnConnect;
    QLabel* _labelMyId;
    QTextEdit* _logView;
    QPushButton* _btnRegister;
    QLineEdit* _editTargetId;
    QPushButton* _btnOffer;
    QPushButton* _btnAnswer;
    QPushButton* _btnIce;

    Ui::TestClientClass ui;
};

