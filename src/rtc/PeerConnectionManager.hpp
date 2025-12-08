#pragma once
#include <QObject>
#include <memory>
#include <rtc/rtc.hpp>

#include "signaling-server/src/Common.hpp"

class WsSignalingClient;

class PeerConnectionManager : public QObject {
    Q_OBJECT
public:
    PeerConnectionManager(QObject* parent = nullptr);
    ~PeerConnectionManager();

    void registerClient();
    void start(const QString& targetId);
    void sendEncodedVideoFrame(const QByteArray& encodedData);
    QString id() const;
    QString target() const;

signals:
    void signalingConnected();
    void signalingError(const QString& msg);
    void peerJoined(const QString& peerId);
    void peersList(const QJsonArray& list);
    void p2pConnected();     // datachannel has established
    void p2pDisconnected();
    void encodedFrameReceived(QByteArray data);

    void connected();
    void disconnected();
    void errorOccurred(const QString& msg);
    void messageReceived(const QString& msg); 

public:
    void onConnectServer(const QString& url);
    void onSignalingMessage(const QJsonObject& obj);
    void onJoined(const QString& peerId);

private:
    void handleSignalingMessage(const QJsonObject& json);
    void sendSignalingMessage(const QString& type, const QString& to, const QJsonObject& data);

    void createPeerConnection();
    void setupDataChannel();
    void bindDataChannel(std::shared_ptr<rtc::DataChannel> dc);
    
    
private:
    std::shared_ptr<rtc::WebSocket> m_ws;
    std::shared_ptr<rtc::PeerConnection> m_pc;
    std::shared_ptr<rtc::DataChannel> m_videoChannel;

    QString m_serverUrl;
    QString m_myId;
    QString m_targetPeerId;
    bool m_isCaller; 
};
