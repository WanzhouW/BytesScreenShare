#ifndef VOICECHAT_H
#define VOICECHAT_H

#include <QObject>
#include <QtMultimedia/QAudioSource>
#include <QtMultimedia/QAudioSink>
#include <QtMultimedia/QMediaDevices>
#include <QIODevice>
#include <QDebug>
#include <rtc/rtc.hpp> // libdatachannel 头文件

class VoiceChat : public QObject
{
    Q_OBJECT
public:
    explicit VoiceChat(QObject* parent = nullptr);
    ~VoiceChat();

    void initAudio();

    // 作为主叫（Offerer）
    void startAsOfferer();
    // 作为被叫（Answerer）
    void startAsAnswerer();

    // Offer 端收到 Answer
    void setRemoteAnswer(const QString& sdp);
    // Answer 端收到 Offer
    void setRemoteOffer(const QString& sdp);

    void addRemoteCandidate(const QString& candidateJson);

signals:
    void localDescriptionGenerated(const QString& sdp, int type); // 0=offer,1=pranswer,2=answer...
    void localCandidateGenerated(const QString& candidateJson);
    void connectionStateChanged(const QString& state);

private slots:
    void onAudioRecorded();

private:
    std::shared_ptr<rtc::PeerConnection> pc;
    std::shared_ptr<rtc::DataChannel>    dc;

    void setupPeerConnection();
    void setupDataChannel();

    QAudioSource* audioInput = nullptr;
    QAudioSink* audioOutput = nullptr;
    QIODevice* inputDevice = nullptr;
    QIODevice* outputDevice = nullptr;
    QAudioFormat   format;
};

#endif // VOICECHAT_H