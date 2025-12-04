#pragma once
#include <QObject>
#include <QWebSocket> // 替换 QTcpSocket，websocket版本
#include <QJsonObject>
#include <QUrl> // 需要 QUrl

class WsSignalingClient : public QObject { // 名称可以改为 WebSocketSignalingClient
    Q_OBJECT
public:
    explicit WsSignalingClient(QObject* parent = nullptr);

    // 更改连接函数，使其接受 URL 或 host/port
    void connectToServer(const QString& host, quint16 port); 
    void disconnectFromServer(); 
    void sendJson(const QJsonObject& obj);

signals:
    void connected();
    void disconnected();
    void jsonReceived(const QJsonObject& obj);

private slots:
    // QWebSocket 的槽函数：
    void onConnected(); 
    void onDisconnected();
    void onTextMessageReceived(const QString& message); // 替换 onReadyRead

private:
    QWebSocket m_socket;
    // QWebSocket 自动处理分帧，通常不再需要手动维护 QByteArray m_buffer;
};