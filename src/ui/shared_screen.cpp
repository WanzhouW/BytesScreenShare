#include "shared_screen.h"
#include <ui_shared_screen.h>
#include <QDateTime>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QShortcut>
#include <QTextBrowser>
#include <QTimer>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QDebug>

#include "signaling/TcpSignalingClient.hpp"
#include "rtc/PeerConnectionManager.hpp"


shared_screen::shared_screen(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::shared_screen)
{
    ui->setupUi(this);

    // ================== P2P 初始化 ==================
    qDebug() << "=== P2P Start ===";
    m_isCaller = true;  // 这台机器作为 Caller；另一台你可以改成 false
    m_signaling = new TcpSignalingClient(this);
    // 假设信令服务器在本机 127.0.0.1:12345
    m_signaling->connectToServer("127.0.0.1", 12345);
    qDebug() << "connectToServer done....";
    m_pcm = new PeerConnectionManager(m_signaling, m_isCaller, this);
    qDebug() << "new PeerConnectionManager done....";
    m_pcm->start();
    qDebug() << "m_pcm->start() done....";

    // ... P2P connect 信号槽 ...
    connect(m_pcm, &PeerConnectionManager::connected, this, [this]() {
        ui->statusLabel->setText(u8"状态：P2P 已连接");
    });
    connect(m_pcm, &PeerConnectionManager::disconnected, this, [this]() {
        ui->statusLabel->setText(u8"状态：P2P 已断开");
    });
    connect(m_pcm, &PeerConnectionManager::errorOccurred, this, [this](const QString& err) {
        ui->statusLabel->setText(u8"错误：" + err);
    });
    connect(m_pcm, &PeerConnectionManager::messageReceived, this, [this](const QString& msg) {
        // 收到对方消息，显示为“远端”
        appendRemoteMessage(u8"对方", msg);
    });


    // ====== 初始界面 ======
    ui->dockChat->hide(); // 初始隐藏聊天面板
    ui->statusLabel->setText("状态：未连接");

    // ====== 底部控制栏：动态扩展按钮（仿腾讯会议） ======
    // 依次添加：摄像头、参会者、录制、举手、设备、离开、网络质量
    btnShareScreen = new QPushButton(u8"📺", this);
    btnChat = new QPushButton(u8"💬", this);
    btnVoice = new QPushButton(u8"🎤", this);
    btnVideo = new QPushButton(u8"🎥", this);
    btnParticipants = new QPushButton(u8"👥", this);
    btnRecord = new QPushButton(u8"⏺", this);
    btnRaiseHand = new QPushButton(u8"✋", this);
    btnDevices = new QPushButton(u8"⚙", this);
    btnLeave = new QPushButton(u8"🚪 离开会议", this);
    netLabel = new QLabel(u8"网络：良好", this);

    // 开关型按钮
    btnVoice->setCheckable(true);
    btnVoice->setChecked(isCameraOn);
    btnShareScreen->setCheckable(true);
    btnVideo->setCheckable(true);
    btnRecord->setCheckable(true);

    // 外观微调
    btnLeave->setStyleSheet("QPushButton{background:#ff3b30;color:white;border-radius:8px;padding:10px 18px;}"
                            "QPushButton:hover{background:#ff453a;}");
    netLabel->setStyleSheet("QLabel{color:#8e8e93;padding-left:6px;}");

    // 设备菜单（选择/设置）
    btnDevices->setMenu(buildDevicesMenu());

    // 挂到底部布局
    auto bar = ui->horizontalLayout;
    bar->addWidget(btnVoice);
    bar->addWidget(btnChat);
    bar->addWidget(btnShareScreen);
    bar->addWidget(btnVideo);
    bar->addWidget(btnParticipants);
    bar->addWidget(btnRecord);
    bar->addWidget(btnRaiseHand);
    bar->addWidget(btnDevices);
    bar->addWidget(btnLeave);
    bar->addWidget(netLabel);

    // 连接新增按钮
    connect(btnChat, &QPushButton::clicked, this, &shared_screen::on_btnChat_clicked);
    connect(btnVoice, &QPushButton::clicked, this, &shared_screen::on_btnVoice_clicked);
    connect(btnShareScreen, &QPushButton::clicked, this, &shared_screen::on_btnShareScreen_clicked);
    connect(btnVideo, &QPushButton::clicked, this, &shared_screen::on_btnVideo_clicked);
    connect(btnParticipants, &QPushButton::clicked, this, &shared_screen::on_btnParticipants_clicked);
    connect(btnRecord, &QPushButton::clicked, this, &shared_screen::on_btnRecord_clicked);
    connect(btnRaiseHand, &QPushButton::clicked, this, &shared_screen::on_btnRaiseHand_clicked);
    connect(btnDevices, &QPushButton::clicked, this, &shared_screen::on_btnDevices_clicked);
    connect(btnLeave, &QPushButton::clicked, this, &shared_screen::on_btnLeave_clicked);

    // ====== 模拟网络质量 & 远端消息 ======
    netTimer = new QTimer(this);
    netTimer->setInterval(4000);
    connect(netTimer, &QTimer::timeout, this, &shared_screen::on_fakeNetworkTick);
    netTimer->start();

    simMsgTimer = new QTimer(this);
    simMsgTimer->setInterval(15000);
    connect(simMsgTimer, &QTimer::timeout, this, &shared_screen::on_fakeRemoteMsg);
    simMsgTimer->start();

    // ====== 快捷键 ======
    buildShortcuts();

    // 初始屏幕提示
    ui->screenPreview->setText(u8"屏幕预览区域\n点击“共享屏幕”开始");
}

