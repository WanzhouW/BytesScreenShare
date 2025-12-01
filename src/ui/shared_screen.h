#ifndef SHARED_SCREEN_H
#define SHARED_SCREEN_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QDockWidget>
#include <QTimer>
#include <QLineEdit>
#include <QTextBrowser>

namespace Ui {
class shared_screen;
}

class TcpSignalingClient;
class PeerConnectionManager;

class shared_screen : public QMainWindow
{
    Q_OBJECT

public:
    explicit shared_screen(QWidget *parent = nullptr); // 构造函数
    ~shared_screen();

    // 这里省略你原来已有的 public / slots 声明...

private slots:
    // 槽函数声明
    void on_btnVoice_clicked();
    void on_btnShareScreen_clicked();
    void on_btnChat_clicked();
    void on_btnSend_clicked();
    void on_btnVideo_clicked();
    void on_btnParticipants_clicked();
    void on_btnRecord_clicked();
    void on_btnRaiseHand_clicked();
    void on_btnDevices_clicked();
    void on_btnLeave_clicked();
    
    // 模拟器槽函数
    void on_fakeNetworkTick();
    void on_fakeRemoteMsg();

protected:
    // 键盘事件处理
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    
private:
    Ui::shared_screen *ui;

    // ===== 新增：P2P 相关 =====
    TcpSignalingClient* m_signaling = nullptr;
    PeerConnectionManager* m_pcm = nullptr;
    bool m_isCaller = true;   // 先简单固定：一端 true，一端 false

    // UI 控件声明
    QPushButton *btnVoice = nullptr;
    QPushButton *btnChat = nullptr;
    QPushButton *btnShareScreen = nullptr;
    QPushButton *btnVideo = nullptr;
    QPushButton *btnParticipants = nullptr;
    QPushButton *btnRecord = nullptr;
    QPushButton *btnRaiseHand = nullptr;
    QPushButton *btnDevices = nullptr;
    QPushButton *btnLeave = nullptr;

    // 标签
    QLabel *netLabel = nullptr;

    // 浮动窗口部件
    QDockWidget *dockParticipants = nullptr;
    QListWidget *participantsList = nullptr;

    // 定时器
    QTimer *netTimer = nullptr;
    QTimer *simMsgTimer = nullptr;

    // 状态变量声明
    bool isVoiceOn = false;
    bool isScreenSharing = false;
    bool isCameraOn = false;
    bool isRecording = false;
    bool isHandRaised = false;
    bool isChatVisible = false;
    bool spaceHeldPTT = false;
    int unreadCount = 0;

    // 辅助方法声明
    void toggleChatPanel();
    void ensureParticipantsDock();
    void appendSystemMessage(const QString &text);
    void appendRemoteMessage(const QString &sender, const QString &text);
    void updateChatBadge();
    void buildShortcuts();
    QMenu *buildDevicesMenu();
};
#endif