#include "shared_screen.h"
#include <ui_shared_screen.h>

// 添加诊断函数
void shared_screen::log(const QString& msg) {
    // 如果当前不在主线程，转发给主线程
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, [this, msg]() { log(msg); }, Qt::QueuedConnection);
        return;
    }
    // logView->append(msg);
}

void shared_screen::diagnoseMultimediaSupport()
{
    qDebug() << "=== Qt Multimedia 完整诊断信息 ===";
    qDebug() << "Qt 版本:" << QT_VERSION_STR;
    qDebug() << "Qt 运行时版本:" << qVersion();
    qDebug() << "应用程序路径:" << QCoreApplication::applicationDirPath();
    qDebug() << "插件搜索路径:" << QCoreApplication::libraryPaths();

    // 检查可用的音视频设备
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    const QList<QAudioDevice> audioInputs = QMediaDevices::audioInputs();
    const QList<QAudioDevice> audioOutputs = QMediaDevices::audioOutputs();

    qDebug() << "\n--- 设备检测 ---";
    qDebug() << "找到的摄像头数量:" << cameras.size();
    qDebug() << "找到的音频输入设备数量:" << audioInputs.size();
    qDebug() << "找到的音频输出设备数量:" << audioOutputs.size();

    // 详细列出摄像头
    for (int i = 0; i < cameras.size(); ++i)
    {
        const QCameraDevice &camera = cameras[i];
        qDebug() << "摄像头" << i << ":";
        qDebug() << "  - ID:" << camera.id();
        qDebug() << "  - 描述:" << camera.description();
        qDebug() << "  - 是否默认:" << camera.isDefault();
        qDebug() << "  - 位置:" << camera.position();

        // 列出支持的分辨率
        auto formats = camera.videoFormats();
        qDebug() << "  - 支持的格式数量:" << formats.size();
        if (!formats.isEmpty())
        {
            qDebug() << "  - 第一个格式:" << formats.first().resolution()
                     << "@" << formats.first().maxFrameRate() << "fps";
        }
    }

    // 列出音频设备（验证 QtMultimedia 是否正常工作）
    if (!audioInputs.isEmpty())
    {
        qDebug() << "\n音频输入设备:";
        for (const auto &device : audioInputs)
        {
            qDebug() << "  -" << device.description();
        }
    }

    if (!audioOutputs.isEmpty())
    {
        qDebug() << "\n音频输出设备:";
        for (const auto &device : audioOutputs)
        {
            qDebug() << "  -" << device.description();
        }
    }

    // 检查 multimedia 插件
    qDebug() << "\n--- 插件检查 ---";
    QString pluginPath = QCoreApplication::applicationDirPath() + "/multimedia";
    QDir pluginDir(pluginPath);
    qDebug() << "Multimedia 插件目录:" << pluginPath;
    qDebug() << "目录是否存在:" << pluginDir.exists();

    if (pluginDir.exists())
    {
        QStringList plugins = pluginDir.entryList(QDir::Files);
        qDebug() << "找到的插件文件:" << plugins;

        // 尝试加载插件
        for (const QString &pluginFile : plugins)
        {
            QString fullPath = pluginDir.absoluteFilePath(pluginFile);
            QPluginLoader loader(fullPath);
            qDebug() << "\n尝试加载插件:" << pluginFile;

            if (loader.load())
            {
                qDebug() << "  [成功] 插件加载成功";
                QObject *plugin = loader.instance();
                if (plugin)
                {
                    qDebug() << "  插件实例创建成功";
                    qDebug() << "  插件元数据:" << loader.metaData();
                }
            }
            else
            {
                qDebug() << "  [失败] 插件加载失败";
                qDebug() << "  错误信息:" << loader.errorString();
            }
        }
    }

    // 检查 FFmpeg DLL
    qDebug() << "\n--- FFmpeg 库检查 ---";
    QDir appDir(QCoreApplication::applicationDirPath());
    QStringList ffmpegDlls = appDir.entryList(QStringList() << "av*.dll" << "sw*.dll", QDir::Files);
    qDebug() << "找到的 FFmpeg DLL:" << ffmpegDlls;

    if (ffmpegDlls.isEmpty())
    {
        qDebug() << "[警告] 未找到 FFmpeg DLL 文件!";
    }

    // 检查 Windows 平台插件
    qDebug() << "\n--- 平台插件检查 ---";
    QDir platformsDir(QCoreApplication::applicationDirPath() + "/platforms");
    if (platformsDir.exists())
    {
        QStringList platformPlugins = platformsDir.entryList(QDir::Files);
        qDebug() << "平台插件:" << platformPlugins;
    }
    else
    {
        qDebug() << "[警告] platforms 目录不存在";
    }

    qDebug() << "=====================================\n";
}