shared_screen::~shared_screen()
{
    delete ui;
}

// =============== 已有功能 ===============
void shared_screen::on_btnVoice_clicked()
{

    isVoiceOn = !isVoiceOn;
    btnVoice->setChecked(isVoiceOn);
    if (isVoiceOn)
    {
        btnVoice->setIcon(QIcon("src/icons/录音_voice.png"));
        ui->statusLabel->setText(u8"麦克风已开启");
    }
    else
    {
        btnVoice->setIcon(QIcon("src/icons/关闭录音_voice-off.png"));
        ui->statusLabel->setText(u8"麦克风已关闭");
    }
    // qDebug() << "slot:on_btnVoice_clicked called";
}

void shared_screen::on_btnShareScreen_clicked()
{

    isScreenSharing = !isScreenSharing;
    btnShareScreen->setChecked(isScreenSharing);

    if (isScreenSharing)
    {
        btnShareScreen->setText(u8"⏹");
        ui->screenPreview->setText(u8"正在共享屏幕...");
        ui->statusLabel->setText(u8"正在共享屏幕");
    }
    else
    {
        btnShareScreen->setText(u8"📺");
        ui->screenPreview->setText(u8"屏幕预览区域\n点击“共享屏幕”开始");
        ui->statusLabel->setText(u8"未共享");
    }
    // qDebug() << "slot:on_btnShareScreen_clicked called";
}

void shared_screen::on_btnChat_clicked()
{

    toggleChatPanel();
}

void shared_screen::on_btnSend_clicked()
{
    const QString text = ui->chatInput->text().trimmed();
    if (text.isEmpty())
        return;

    // 时间戳 + 我的消息
    const QString ts = QDateTime::currentDateTime().toString("hh:mm");
    const QString html = QString("<div style='margin:8px 0;'>"
                                 "<span style='color:#888;font-size:11px;'>%1</span> "
                                 "<span style='color:#1a73e8;font-weight:bold;'>我：</span>"
                                 "<span style='color:#fff;'>%2</span>"
                                 "</div>")
                             .arg(ts, text);
    ui->chatView->append(html);
    ui->chatInput->clear();
    ui->chatInput->setFocus();

    // ===== 新增：通过 P2P 发送到对方 =====
    if (m_pcm) {
        m_pcm->sendMessage(text);
    }
}


