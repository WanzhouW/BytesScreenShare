#include "TcpSignalingClient.hpp"
#include <QJsonDocument>

TcpSignalingClient::TcpSignalingClient(QObject* parent)
    : QObject(parent) {

    connect(&m_socket, &QTcpSocket::readyRead,
            this, &TcpSignalingClient::onReadyRead);
    connect(&m_socket, &QTcpSocket::connected,
            this, &TcpSignalingClient::onConnected);
    connect(&m_socket, &QTcpSocket::disconnected,
            this, &TcpSignalingClient::onDisconnected);
}

void TcpSignalingClient::connectToServer(const QString& host, quint16 port) {
    m_socket.connectToHost(host, port);
}

void TcpSignalingClient::sendJson(const QJsonObject& obj) {
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    data.append('\n'); // 约定一条消息一行
    m_socket.write(data);
}

void TcpSignalingClient::onConnected() {
    emit connected();
}

void TcpSignalingClient::onDisconnected() {
    emit disconnected();
}

void TcpSignalingClient::onReadyRead() {
    m_buffer.append(m_socket.readAll());
    int index;
    while ((index = m_buffer.indexOf('\n')) != -1) {
        QByteArray line = m_buffer.left(index);
        m_buffer.remove(0, index + 1);

        QJsonParseError err{};
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            emit jsonReceived(doc.object());
        }
    }
}