shared_screen::shared_screen(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::shared_screen)
    , pcMgr(new PeerConnectionManager(this))
    , isConnected(false)
{
    ui->setupUi(this);

    // 运行诊断
    // diagnoseMultimediaSupport();

    // ====== 初始界面 ======
    ui->stackedWidget->setCurrentIndex(0);
    ui->dockChat->hide();
    ui->statusLabel->setText("状态:未连接");

    captureSession = new QMediaCaptureSession(this);

    // 初始化录制计时器
    recordingTimer = new QTimer(this);
    recordingTimer->setInterval(1000); // 每秒更新一次
    connect(recordingTimer, &QTimer::timeout, this, &shared_screen::updateRecordingTime);

    // 初始化屏幕捕获定时器（用于录制屏幕）
    screenCaptureTimer = new QTimer(this);
    screenCaptureTimer->setInterval(33); // ~30fps
    connect(screenCaptureTimer, &QTimer::timeout, this, &shared_screen::captureScreen);

    // ====== 底部控制栏按钮 ======
    btnVoice = new QPushButton(this);
    btnVoice->setIcon(QIcon("../../src/icons/voice-off.png"));
    btnVoice->setIconSize(QSize(32, 32));
    btnVoice->setToolTip(u8"开启/关闭麦克风");

    btnShareScreen = new QPushButton(this);
    btnShareScreen->setIcon(QIcon("../../src/icons/monitor-one.png"));
    btnShareScreen->setIconSize(QSize(32, 32));
    btnShareScreen->setToolTip(u8"开始/停止共享屏幕");

    btnChat = new QPushButton(this);
    btnChat->setIcon(QIcon("../../src/icons/message.png"));
    btnChat->setIconSize(QSize(32, 32));
    btnChat->setToolTip(u8"显示/隐藏聊天面板");

    btnVideo = new QPushButton(this);
    btnVideo->setIcon(QIcon("../../src/icons/camera-one.png"));
    btnVideo->setIconSize(QSize(32, 32));
    btnVideo->setToolTip(u8"开启/关闭摄像头");

    btnParticipants = new QPushButton(this);
    btnParticipants->setIcon(QIcon("../../src/icons/participants.png"));
    btnParticipants->setIconSize(QSize(32, 32));
    btnParticipants->setToolTip(u8"参会者");
    btnParticipants->setCheckable(true);

    btnRecord = new QPushButton(this);
    btnRecord->setIcon(QIcon("../../src/icons/facetime.png"));
    btnRecord->setIconSize(QSize(32, 32));
    btnRecord->setToolTip(u8"开始/停止录制");

    btnRaiseHand = new QPushButton(this);
    btnRaiseHand->setIcon(QIcon("../../src/icons/palm.png"));
    btnRaiseHand->setIconSize(QSize(32, 32));
    btnRaiseHand->setToolTip(u8"举手");

    btnLeave = new QPushButton(this);
    btnLeave->setIcon(QIcon("../../src/icons/phone-off.png"));
    btnLeave->setIconSize(QSize(20, 20));
    btnLeave->setText(u8"离开会议");
    netLabel = new QLabel(u8"网络:良好", this);

    btnVoice->setCheckable(true);
    btnVoice->setChecked(isVoiceOn);
    btnShareScreen->setCheckable(true);
    btnShareScreen->setChecked(isScreenSharing);
    btnVideo->setCheckable(true);
    btnVideo->setChecked(isCameraOn);
    btnRecord->setCheckable(true);

    // 按钮样式
    QString iconButtonStyle =
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                stop:0 #ffffff, stop:1 #f5f5f7);"
        "    border: 1px solid #d1d1d6;"
        "    border-radius: 12px;"
        "    padding: 10px;"
        "    min-width: 52px;"
        "    min-height: 52px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                stop:0 #f0f0f5, stop:1 #e8e8ed);"
        "    border: 1px solid #b8b8be;"
        "}"
        "QPushButton:pressed {"
        "    background: #e0e0e5;"
        "    border: 1px solid #a8a8ae;"
        "}"
        "QPushButton:checked {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                stop:0 #0a84ff, stop:1 #0077ed);"
        "    border: 1px solid #0066cc;"
        "}";

    btnVoice->setStyleSheet(iconButtonStyle);
    btnShareScreen->setStyleSheet(iconButtonStyle);
    btnChat->setStyleSheet(iconButtonStyle);
    btnVideo->setStyleSheet(iconButtonStyle);
    btnParticipants->setStyleSheet(iconButtonStyle);
    btnRecord->setStyleSheet(iconButtonStyle);
    btnRaiseHand->setStyleSheet(iconButtonStyle);

    // 离开会议按钮样式
    btnLeave->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                stop:0 #ff3b30, stop:1 #ff2d20);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    padding: 12px 24px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                stop:0 #ff453a, stop:1 #ff3b30);"
        "}"
        "QPushButton:pressed {"
        "    background: #ff2d20;"
        "}");

    // 网络
    netLabel->setStyleSheet(
        "QLabel {"
        "    color: #6e6e73;"
        "    font-size: 12px;"
        "    font-weight: 500;"
        "    padding: 8px 12px;"
        "    background: rgba(255, 255, 255, 0.6);"
        "    border-radius: 6px;"
        "}");

    auto bar = ui->horizontalLayout;
    bar->addWidget(btnVoice);
    bar->addWidget(btnShareScreen);
    bar->addWidget(btnChat);
    bar->addWidget(btnVideo);
    bar->addWidget(btnParticipants);
    bar->addWidget(btnRecord);
    bar->addWidget(btnRaiseHand);
    bar->addWidget(btnLeave);
    bar->addWidget(netLabel);

    connect(ui->btnJoinMeeting, &QPushButton::clicked, this, &shared_screen::on_btnJoinMeetingClicked);
    connect(btnChat, &QPushButton::clicked, this, &shared_screen::on_btnChatClicked);
    connect(btnVoice, &QPushButton::clicked, this, &shared_screen::on_btnVoiceClicked);
    connect(btnShareScreen, &QPushButton::clicked, this, &shared_screen::on_btnShareScreenClicked);
    // connect(btnShareScreen, &QPushButton::clicked, this, &shared_screen::connectSignaling);
    connect(btnVideo, &QPushButton::clicked, this, &shared_screen::on_btnVideoClicked);
    connect(btnParticipants, &QPushButton::clicked, this, &shared_screen::on_btnParticipantsClicked);
    connect(btnRecord, &QPushButton::clicked, this, &shared_screen::on_btnRecordClicked);
    connect(btnRaiseHand, &QPushButton::clicked, this, &shared_screen::on_btnRaiseHandClicked);
    connect(btnLeave, &QPushButton::clicked, this, &shared_screen::on_btnLeaveClicked);

    if (ui->btnSend)
        connect(ui->btnSend, &QPushButton::clicked, this, &shared_screen::on_btnSendClicked);
    connect(ui->chatInput, &QLineEdit::returnPressed, this, &shared_screen::on_btnSendClicked);

    if (ui->chatInput)
        ui->chatInput->setStyleSheet("QLineEdit{color:#000000;}");

    netTimer = new QTimer(this);
    netTimer->setInterval(4000);
    connect(netTimer, &QTimer::timeout, this, &shared_screen::on_fakeNetworkTick);
    netTimer->start();

    simMsgTimer = new QTimer(this);
    simMsgTimer->setInterval(15000);
    connect(simMsgTimer, &QTimer::timeout, this, &shared_screen::on_fakeRemoteMsg);
    simMsgTimer->start();

    buildShortcuts();

    ui->screenPreview->setText(u8"屏幕预览区域\n点击共享屏幕开始");
    ui->dockChat->setFloating(true);
    ui->dockChat->setAllowedAreas(Qt::NoDockWidgetArea);
    ui->dockChat->hide();
}

