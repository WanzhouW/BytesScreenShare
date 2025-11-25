#pragma once
// ===============================
// 共享屏幕示例主窗口（仿腾讯会议布局）
// 功能：
//  - 语音开关、屏幕共享
//  - 聊天面板（支持收/发、未读提醒）
//  - 参会者列表面板
//  - 录制开关、举手、设备菜单、离开
//  - 快捷键（Ctrl+D麦克风、Ctrl+E摄像头、Ctrl+S共享、Ctrl+H聊天、Ctrl+P参会者、Ctrl+R录制）
// 说明：纯前端模拟，未接入真实音视频采集/编解码/传输，便于快速集成实际 SDK（WebRTC/腾讯实时音视频TRTC等）。
// ===============================
#include <QMainWindow>
#include <QPointer>

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
    // 空格“按住说话”演示（按下临时开麦，松开恢复）
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    // 已有按钮
    void on_btnVoice_clicked();
    void on_btnShareScreen_clicked();
    void on_btnChat_clicked();
    void on_btnSend_clicked();

    // 新增功能按钮
    void on_btnVideo_clicked();
    void on_btnParticipants_clicked();
    void on_btnRecord_clicked();
    void on_btnRaiseHand_clicked();
    void on_btnDevices_clicked();
    void on_btnLeave_clicked();

    // 模拟网络质量变化/远端消息到达
    void on_fakeNetworkTick();
    void on_fakeRemoteMsg();

private:
    // ===== 帮助函数 =====
    void toggleChatPanel();
    void ensureParticipantsDock();
    void appendSystemMessage(const QString &text);
    void appendRemoteMessage(const QString &sender, const QString &text);
    void updateChatBadge();
    void buildShortcuts();
    QMenu *buildDevicesMenu();

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

    // 底部附加控件
    QPointer<QPushButton> btnVoice;
    QPointer<QPushButton> btnChat;
    QPointer<QPushButton> btnShareScreen;
    QPointer<QPushButton> btnVideo;
    QPointer<QPushButton> btnParticipants;
    QPointer<QPushButton> btnRecord;
    QPointer<QPushButton> btnRaiseHand;
    QPointer<QPushButton> btnDevices;
    QPointer<QPushButton> btnLeave;
    QPointer<QLabel> netLabel;

    // 定时器
    QPointer<QTimer> netTimer;
    QPointer<QTimer> simMsgTimer;
};
