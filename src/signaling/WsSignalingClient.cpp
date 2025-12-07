#include "WsSignalingClient.hpp"
#include <QJsonDocument>
#include <QDebug>

WsSignalingClient::WsSignalingClient(QObject* parent)
    : QObject(parent) {

    // 更改连接：从 readyRead 变为 textMessageReceived
    connect(&m_socket, &QWebSocket::textMessageReceived,
            this, &WsSignalingClient::onTextMessageReceived); 
            
    connect(&m_socket, &QWebSocket::connected,
            this, &WsSignalingClient::onConnected);
            
    connect(&m_socket, &QWebSocket::disconnected,
            this, &WsSignalingClient::onDisconnected);
    
    // 建议：添加错误连接
    connect(&m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred),
        this, [](QAbstractSocket::SocketError error){
            qDebug() << "WebSocket Error:" << error;
        });
}

void WsSignalingClient::connectToServer(const QString& host, quint16 port) {
    // 构造 WebSocket URL (ws://...)
    QUrl url;
    url.setScheme("ws"); // 假设使用非加密的 ws
    url.setHost(host);
    url.setPort(port);

    qDebug() << "Attempting to open WebSocket to:" << url.toString();
    m_socket.open(url); // QWebSocket 使用 open()
}

void WsSignalingClient::connectToServer(const QString& url)
{
    qDebug() << "Attempting to open WebSocket to:" << url;
    m_socket.open(url);
}

void WsSignalingClient::sendJson(const QJsonObject& obj) {
    QJsonDocument doc(obj);
    // QWebSocket 发送文本消息 (QString)
    QString strJson(doc.toJson(QJsonDocument::Compact)); 
    
    // 发送消息
    m_socket.sendTextMessage(strJson);
    qDebug() << ">> SEND JSON:" << strJson;
}

void WsSignalingClient::onConnected() {
    qDebug() << ">>> WebSocket Client: Connected! <<<";
    emit connected();
}

void WsSignalingClient::onDisconnected() {
    qDebug() << ">>> WebSocket Client: Disconnected. <<<";
    emit disconnected();
}

void WsSignalingClient::disconnectFromServer()
    {
    if (m_socket.isValid()) {
        qDebug() << "[WsSignalingClient] disconnectFromServer() - closing websocket";
        // m_isClosing = true;        // 禁止触发信号
        m_socket.close();
    }
    }

// 新的槽函数：处理接收到的文本消息
void WsSignalingClient::onTextMessageReceived(const QString& message) {
    qDebug() << "<< RECV JSON:" << message;
    
    QJsonParseError err{};
    // QWebSocket 保证接收到的 message 是完整的 JSON 帧
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &err); 
    
    if (err.error == QJsonParseError::NoError && doc.isObject()) {
        emit jsonReceived(doc.object());
    } else {
        qDebug() << "JSON Parse Error:" << err.errorString();
    }
}

