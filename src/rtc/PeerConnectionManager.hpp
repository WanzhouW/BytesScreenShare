#pragma once
#include <QObject>
#include <memory>
#include <rtc/rtc.hpp>

class WsSignalingClient;

class PeerConnectionManager : public QObject {
    Q_OBJECT
public:
    PeerConnectionManager(WsSignalingClient* signaling, bool isCaller, QObject* parent = nullptr);

    void start();  
public slots:
    void sendMessage(const QString&); 

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& msg);
    void messageReceived(const QString& msg); 

public slots:
    void onSignalingMessage(const QJsonObject& obj);

private:
    void createPeerConnection();
    void setupDataChannel(const std::shared_ptr<rtc::DataChannel>& dc);
    void handleLocalDescription(std::shared_ptr<rtc::Description> desc);
    void handleLocalCandidate(std::shared_ptr<rtc::Candidate> cand);

    WsSignalingClient* m_signaling;
    bool m_isCaller;
    std::shared_ptr<rtc::PeerConnection> m_pc;
    std::shared_ptr<rtc::DataChannel> m_dc;
};