shared_screen::~shared_screen()
{
    qDebug("~shared_screen destructed");
    // 如果正在录制，先停止
    if (mediaRecorder && mediaRecorder->recorderState() == QMediaRecorder::RecordingState)
    {
        mediaRecorder->stop();

        // 等待一下让文件写入完成
        QEventLoop loop;
        QTimer::singleShot(500, [&loop]()
                           { loop.quit(); });
        loop.exec();
    }

    // 录制时间
    if (recordingTimer)
    {
        recordingTimer->stop();
    }

    if (screenCaptureTimer)
    {
        screenCaptureTimer->stop();
    }

    // 清理临时文件
    if (!currentRecordingPath.isEmpty() && QFile::exists(currentRecordingPath))
    {
        QFile::remove(currentRecordingPath);
        // qDebug() << "清理临时录制文件:" << currentRecordingPath;
    }
    // 摄像头
    if (camera)
    {
        camera->stop();
        delete camera;
        camera = nullptr;
    }

    // 媒体录制器
    if (mediaRecorder)
    {
        delete mediaRecorder;
        mediaRecorder = nullptr;
    }

    if (audioInput)
    {
        delete audioInput;
        audioInput = nullptr;
    }
    // if(m_signaling) m_signaling-> disconnectFromServer();
    delete ui;
}

// =============== 首页功能 ===============
// 加入会议按钮，包含链接服务器的逻辑，链接成功才会跳转
void shared_screen::on_btnJoinMeetingClicked()
{
    // 房间号
    QString url = ui->editRoomId->text().trimmed();
    auto check = [url]() {
        QUrl parsedUrl(url);
        if (!parsedUrl.isValid() || parsedUrl.scheme() != "ws" || parsedUrl.host().isEmpty()) { return false; }
        return true;
    };

    if (url.isEmpty())
    {
        QMessageBox::warning(this, "提示", "请输入信令服务器URL！");
        return;
    }
    else if (!check()) {
        QMessageBox::warning(nullptr, "无效的URL", "请输入一个有效的WebSocket URL，例如: ws://example.com:1234");
        return;
    }

    DEBUG() << "=== Start to connect server ===";
    QObject::connect(pcMgr, &PeerConnectionManager::signalingError, [this](const QString& msg) {
        isConnected = false;
        WARNING() << "信令服务器断开连接, 抛出异常：" << msg;
        QMessageBox::warning(this, "提示", "无法连接服务器");
        return;
        });
    QObject::connect(pcMgr, &PeerConnectionManager::signalingConnected, this, &shared_screen::onConnected);

    pcMgr->onConnectServer(url);
}

void shared_screen::onConnected() {
    isConnected = true;
    qDebug() << "connect success";
    ui->stackedWidget->setCurrentIndex(1);
    ui->statusLabel->setText(u8"状态:未连接");

    isVoiceOn = false;
    isScreenSharing = false;
    isCameraOn = false;
    isRecording = false;
    isHandRaised = false;
    unreadCount = 0;

    ui->dockChat->hide();
    if (dockParticipants)
    {
        dockParticipants->hide();
    }
    btnVoice->setChecked(false);
    btnShareScreen->setChecked(false);
    btnVideo->setChecked(false);
    btnRecord->setChecked(false);
    btnRaiseHand->setChecked(false);
    btnChat->setText("");
    btnVoice->setIcon(QIcon("../../src/icons/voice-off.png"));
}

// 语音按钮
void shared_screen::on_btnVoiceClicked()
{
    isVoiceOn = !isVoiceOn;
    btnVoice->setChecked(isVoiceOn);
    if (isVoiceOn)
    {
        btnVoice->setIcon(QIcon("../../src/icons/voice.png"));
        ui->statusLabel->setText(u8"麦克风已开启");
    }
    else
    {
        btnVoice->setIcon(QIcon("../../src/icons/voice-off.png"));
        ui->statusLabel->setText(u8"麦克风已关闭");
    }
}

