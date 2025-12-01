#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>

class TcpSignalingClient : public QObject {
    Q_OBJECT
public:
    explicit TcpSignalingClient(QObject* parent = nullptr);

    void connectToServer(const QString& host, quint16 port);
    void sendJson(const QJsonObject& obj);

signals:
    void connected();
    void disconnected();
    void jsonReceived(const QJsonObject& obj);

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();

private:
    QTcpSocket m_socket;
    QByteArray m_buffer; // 处理拆包粘包，这里简单按行分隔
};
