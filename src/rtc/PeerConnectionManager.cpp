#include "PeerConnectionManager.hpp"
#include "../signaling/WsSignalingClient.hpp"
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

PeerConnectionManager::PeerConnectionManager(WsSignalingClient* signaling, bool isCaller, QObject* parent)
    : QObject(parent)
    , m_signaling(signaling)
    , m_isCaller(isCaller)
{
    connect(m_signaling, &WsSignalingClient::jsonReceived,
            this, &PeerConnectionManager::onSignalingMessage);
}

void PeerConnectionManager::start()
{
    createPeerConnection();
    qDebug() << "createPeerConnection done....";

    if (m_isCaller) {

        m_dc = m_pc->createDataChannel("chat");
        qDebug() << "m_pc->createDataChannel done....";
        setupDataChannel(m_dc);
        // qDebug() << "setupDataChannel....";
        // qDebug() << "m_pc == nullptr ?" << (m_pc ? "NO" : "YES");
        m_pc->setLocalDescription(); 
        // qDebug() << "setLocalDescription....";
        
    }
}

void PeerConnectionManager::createPeerConnection()
{
    rtc::Configuration config;
    config.iceServers.clear();

    m_pc = std::make_shared<rtc::PeerConnection>(config);

    // ---------- SDP ----------
    m_pc->onLocalDescription([this](rtc::Description desc) {
        handleLocalDescription(std::make_shared<rtc::Description>(std::move(desc)));
    });

    // ---------- ICE ----------
    m_pc->onLocalCandidate([this](rtc::Candidate cand) {
        handleLocalCandidate(std::make_shared<rtc::Candidate>(std::move(cand)));
    });

    // 
    m_pc->onStateChange([this](rtc::PeerConnection::State state) {
        qDebug() << "PeerConnection state:" << (int)state;
        if (state == rtc::PeerConnection::State::Connected) {
            emit connected();
        } else if (state == rtc::PeerConnection::State::Disconnected ||
                   state == rtc::PeerConnection::State::Failed ||
                   state == rtc::PeerConnection::State::Closed) {
            emit disconnected();
        }
    });

    // ----------  DataChannel ----------
    m_pc->onDataChannel([this](std::shared_ptr<rtc::DataChannel> dc) {
        qDebug() << "Remote DataChannel received";
        m_dc = dc;
        setupDataChannel(m_dc);
    });
}

void PeerConnectionManager::setupDataChannel(const std::shared_ptr<rtc::DataChannel>& dc)
{
    if (!dc) return;

    dc->onOpen([dc]() {
        qDebug() << "DataChannel open";
    });

    dc->onClosed([]() {
        qDebug() << "DataChannel closed";
    });

    dc->onError([this](std::string err) {
        emit errorOccurred(QString::fromStdString(err));
    });

    dc->onMessage([this](rtc::message_variant msg) {
        if (const std::string* str = std::get_if<std::string>(&msg)) {
            emit messageReceived(QString::fromStdString(*str));
        }
    });
}

void PeerConnectionManager::handleLocalDescription(std::shared_ptr<rtc::Description> desc)
{
    QJsonObject obj;
    obj["type"] = QString::fromStdString(desc->typeString());
    obj["sdp"]  = QString::fromStdString(std::string(desc->generateSdp()));
    m_signaling->sendJson(obj);
}

void PeerConnectionManager::handleLocalCandidate(std::shared_ptr<rtc::Candidate> cand)
{
    QJsonObject obj;
    obj["type"] = "candidate";
    obj["candidate"] = QString::fromStdString(cand->candidate());
    obj["mid"] = QString::fromStdString(cand->mid());
    // obj["mlineindex"] = cand->mlineIndex();
    m_signaling->sendJson(obj);
}

void PeerConnectionManager::onSignalingMessage(const QJsonObject& obj)
{
    const QString type = obj.value("type").toString();

    if (type == "offer" || type == "answer") {
        std::string sdp = obj.value("sdp").toString().toStdString();
        std::string stype = type.toStdString();
        rtc::Description desc(sdp, stype);
        m_pc->setRemoteDescription(desc);

        // 
        if (!m_isCaller && type == "offer") {
            m_pc->setLocalDescription();  // 
        } 
    }else if (type == "candidate") {
        std::string candidate = obj.value("candidate").toString().toStdString();
        std::string mid = obj.value("mid").toString().toStdString();
        // int mlineindex = obj.value("mlineindex").toInt();

        rtc::Candidate cand(candidate, mid);
        m_pc->addRemoteCandidate(cand);
    }
}


void PeerConnectionManager::sendMessage(const QString& msg)
{
    if (!m_dc) {
        qDebug() << "DataChannel not ready, cannot send";
        return;
    }
    m_dc->send(msg.toStdString());
}