// =============== 新增功能 ===============
void shared_screen::on_btnVideo_clicked()
{
    isCameraOn = !isCameraOn;
    btnVideo->setChecked(isCameraOn);
    if (isCameraOn)
    {
        btnVideo->setText(u8"🎥");

        appendSystemMessage(u8"你已开启摄像头");
    }
    else
    {
        btnVideo->setText(u8"🎥");
        appendSystemMessage(u8"你已关闭摄像头");
    }
}

void shared_screen::on_btnParticipants_clicked()
{
    ensureParticipantsDock();
    if (dockParticipants->isHidden())
    {
        dockParticipants->show();
        btnParticipants->setText(u8"👥✓");
    }
    else
    {
        dockParticipants->hide();
        btnParticipants->setText(u8"👥");
    }
}

void shared_screen::on_btnRecord_clicked()
{
    isRecording = !isRecording;
    btnRecord->setChecked(isRecording);
    if (isRecording)
    {
        btnRecord->setText(u8"⏺");
        ui->statusLabel->setText(u8"正在录制");
        appendSystemMessage(u8"会议录制已开始（演示）");
    }
    else
    {
        btnRecord->setText(u8"⏺");
        ui->statusLabel->setText(u8"录制结束");
        appendSystemMessage(u8"会议录制已停止（演示）");
    }
}

void shared_screen::on_btnRaiseHand_clicked()
{
    isHandRaised = !isHandRaised;
    if (isHandRaised)
    {
        btnRaiseHand->setText(u8"✋");
        appendSystemMessage(u8"你举手了");
    }
    else
    {
        btnRaiseHand->setText(u8"✋");
        appendSystemMessage(u8"你放下了手");
    }
}

void shared_screen::on_btnDevices_clicked()
{
    // 这里使用菜单动作即可，真实项目可打开设置对话框
    // 演示在系统消息中提示
    appendSystemMessage(u8"打开设备设置（演示）：切换麦克风/扬声器/摄像头");
}

void shared_screen::on_btnLeave_clicked()
{
    if (QMessageBox::question(this, u8"离开会议", u8"确定要离开会议吗？") == QMessageBox::Yes)
    {
        close();
    }
}

// =============== 模拟器 ===============
void shared_screen::on_fakeNetworkTick()
{
    // 随机网络质量：优、良、中、差
    static const char *labels[] = {"优", "良", "中", "差"};
    int idx = QRandomGenerator::global()->bounded(0, 4);
    QString s = u8"网络：" + QString::fromUtf8(labels[idx]);

    // 颜色提示
    QString color = "#34c759"; // 优
    if (idx == 1)
        color = "#0a84ff"; // 良
    if (idx == 2)
        color = "#ffd60a"; // 中
    if (idx == 3)
        color = "#ff3b30"; // 差
    netLabel->setStyleSheet(QString("QLabel{color:%1;padding-left:6px;}").arg(color));
    netLabel->setText(s);
}

void shared_screen::on_fakeRemoteMsg()
{
    // 如果聊天面板关闭，模拟远端消息 + 未读角标
    appendRemoteMessage(u8"小张", u8"收到~");
    if (!isChatVisible)
    {
        unreadCount++;
        updateChatBadge();
    }
}

// =============== 私有帮助函数 ===============
void shared_screen::toggleChatPanel()
{
    isChatVisible = !isChatVisible;
    if (isChatVisible)
    {
        ui->dockChat->show();
        btnChat->setText(u8"💬 ✓");
        unreadCount = 0;
        updateChatBadge();
    }
    else
    {
        ui->dockChat->hide();
        btnChat->setText(u8"💬");
    }
}

void shared_screen::ensureParticipantsDock()
{
    if (dockParticipants)
        return;

    dockParticipants = new QDockWidget(u8"👥 参会者", this);
    participantsList = new QListWidget(dockParticipants);
    dockParticipants->setWidget(participantsList);
    dockParticipants->setMinimumWidth(260);
    addDockWidget(Qt::RightDockWidgetArea, dockParticipants);

    // 先放一些演示数据
    participantsList->addItem(u8"我（主持人）");
    participantsList->addItem(u8"小张");
    participantsList->addItem(u8"小李");

    dockParticipants->hide();
}

