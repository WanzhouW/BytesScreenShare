#pragma once
#include <QObject>
#include <memory>
#include <rtc/rtc.hpp>

class TcpSignalingClient;

class PeerConnectionManager : public QObject {
    Q_OBJECT
public:
    PeerConnectionManager(TcpSignalingClient* signaling, bool isCaller, QObject* parent = nullptr);

    void start();                       // 建立 PeerConnection，必要时发 offer
public slots:
    void sendMessage(const QString&);   // 发聊天消息

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& msg);
    void messageReceived(const QString& msg);   // 收到对方发来的文本

public slots:
    void onSignalingMessage(const QJsonObject& obj);

private:
    void createPeerConnection();
    void setupDataChannel(const std::shared_ptr<rtc::DataChannel>& dc);
    void handleLocalDescription(std::shared_ptr<rtc::Description> desc);
    void handleLocalCandidate(std::shared_ptr<rtc::Candidate> cand);

    TcpSignalingClient* m_signaling;
    bool m_isCaller;
    std::shared_ptr<rtc::PeerConnection> m_pc;
    std::shared_ptr<rtc::DataChannel> m_dc;
};