// 共享屏幕按钮，点击建立p2p
void shared_screen::on_btnShareScreenClicked()
{
    // startP2P();
    QString targetId = pcMgr->target();
    if (targetId.isEmpty()) {
        QMessageBox::warning(this, "提示", "无其他在线用户");
        return;
    }
    pcMgr->start(targetId);

    isScreenSharing = !isScreenSharing;
    btnShareScreen->setChecked(isScreenSharing);
    
    if (isScreenSharing)
    {
        ui->screenPreview->setText(u8"正在共享屏幕...");
        ui->statusLabel->setText(u8"正在共享屏幕");
    }
    else
    {
        ui->screenPreview->setText(u8"屏幕预览区域\n点击共享屏幕开始");
        ui->statusLabel->setText(u8"未共享");
    }
    // =============== 之后的逻辑 ===============
}

// 聊天按钮
void shared_screen::on_btnChatClicked()
{
    // 聊天框
    toggleChatPanel();
}

// 聊天框发送按钮
void shared_screen::on_btnSendClicked()
{
    const QString text = ui->chatInput->text().trimmed();
    if (text.isEmpty())
        return;

    const QString ts = QDateTime::currentDateTime().toString("hh:mm");
    const QString html = QString("<div style='margin:8px 0;'>"
                                 "<span style='color:#888;font-size:11px;'>%1</span> "
                                 "<span style='color:#1a73e8;font-weight:bold;'>我:</span>"
                                 "<span style='color:#000;'>%2</span>"
                                 "</div>")
                             .arg(ts, text);
    ui->chatView->append(html);
    ui->chatInput->clear();
    ui->chatInput->setFocus();
}

// 点击摄像头按钮
void shared_screen::on_btnVideoClicked()
{
    isCameraOn = !isCameraOn;
    btnVideo->setChecked(isCameraOn);

    ensureParticipantsDock();

    if (isCameraOn)
    {
        // 详细的错误检查
        const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
        const QList<QAudioDevice> audioInputs = QMediaDevices::audioInputs();

        // qDebug() << "\n=== 尝试启动摄像头 ===";
        // qDebug() << "可用摄像头数量:" << cameras.size();
        // qDebug() << "可用音频设备数量:" << audioInputs.size();

        if (cameras.isEmpty())
        {
            QString errorMsg = u8"未找到系统摄像头\n\n";
            errorMsg += u8"Qt 版本: " + QString(QT_VERSION_STR) + "\n";
            errorMsg += u8"Qt 运行时: " + QString(qVersion()) + "\n\n";

            // 检查音频设备以判断 QtMultimedia 是否工作
            if (audioInputs.isEmpty())
            {
                errorMsg += u8"❌ 也未找到音频设备\n";
                errorMsg += u8"这表明 QtMultimedia 后端可能未正常初始化\n\n";
            }
            else
            {
                errorMsg += u8"✓ 找到了 " + QString::number(audioInputs.size()) + u8" 个音频设备\n";
                errorMsg += u8"这表明 QtMultimedia 后端已加载，但无摄像头硬件\n\n";
            }

            errorMsg += u8"可能的原因:\n";
            errorMsg += u8"1. 电脑没有摄像头硬件（或已禁用）\n";
            errorMsg += u8"2. Windows 隐私设置阻止了摄像头访问\n";
            errorMsg += u8"3. 摄像头驱动程序问题\n";
            errorMsg += u8"4. 摄像头被其他程序占用\n\n";

            errorMsg += u8"请尝试:\n";
            errorMsg += u8"• 检查设备管理器中的摄像头状态\n";
            errorMsg += u8"• Windows 设置 → 隐私 → 摄像头 → 允许应用访问摄像头\n";
            errorMsg += u8"• 在其他应用（如相机应用）中测试摄像头\n";
            errorMsg += u8"• 查看控制台的详细诊断信息";

            QMessageBox::critical(this, u8"摄像头错误", errorMsg);

            isCameraOn = false;
            btnVideo->setChecked(false);
            appendSystemMessage(u8"摄像头初始化失败 - 未找到摄像头设备");

            // 再次运行诊断
            // diagnoseMultimediaSupport();
            return;
        }

        ui->statusLabel->setText(u8"已开启摄像头");
        appendSystemMessage(u8"你已开启摄像头");

        // 显示摄像头信息
        const QCameraDevice &selectedCamera = cameras.first();
        // qDebug() << "\n使用摄像头:" << selectedCamera.description();
        // qDebug() << "摄像头 ID:" << selectedCamera.id();
        // qDebug() << "摄像头位置:" << selectedCamera.position();

        try
        {
            // 创建摄像头对象
            camera = new QCamera(selectedCamera, this);

            // 连接状态变化信号
            connect(camera, &QCamera::activeChanged, this,
                    [this](bool active)
                    {
                        // qDebug() << "摄像头活动状态变化:" << active;
                    });

            // 连接错误信号
            connect(camera, &QCamera::errorOccurred, this,
                    [this](QCamera::Error error, const QString &errorString)
                    {
                        // qWarning() << "摄像头错误:" << error << errorString;

                        QString msg = u8"摄像头错误: " + errorString + "\n\n";
                        msg += u8"错误代码: " + QString::number(error);

                        QMessageBox::warning(this, u8"摄像头错误", msg);

                        if (camera)
                        {
                            camera->stop();
                            delete camera;
                            camera = nullptr;
                        }
                        isCameraOn = false;
                        btnVideo->setChecked(false);
                        if (videoWidget)
                            videoWidget->hide();
                    });

            // 建立连接
            if (!captureSession)
            {
                captureSession = new QMediaCaptureSession(this);
            }

            // qDebug() << "设置摄像头到 capture session...";
            captureSession->setCamera(camera);
            captureSession->setVideoOutput(videoWidget);

            // qDebug() << "启动摄像头...";
            camera->start();

            // 等待一小段时间让摄像头启动
            QTimer::singleShot(500, this, [this]()
                               {
                if (camera && camera->error() != QCamera::NoError) {
                    QString errStr = camera->errorString();
                    // qWarning() << "摄像头启动失败:" << errStr;
                    
                    QMessageBox::critical(this, u8"启动失败", 
                        QString(u8"摄像头无法启动:\n%1").arg(errStr));
                    
                    if (camera) {
                        delete camera;
                        camera = nullptr;
                    }
                    isCameraOn = false;
                    btnVideo->setChecked(false);
                    if (videoWidget) videoWidget->hide();
                } else {
                    // qDebug() << "摄像头启动成功!";
                } });

            // 显示视频窗口
            videoWidget->show();
            dockParticipants->show();
            btnParticipants->setChecked(true);
        }
        catch (const std::exception &e)
        {
            qCritical() << "异常:" << e.what();
            QMessageBox::critical(this, u8"初始化失败",
                                  QString(u8"摄像头启动异常: %1").arg(e.what()));

            if (camera)
            {
                delete camera;
                camera = nullptr;
            }
            isCameraOn = false;
            btnVideo->setChecked(false);
            appendSystemMessage(u8"摄像头启动失败");
        }
    }
    else
    {
        ui->statusLabel->setText(u8"已关闭摄像头");
        appendSystemMessage(u8"你已关闭摄像头");

        if (camera)
        {
            // qDebug() << "停止摄像头...";
            camera->stop();
            delete camera;
            camera = nullptr;
        }
        if (videoWidget)
        {
            videoWidget->hide();
        }
    }
}

