#pragma once
// ===============================
// 共享屏幕示例主窗口（仿腾讯会议布局）
// 功能：
//  - 语音开关、屏幕共享
//  - 聊天面板（支持收/发、未读提醒）
//  - 参会者列表面板
//  - 录制开关（支持录制音视频到文件）、举手、设备菜单、离开
//  - 快捷键（Ctrl+D麦克风、Ctrl+E摄像头、Ctrl+S共享、Ctrl+H聊天、Ctrl+P参会者、Ctrl+R录制）
// 说明：集成了真实的音视频录制功能，使用 QMediaRecorder
// ===============================
#include <QMainWindow>
#include "VideoOpenGLWidget.h"
#include <QPointer>
#include <QLabel>
#include <QTime>
#include <QtMultimedia/QCamera>
#include <QtMultimedia/QMediaDevices>
#include <QtMultimedia/QMediaCaptureSession>
#include <QtMultimedia/QMediaRecorder>
#include <QtMultimedia/QAudioInput>
#include <QtMultimediaWidgets/QVideoWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class shared_screen;
}
QT_END_NAMESPACE

class QDockWidget;
class QListWidget;
class QPushButton;
class QLabel;
class QWidget;
class QTimer;
class QShortcut;
class QMenu;

class shared_screen : public QMainWindow
{
    Q_OBJECT
public:
    explicit shared_screen(QWidget *parent = nullptr);
    ~shared_screen();

protected:
    // 空格"按住说话"演示（按下临时开麦，松开恢复）
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    // 已有按钮
    void on_btnVoiceClicked();
    void on_btnShareScreenClicked();
    void on_btnChatClicked();
    void on_btnSendClicked();

    // 新增功能按钮
    void on_btnVideoClicked();
    void on_btnParticipantsClicked();
    void on_btnRecordClicked();
    void on_btnRaiseHandClicked();
    void on_btnLeaveClicked();

    // 模拟网络质量变化/远端消息到达
    void on_fakeNetworkTick();
    void on_fakeRemoteMsg();
    void on_btnJoinMeetingClicked();

    // 录制相关槽函数
    void onRecorderStateChanged(QMediaRecorder::RecorderState state);
    void onRecorderError(QMediaRecorder::Error error, const QString &errorString);
    void onRecordingDurationChanged(qint64 duration);
    void updateRecordingTime();
    void captureScreen();
    //模拟视频流槽函数
    void onSimulateVideoFrame();

private:
    // ===== 帮助函数 =====
    void toggleChatPanel();
    void ensureParticipantsDock();
    void appendSystemMessage(const QString &text);
    void appendRemoteMessage(const QString &sender, const QString &text);
    void updateChatBadge();
    void buildShortcuts();
    void diagnoseMultimediaSupport();

    // 录制功能
    void startRecording();
    void stopRecording();
    void saveRecordedFile();

    QString iconBasePath;

private:
    Ui::shared_screen *ui;

    // 状态管理
    bool isChatVisible{false};
    bool isVoiceOn{false};
    bool isScreenSharing{false};
    bool isCameraOn{false};
    bool isRecording{false};
    bool isHandRaised{false};
    bool spaceHeldPTT{false}; // 按住说话（Push-To-Talk）

    int unreadCount{0};

    // 动态创建的部件
    QPointer<QDockWidget> dockParticipants;
    QPointer<QListWidget> participantsList;
    QPointer<QWidget> participantsWidget;

    // 音视频组件
    QCamera *camera = nullptr;
    QMediaCaptureSession *captureSession = nullptr;
    QVideoWidget *videoWidget = nullptr;
    QMediaRecorder *mediaRecorder = nullptr;
    QAudioInput *audioInput = nullptr;

    // 录制状态
    QString currentRecordingPath;
    QTime recordingStartTime;
    QTimer *recordingTimer = nullptr;
    QTimer *screenCaptureTimer = nullptr;

    // 底部附加控件
    QPointer<QPushButton> btnVoice;
    QPointer<QPushButton> btnChat;
    QPointer<QPushButton> btnShareScreen;
    QPointer<QPushButton> btnVideo;
    QPointer<QPushButton> btnParticipants;
    QPointer<QPushButton> btnRecord;
    QPointer<QPushButton> btnRaiseHand;
    QPointer<QPushButton> btnLeave;
    QPointer<QLabel> netLabel;

    // 定时器
    QPointer<QTimer> netTimer;
    QPointer<QTimer> simMsgTimer;

    // 首页覆盖层（运行时显示，点击加入会议后隐藏）
    QPointer<QWidget> homeWidget;
    QPointer<QPushButton> joinButton;
    bool joined{false};

//测试渲染模块的一些添加的变量
private:
    VideoOpenGLWidget* m_glScreenWidget = nullptr;
    QTimer* m_simulationTimer = nullptr;
    QTimer* m_patternSwitchTimer = nullptr;
    int m_currentPatternType = 0;
};