void shared_screen::appendSystemMessage(const QString &text)
{
    const QString ts = QDateTime::currentDateTime().toString("hh:mm");
    const QString html = QString("<div style='margin:8px 0;'>"
                                 "<span style='color:#888;font-size:11px;'>%1</span> "
                                 "<span style='color:#e6e6e6;font-weight:bold;'>系统：</span>"
                                 "<span style='color:#aaa;'>%2</span>"
                                 "</div>")
                             .arg(ts, text.toHtmlEscaped());
    ui->chatView->append(html);
}

void shared_screen::appendRemoteMessage(const QString &sender, const QString &text)
{
    const QString ts = QDateTime::currentDateTime().toString("hh:mm");
    const QString html = QString("<div style='margin:8px 0;'>"
                                 "<span style='color:#888;font-size:11px;'>%1</span> "
                                 "<span style='color:#40c4ff;font-weight:bold;'>%2：</span>"
                                 "<span style='color:#fff;'>%3</span>"
                                 "</div>")
                             .arg(ts, sender.toHtmlEscaped(), text.toHtmlEscaped());
    ui->chatView->append(html);
}

void shared_screen::updateChatBadge()
{
    if (unreadCount > 0)
    {
        btnChat->setText(QString(u8"💬(%1)").arg(unreadCount));
    }
    else
    {
        btnChat->setText(u8"💬");
    }
}

void shared_screen::buildShortcuts()
{
    // Ctrl+D：麦克风
    auto s1 = new QShortcut(QKeySequence("Ctrl+D"), this);
    connect(s1, &QShortcut::activated, this, &shared_screen::on_btnVoice_clicked);

    // Ctrl+E：摄像头
    auto s2 = new QShortcut(QKeySequence("Ctrl+E"), this);
    connect(s2, &QShortcut::activated, this, &shared_screen::on_btnVideo_clicked);

    // Ctrl+S：共享屏幕
    auto s3 = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(s3, &QShortcut::activated, this, &shared_screen::on_btnShareScreen_clicked);

    // Ctrl+H：聊天
    auto s4 = new QShortcut(QKeySequence("Ctrl+H"), this);
    connect(s4, &QShortcut::activated, this, &shared_screen::on_btnChat_clicked);

    // Ctrl+P：参会者
    auto s5 = new QShortcut(QKeySequence("Ctrl+P"), this);
    connect(s5, &QShortcut::activated, this, &shared_screen::on_btnParticipants_clicked);

    // Ctrl+R：录制
    auto s6 = new QShortcut(QKeySequence("Ctrl+R"), this);
    connect(s6, &QShortcut::activated, this, &shared_screen::on_btnRecord_clicked);
}

QMenu *shared_screen::buildDevicesMenu()
{
    auto m = new QMenu(this);
    m->addAction(u8"选择麦克风...");
    m->addAction(u8"选择扬声器...");
    m->addAction(u8"选择摄像头...");
    m->addSeparator();
    m->addAction(u8"打开设备设置...");
    return m;
}

// =============== “按住说话”演示 ===============
void shared_screen::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat())
    {
        if (!spaceHeldPTT)
        {
            spaceHeldPTT = true;
            if (!isVoiceOn)
            {
                // 临时开麦
                on_btnVoice_clicked();
                appendSystemMessage(u8"按住空格键：临时开麦");
            }
        }
    }
    QMainWindow::keyPressEvent(event);
}

void shared_screen::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat())
    {
        if (spaceHeldPTT)
        {
            spaceHeldPTT = false;
            if (isVoiceOn)
            {
                // 松开后还原（若需要可判断之前是否是关麦）
                on_btnVoice_clicked();
                appendSystemMessage(u8"松开空格键：还原为静音");
            }
        }
    }
    QMainWindow::keyReleaseEvent(event);
}