// 点击参会者按钮
void shared_screen::on_btnParticipantsClicked()
{
    ensureParticipantsDock(); // 参会者窗口
    if (dockParticipants->isHidden())
    {
        dockParticipants->show();
        btnParticipants->setChecked(true);
    }
    else
    {
        dockParticipants->hide();
        btnParticipants->setChecked(false);
    }
}

// 点击录制按钮
void shared_screen::on_btnRecordClicked()
{
    isRecording = !isRecording;
    btnRecord->setChecked(isRecording);
    if (isRecording)
    {
        ui->statusLabel->setText(u8"正在录制");
        appendSystemMessage(u8"会议录制已开始(演示)");
        startRecording();
    }
    else
    {
        ui->statusLabel->setText(u8"录制结束");
        appendSystemMessage(u8"会议录制已停止(演示)");
        stopRecording();
    }
}

// 开始录制
void shared_screen::startRecording()
{
    try
    {
        // 生成临时文件路径（录制时使用）
        QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
        currentRecordingPath = tempPath + "/Meeting_Recording_" + timestamp + ".mp4";

        // qDebug() << "临时录制文件:" << currentRecordingPath;

        // 创建媒体录制器
        if (!mediaRecorder)
        {
            mediaRecorder = new QMediaRecorder(this);

            // 连接录制信号
            connect(mediaRecorder, &QMediaRecorder::recorderStateChanged,
                    this, &shared_screen::onRecorderStateChanged);
            connect(mediaRecorder, &QMediaRecorder::errorOccurred,
                    this, &shared_screen::onRecorderError);
            connect(mediaRecorder, &QMediaRecorder::durationChanged,
                    this, &shared_screen::onRecordingDurationChanged);
        }

        // 设置录制格式和编码
        QMediaFormat format;
        format.setFileFormat(QMediaFormat::MPEG4);
        format.setVideoCodec(QMediaFormat::VideoCodec::H264);
        format.setAudioCodec(QMediaFormat::AudioCodec::AAC);
        mediaRecorder->setMediaFormat(format);

        // 设置视频质量
        mediaRecorder->setQuality(QMediaRecorder::HighQuality);
        mediaRecorder->setVideoResolution(1920, 1080);
        mediaRecorder->setVideoFrameRate(30);
        mediaRecorder->setVideoBitRate(5000000); // 5 Mbps
        mediaRecorder->setAudioBitRate(128000);  // 128 kbps

        // 设置输出文件
        mediaRecorder->setOutputLocation(QUrl::fromLocalFile(currentRecordingPath));

        // 配置音频输入
        if (!audioInput)
        {
            const QList<QAudioDevice> audioInputs = QMediaDevices::audioInputs();
            if (!audioInputs.isEmpty())
            {
                audioInput = new QAudioInput(audioInputs.first(), this);
                captureSession->setAudioInput(audioInput);
                // qDebug() << "音频输入已配置:" << audioInputs.first().description();
            }
            else
            {
                // qWarning() << "未找到音频输入设备";
            }
        }

        // 重要：确保视频源已连接
        if (isCameraOn && camera)
        {
            // 如果摄像头开启，录制摄像头
            // qDebug() << "录制模式: 摄像头";
            captureSession->setCamera(camera);
            captureSession->setVideoOutput(videoWidget);
        }
        else if (isScreenSharing)
        {
            // 如果屏幕共享开启，录制屏幕
            // qDebug() << "录制模式: 屏幕共享";
            // 屏幕录制需要特殊处理（见下方实现）
            QMessageBox::information(this, u8"提示",
                                     u8"当前版本暂不支持直接录制屏幕共享。\n"
                                     u8"建议：\n"
                                     u8"1. 先开启摄像头再录制\n"
                                     u8"2. 或使用专业的屏幕录制软件（如OBS Studio）");

            isRecording = false;
            btnRecord->setChecked(false);
            return;
        }
        else
        {
            // 没有视频源，只录制音频
            // qDebug() << "录制模式: 仅音频";
            QMessageBox::information(this, u8"提示",
                                     u8"当前没有视频源（摄像头或屏幕共享）。\n"
                                     u8"将只录制音频。\n\n"
                                     u8"建议先开启摄像头或屏幕共享。");

            // 可以选择继续录制音频，或者取消
            int ret = QMessageBox::question(this, u8"继续录制?",
                                            u8"是否继续录制（仅音频）？",
                                            QMessageBox::Yes | QMessageBox::No);

            if (ret == QMessageBox::No)
            {
                isRecording = false;
                btnRecord->setChecked(false);
                return;
            }
        }

        // 将录制器关联到捕获会话
        captureSession->setRecorder(mediaRecorder);

        // 开始录制
        // qDebug() << "开始录制...";
        mediaRecorder->record();

        // 检查是否真的开始录制了
        QTimer::singleShot(500, this, [this]()
                           {
            if (mediaRecorder->recorderState() != QMediaRecorder::RecordingState) {
                // qWarning() << "录制未能启动，当前状态:" << mediaRecorder->recorderState();
                // qWarning() << "错误:" << mediaRecorder->errorString();
                
                QMessageBox::critical(this, u8"录制失败", 
                    u8"无法启动录制。\n\n错误信息:\n" + mediaRecorder->errorString() +
                    u8"\n\n可能原因:\n"
                    u8"1. 没有可用的视频源（请先开启摄像头）\n"
                    u8"2. 缺少必要的编解码器\n"
                    u8"3. 文件路径无法访问");
                
                isRecording = false;
                btnRecord->setChecked(false);
            } else {
                // qDebug() << "录制已成功启动";
            } });

        // 记录开始时间
        recordingStartTime = QTime::currentTime();
        recordingTimer->start();

        ui->statusLabel->setText(u8"● 正在录制 00:00:00");
        appendSystemMessage(u8"会议录制已开始（临时文件）");

        // 更新按钮外观
        btnRecord->setStyleSheet(
            btnRecord->styleSheet() +
            "QPushButton:checked { background: #ff3b30; }");
        btnRecord->setToolTip(u8"停止录制");
    }
    catch (const std::exception &e)
    {
        qCritical() << "录制启动失败:" << e.what();
        QMessageBox::critical(this, u8"录制失败",
                              QString(u8"无法启动录制:\n%1").arg(e.what()));

        isRecording = false;
        btnRecord->setChecked(false);
    }
}

// 停止录制
void shared_screen::stopRecording()
{
    if (!mediaRecorder)
    {
        qWarning() << "mediaRecorder 为空";
        return;
    }

    if (mediaRecorder->recorderState() == QMediaRecorder::RecordingState)
    {
        // qDebug() << "停止录制...";
        mediaRecorder->stop();
        recordingTimer->stop();

        // 等待录制完成
        QTimer::singleShot(1000, this, [this]()
                           { saveRecordedFile(); });

        ui->statusLabel->setText(u8"处理录制文件中...");
        appendSystemMessage(u8"正在保存录制文件...");

        // btnRecord->setStyleSheet(""); // 恢复默认样式
        btnRecord->setToolTip(u8"开始录制");
    }
    else
    {
        // qWarning() << "录制器未在录制状态";
        ui->statusLabel->setText(u8"录制未启动");
    }
}

// 保存录制的文件
void shared_screen::saveRecordedFile()
{
    if (currentRecordingPath.isEmpty() || !QFile::exists(currentRecordingPath))
    {
        QMessageBox::warning(this, u8"保存失败", u8"录制文件不存在或为空");
        return;
    }

    // 检查文件大小
    QFileInfo fileInfo(currentRecordingPath);
    qint64 fileSize = fileInfo.size();
    // qDebug() << "录制文件大小:" << fileSize << "bytes";

    if (fileSize < 1024)
    {
        QMessageBox::warning(this, u8"警告",
                             u8"录制的文件非常小（" + QString::number(fileSize) + u8" 字节），可能没有成功录制内容。");
    }

    // 生成默认保存路径
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    if (defaultPath.isEmpty())
    {
        defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString defaultFileName = defaultPath + "/Meeting_" + timestamp + ".mp4";

    // 询问用户保存位置
    QString savePath = QFileDialog::getSaveFileName(
        this,
        u8"保存录制文件",
        defaultFileName,
        u8"视频文件 (*.mp4);;所有文件 (*.*)");

    if (savePath.isEmpty())
    {
        // 用户取消了保存，询问是否删除临时文件
        int ret = QMessageBox::question(this, u8"取消保存",
                                        u8"您取消了保存，是否删除录制的临时文件？",
                                        QMessageBox::Yes | QMessageBox::No);

        if (ret == QMessageBox::Yes)
        {
            QFile::remove(currentRecordingPath);
            // qDebug() << "已删除临时文件:" << currentRecordingPath;
        }
        else
        {
            QMessageBox::information(this, u8"提示",
                                     u8"临时文件保留在:\n" + currentRecordingPath);
        }
        return;
    }

    // 移动或复制文件到目标位置
    if (QFile::exists(savePath))
    {
        QFile::remove(savePath);
    }

    bool success = QFile::copy(currentRecordingPath, savePath);

    if (success)
    {
        // qDebug() << "文件已保存到:" << savePath;

        // 删除临时文件
        QFile::remove(currentRecordingPath);

        ui->statusLabel->setText(u8"录制已保存");

        // 询问是否打开文件所在位置
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(u8"录制完成");
        msgBox.setText(u8"录制已成功保存！\n\n文件位置:\n" + savePath +
                       u8"\n\n文件大小: " + QString::number(fileSize / 1024) + u8" KB");
        msgBox.setStandardButtons(QMessageBox::Open | QMessageBox::Close);
        msgBox.button(QMessageBox::Open)->setText(u8"打开文件位置");
        msgBox.button(QMessageBox::Close)->setText(u8"关闭");

        if (msgBox.exec() == QMessageBox::Open)
        {
            // 打开文件所在文件夹并选中文件
#ifdef Q_OS_WIN
            QProcess::startDetached("explorer", QStringList() << "/select," << QDir::toNativeSeparators(savePath));
#elif defined(Q_OS_MAC)
            QProcess::startDetached("open", QStringList() << "-R" << savePath);
#else
            QString folderPath = QFileInfo(savePath).absolutePath();
            QProcess::startDetached("xdg-open", QStringList() << folderPath);
#endif
        }
    }
    else
    {
        QMessageBox::critical(this, u8"保存失败",
                              u8"无法保存文件到:\n" + savePath +
                                  u8"\n\n临时文件仍在:\n" + currentRecordingPath);
    }
}

// 录制状态改变处理
void shared_screen::onRecorderStateChanged(QMediaRecorder::RecorderState state)
{
    // qDebug() << "录制器状态变化:" << state;

    switch (state)
    {
    case QMediaRecorder::StoppedState:
        // qDebug() << "录制已停止";
        // qDebug() << "输出位置:" << mediaRecorder->actualLocation();
        break;
    case QMediaRecorder::RecordingState:
        // qDebug() << "正在录制...";
        // qDebug() << "输出位置:" << mediaRecorder->outputLocation();
        break;
    case QMediaRecorder::PausedState:
        // qDebug() << "录制已暂停";
        break;
    }
}

// 录制错误处理
void shared_screen::onRecorderError(QMediaRecorder::Error error, const QString &errorString)
{
    qCritical() << "录制错误:" << error << errorString;

    QString errorMsg;
    switch (error)
    {
    case QMediaRecorder::NoError:
        return;
    case QMediaRecorder::ResourceError:
        errorMsg = u8"资源错误: " + errorString + u8"\n\n可能原因:\n- 摄像头被其他程序占用\n- 音频设备不可用";
        break;
    case QMediaRecorder::FormatError:
        errorMsg = u8"格式错误: " + errorString + u8"\n\n可能原因:\n- 不支持的编解码器\n- 文件格式不兼容";
        break;
    case QMediaRecorder::OutOfSpaceError:
        errorMsg = u8"磁盘空间不足: " + errorString;
        break;
    case QMediaRecorder::LocationNotWritable:
        errorMsg = u8"无法写入文件: " + errorString + u8"\n\n请检查文件路径权限";
        break;
    default:
        errorMsg = u8"录制错误: " + errorString;
        break;
    }

    QMessageBox::critical(this, u8"录制错误", errorMsg);

    isRecording = false;
    btnRecord->setChecked(false);
    recordingTimer->stop();

    ui->statusLabel->setText(u8"录制失败");
    appendSystemMessage(u8"录制失败: " + errorString);
}

// 屏幕捕获
void shared_screen::captureScreen()
{
}
// 录制时长更新
void shared_screen::onRecordingDurationChanged(qint64 duration)
{
    // duration 是毫秒数
    int seconds = duration / 1000;
    int minutes = seconds / 60;
    int hours = minutes / 60;

    seconds = seconds % 60;
    minutes = minutes % 60;

    QString timeStr = QString("%1:%2:%3")
                          .arg(hours, 2, 10, QChar('0'))
                          .arg(minutes, 2, 10, QChar('0'))
                          .arg(seconds, 2, 10, QChar('0'));
}

// 更新录制时间显示
void shared_screen::updateRecordingTime()
{
    if (!isRecording)
    {
        return;
    }

    QTime currentTime = QTime::currentTime();
    int elapsed = recordingStartTime.secsTo(currentTime);

    int hours = elapsed / 3600;
    int minutes = (elapsed % 3600) / 60;
    int seconds = elapsed % 60;

    QString timeStr = QString("%1:%2:%3")
                          .arg(hours, 2, 10, QChar('0'))
                          .arg(minutes, 2, 10, QChar('0'))
                          .arg(seconds, 2, 10, QChar('0'));

    ui->statusLabel->setText(u8"● 正在录制 " + timeStr);
}

// 点击举手按钮
void shared_screen::on_btnRaiseHandClicked()
{   


    isHandRaised = !isHandRaised;
    if (isHandRaised)
    {
        ui->statusLabel->setText(u8"你举手了");
        appendSystemMessage(u8"你举手了");
    }
    else
    {
        ui->statusLabel->setText(u8"你放下了手");
        appendSystemMessage(u8"你放下了手");
    }
    // =============== 之后的逻辑 ===============
}

// 离开会议按钮
void shared_screen::on_btnLeaveClicked()
{
    if (QMessageBox::question(this, u8"离开会议", u8"确定要离开会议吗?") == QMessageBox::Yes)
    {
        if (camera)
        {
            camera->stop();
            delete camera;
            camera = nullptr;
        }
        ui->stackedWidget->setCurrentIndex(0);
        ui->statusLabel->setText(u8"状态:未连接");
    }
    // m_signaling-> disconnectFromServer();
    close();
}

// =============== 模拟器 ===============
void shared_screen::on_fakeNetworkTick()
{
    static const char *labels[] = {"优", "良", "中", "差"};
    int idx = QRandomGenerator::global()->bounded(0, 4);
    QString s = u8"网络:" + QString::fromUtf8(labels[idx]);

    QString color = "#34c759";
    if (idx == 1)
        color = "#0a84ff";
    if (idx == 2)
        color = "#ffd60a";
    if (idx == 3)
        color = "#ff3b30";
    netLabel->setStyleSheet(QString("QLabel{color:%1;padding-left:6px;}").arg(color));
    netLabel->setText(s);
}

void shared_screen::on_fakeRemoteMsg()
{
    appendRemoteMessage(u8"小张", u8"收到~");
    if (!isChatVisible)
    {
        unreadCount++;
        updateChatBadge();
    }
}

// 聊天框窗口
void shared_screen::toggleChatPanel()
{
    isChatVisible = !isChatVisible;
    if (isChatVisible)
    {
        ui->dockChat->show();
        unreadCount = 0;
        updateChatBadge();
    }
    else
    {
        ui->dockChat->hide();
    }
}

void shared_screen::ensureParticipantsDock()
{
    if (dockParticipants)
        return;

    dockParticipants = new QDockWidget(u8"👥 参会者", this);
    dockParticipants->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

    participantsWidget = new QWidget(dockParticipants);
    QVBoxLayout *layout = new QVBoxLayout(participantsWidget);

    // ===== 视频显示窗口 =====
    videoWidget = new QVideoWidget(participantsWidget);
    videoWidget->setMinimumSize(320, 240);
    videoWidget->setStyleSheet("background:black;");
    layout->addWidget(videoWidget);

    participantsWidget->setLayout(layout);
    dockParticipants->setWidget(participantsWidget);
    // 让它一旦创建就是浮动窗，不挤压主界面 ---
    dockParticipants->setFloating(true);
    dockParticipants->setAllowedAreas(Qt::NoDockWidgetArea);

    // 不会真正停靠，只是让 Qt 管理它
    addDockWidget(Qt::RightDockWidgetArea, dockParticipants);
    dockParticipants->hide();

    // =============== 之后的逻辑 ===============
    // 添加参会者列表（模拟）
    participantsList = new QListWidget(participantsWidget);
    participantsList->addItem(u8"我(主持人)");
    participantsList->addItem(u8"小张");
    participantsList->addItem(u8"小李");
    layout->addWidget(participantsList);
}

// 聊天框中的系统消息
void shared_screen::appendSystemMessage(const QString &text)
{
    const QString ts = QDateTime::currentDateTime().toString("hh:mm");
    const QString html = QString("<div style='margin:8px 0;'>"
                                 "<span style='color:#888;font-size:11px;'>%1</span> "
                                 "<span style='color:#3C3C3C;font-weight:bold;'>系统:</span>"
                                 "<span style='color:#3C3C3C;'>%2</span>"
                                 "</div>")
                             .arg(ts, text.toHtmlEscaped());
    ui->chatView->append(html);
}

// 聊天框中参会者的消息
void shared_screen::appendRemoteMessage(const QString &sender, const QString &text)
{
    const QString ts = QDateTime::currentDateTime().toString("hh:mm");
    const QString html = QString("<div style='margin:8px 0;'>"
                                 "<span style='color:#888;font-size:11px;'>%1</span> "
                                 "<span style='color:#40c4ff;font-weight:bold;'>%2:</span>"
                                 "<span style='color:#1d1d1f;'>%3</span>"
                                 "</div>")
                             .arg(ts, sender.toHtmlEscaped(), text.toHtmlEscaped());
    ui->chatView->append(html);
}

void shared_screen::updateChatBadge()
{
    btnChat->setIcon(QIcon("../../src/icons/message.png"));
    if (unreadCount > 0)
    {
        btnChat->setText(QString::fromUtf8("(%1)").arg(unreadCount));
    }
    else
    {
        btnChat->setText("");
    }
}

void shared_screen::buildShortcuts()
{
    auto s1 = new QShortcut(QKeySequence("Ctrl+D"), this);
    connect(s1, &QShortcut::activated, this, &shared_screen::on_btnVoiceClicked);

    auto s2 = new QShortcut(QKeySequence("Ctrl+E"), this);
    connect(s2, &QShortcut::activated, this, &shared_screen::on_btnVideoClicked);

    auto s3 = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(s3, &QShortcut::activated, this, &shared_screen::on_btnShareScreenClicked);

    auto s4 = new QShortcut(QKeySequence("Ctrl+H"), this);
    connect(s4, &QShortcut::activated, this, &shared_screen::on_btnChatClicked);

    auto s5 = new QShortcut(QKeySequence("Ctrl+P"), this);
    connect(s5, &QShortcut::activated, this, &shared_screen::on_btnParticipantsClicked);

    auto s6 = new QShortcut(QKeySequence("Ctrl+R"), this);
    connect(s6, &QShortcut::activated, this, &shared_screen::on_btnRecordClicked);
}

// =============== "按住说话"演示 ===============
void shared_screen::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat())
    {
        if (!spaceHeldPTT)
        {
            spaceHeldPTT = true;
            if (!isVoiceOn)
            {
                on_btnVoiceClicked();
                appendSystemMessage(u8"按住空格键:临时开麦");
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
                on_btnVoiceClicked();
                appendSystemMessage(u8"松开空格键:还原为静音");
            }
        }
    }
    QMainWindow::keyReleaseEvent(event);
}
