# BytesScreenShare - 跨平台屏幕共享与实时推流系统

**[English](./README_EN.md)** | 简体中文
> 主要关注点：
将屏幕统一变换为1080p，30fps的h.264编码的视频流传输；支持不同分辨率的屏幕
此版本中Cmake内，FFmpeg、Qt、libdatachannel等第三方库路径均需根据本地环境修改。
尚未按信令服务器设计完成交互设计对齐，编译已经通过；
main.cpp中测试用例尚未完成编写，尽可能尽快完成；
下面给出了线程模型，可供查看；
> 根据1207凌晨注释了很多信令代码；
> ScreenCaptureService文件夹实现主要屏幕采集与发送功能
> VideoEncoder实现编码
> RtcRtpSender实现RFC 6184 H.264 RTP 封装、RTC对接；


## 项目概述

| 特性 | 说明 |
|------|------|
| **屏幕捕获** | 使用 Qt Multimedia 的 `QScreenCapture` 实时捕获屏幕 |
| **视频编码** | 基于 FFmpeg 的 H.264 编码，支持自适应分辨率 |
硬件加速编码**：使用 FFmpeg 进行高效的视频编码
- 🌐 **WebRTC 推流**：通过 WebRTC 标准协议实现低延迟的网络传输
- 🎨 **跨平台 UI**：基于 Qt6 的现代化用户界面
- ⚡ **低延迟设计**：从捕获到编码到传输的全链路优化
| **格式转换** | 支持 RGB/BGRA → YUV420P 的高效转换 |
| **网络传输** | WebRTC + RTP 协议，自动处理 NAL 单元分片 |
| **实时预览** | 编码前的原始画面实时预览 |
| **响应式调整** | 自动检测分辨率变化并重新初始化 |

## 依赖环境

### 核心依赖库

✓ Qt 6.8.3 (Core, Gui, Widgets, Multimedia, Network)  
✓ FFmpeg 8.0.1 (libavcodec, libavformat, libavutil, libswscale)  
✓ libdatachannel (WebRTC 信令)  
✓ OpenSSL 3.x (加密和认证)  

## 快速开始


### 项目编译步骤

#### 方式一：使用 Visual Studio

# 1. 打开项目目录
cd BytesScreenShare

# 2. 配置 CMake（Visual Studio 会自动处理）
编辑 CMakeLists.txt，确保路径正确：把ffmpeg、Qt和libdatachannel的路径改成你本地的路径，例如：
# - set(CMAKE_PREFIX_PATH "D:/Qt/6.8.3/msvc2022_64")
# - set(FFMPEG_ROOT "D:/ffmpeg-8.0.1-full_build-shared")
# - set(LIBDATACHANNEL_PATH "D:/libdatachannel/libdatachannel")

# 3. 在 Visual Studio 中打开文件夹
# 文件 -> 打开 -> 文件夹 -> 选择 BytesScreenShare

# 4. 选择配置和编译
# 在工具栏中选择：Release 模式
# 生成 -> 生成解决方案（或按 Ctrl+Shift+B）  

#### 方式二：使用命令行（Ninja）

# 1. 进入项目目录
cd BytesScreenShare

# 2. 配置项目
cmake -B build -G Ninja `
  -DCMAKE_PREFIX_PATH="D:/Qt/6.8.3/msvc2022_64" `
  -DFFMPEG_ROOT="D:/ffmpeg-8.0.1-full_build-shared"

# 3. 编译
cmake --build build --config Release

# 4. 运行
./build/shared_screen.exe  



## 项目结构

```
BytesScreenShare/
├── src/
│   ├── main.cpp                         # 测试用窗口入口
│   ├── shared_screen.h/cpp              # 主窗口
│   ├── Capture/
│   │   ├── ScreenCaptureService.h/cpp   # 屏幕捕获模块（Qt Multimedia）
│   │   └── ...
│   ├── encoder/
│   │   ├── VideoEncoder.h/cpp           # H.264 编码器（FFmpeg）
│   │   └── ...
│   ├── network/
│   │   ├── RtcRtpSender.h/cpp           # WebRTC RTP 发送器
│   │   └── ...
│   └── ...
├── CMakeLists.txt                       # CMake 配置文件
├── README.md                            # 本文件
└── .gitignore                           # Git 忽略规则
```

### 核心模块说明

#### 📷 ScreenCaptureService（屏幕捕获）

**职责**：
- 使用 `QScreenCapture` 实时捕获屏幕画面
- 通过 `QVideoSink` 提取帧数据
- 将原始帧传递给编码器

**关键接口**：
void startCapture();              // 启动捏获
void stopCapture();               // 停止捕获
void initEncoder(const QString& targetIp);  // 初始化编码器
QVideoWidget* getVideoPreviewWidget();      // 获取预览窗口

#### 🎥 VideoEncoder（视频编码）

**职责**：
- 初始化 H.264 编码器上下文
- 将 QVideoFrame 转换为 YUV420P 格式
- 使用 libswscale 进行图像缩放和格式转换
- 编码为 H.264 NAL 单元

**关键参数**：
bool init(int width, int height, int fps, int bitrate);
// 默认目标：1920x1080@30fps, 4Mbps

#### 🌐 RtcRtpSender（WebRTC 推流）

**职责**：
- 建立 WebRTC PeerConnection
- 生成本地 SDP Offer
- 接收远端 SDP Answer
- 通过 RTP 发送 H.264 NAL 单元
- 自动处理 FU-A 分片和序列号管理

**信令流程**：
本地         信令服务器       远端
  |                |            |
  |----Offer------>|            |
  |                |-----Offer--->|
  |                |<----Answer----|
  |<----Answer-----|            |
  |                |<--ICE Candidate--|
  |<--ICE Candidate-|            |
  |================建立 DTLS-SRTP 连接================|
  |----H.264 RTP Stream--------->|

## 使用指南

### 基本使用步骤

#### 步骤 1：启动应用

# 直接运行编译后的可执行文件
./build/shared_screen.exe

# 或从 Visual Studio 运行
按 F5 或菜单 -> 调试 -> 开始执行

#### 步骤 2：查看实时预览

应用启动后会自动：
- ✅ 打开预览窗口（800x600）
- ✅ 开始捕获屏幕画面
- ✅ 显示原始帧率和分辨率

#### 步骤 3：初始化编码器

// 在代码中调用
screenService.initEncoder("127.0.0.1");

// 或通过 UI 按钮触发（待实现）

#### 步骤 4：建立 WebRTC 连接

// RtcRtpSender 会自动生成 SDP Offer
// 通过信令服务器发送给对端
// 接收到 Answer 后调用
screenService.setRemoteSdp(answerSdp);

#### 步骤 5：开始推流

// 编码器自动将编码数据通过 RTP 发送
// 无需额外操作

### 常用配置

#### 修改编码参数

编辑 `src/encoder/VideoEncoder.cpp` 中的 `init()` 函数：

```cpp
bool VideoEncoder::init(int width, int height, int fps, int bitrate) {
    // 目标分辨率（可调整）
    m_targetW = 1280;  // 改为 720p
    m_targetH = 720;

    // 编码器参数
    m_codecCtx->bit_rate = 2500000;      // 2.5 Mbps（降低码率）
    m_codecCtx->gop_size = 30;            // 30帧一个关键帧
    m_codecCtx->max_b_frames = 0;         // 无 B 帧（降低延迟）
    
    // FFmpeg 优化选项
    av_dict_set(&opts, "preset", "veryfast", 0);  // 更快的编码速度
    av_dict_set(&opts, "tune", "zerolatency", 0); // 低延迟模式
}
```

#### 调整捕获帧率

编辑 `src/Capture/ScreenCaptureService.cpp`：

```cpp
// 在 init() 函数中修改
m_screenCapture->setFrameRate(60);  // 改为 60 fps
m_encoder->init(1920, 1080, 60, 8000000);  // 8 Mbps
```

### 调试模式

#### 启用日志输出

编辑 `CMakeLists.txt`：

```cmake
# 添加调试标志
add_compile_options($<$<CONFIG:Debug>:-DDEBUG_MODE>)

# 在代码中使用
#ifdef DEBUG_MODE
qDebug() << "Frame encoded:" << pts;
#endif
```

#### 性能监测

在 `VideoEncoder` 中添加计时器：

```cpp
#include <chrono>

auto start = std::chrono::high_resolution_clock::now();
// ... 编码操作 ...
auto end = std::chrono::high_resolution_clock::now();
qDebug() << "Encoding time:" 
         << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() 
         << "ms";
```

## 技术架构

### 数据流管道

屏幕画面 (RGB)
   ↓
QScreenCapture (Qt Multimedia)
   ↓
QVideoFrame (Qt 帧格式)
   ↓
VideoEncoder (FFmpeg)
   ├─ sws_scale: RGB → YUV420P 转换
   ├─ avcodec_send_frame: 发送到编码器
   └─ avcodec_receive_packet: 接收编码数据
   ↓
H.264 NAL 单元
   ↓
RtcRtpSender (libdatachannel)
   ├─ FU-A 分片（>1100 字节）
   ├─ RTP 头封装
   └─ DTLS-SRTP 加密
   ↓
网络传输 (UDP)

### 编码参数优化

| 分辨率 | 码率 | 帧率 | 延迟 | 用途 |
|--------|------|------|------|------|
| 1920×1080 | 4-6 Mbps | 30 fps | ~100ms | 高清会议 |
| 1280×720 | 2-3 Mbps | 30 fps | ~80ms | 标清会议 |
| 640×480 | 1-1.5 Mbps | 24 fps | ~60ms | 低带宽 |

### 线程模型

主线程 (QApplication)
├─ UI 绘制
├─ 事件处理
└─ 信令交互

捕获线程 (Qt Multimedia)
├─ 屏幕捕获
└─ 帧回调

编码线程（同步，在捕获回调中）
├─ 格式转换
└─ H.264 编码

网络线程 (libdatachannel)
├─ RTP 打包
└─ UDP 发送

## 常见问题

### Q1: 编译失败 - "找不到 FFmpeg 头文件"

**原因**：CMakeLists.txt 中的 FFmpeg 路径不正确

**解决**：
```bash
# 确认 FFmpeg 安装位置
dir D:\ffmpeg-8.0.1-full_build-shared\include

# 更新 CMakeLists.txt
set(FFMPEG_ROOT "你的实际路径")
```

### Q2: 运行时崩溃 - "QVideoFrame::map() 失败"

**原因**：输入帧格式不兼容

**解决**：
```cpp
// 在 VideoEncoder::encode() 中添加调试
qDebug() << "Frame format:" << inputFrame.pixelFormat();
qDebug() << "Frame size:" << inputFrame.width() << "x" << inputFrame.height();

// 确认格式为 BGRA 或 RGB32
```

### Q3: 性能不足 - 帧率下降

**原因**：编码器配置或网络带宽不足

**解决**：
```cpp
// 降低目标分辨率
m_targetW = 1280;
m_targetH = 720;

// 使用更快的编码预设
av_dict_set(&opts, "preset", "ultrafast", 0);

// 减少关键帧间隔
m_codecCtx->gop_size = 15;
```

### Q4: WebRTC 连接失败

**原因**：
- 防火墙阻止 UDP
- STUN/TURN 服务器配置错误
- SDP Answer 格式错误

**解决**：
```cpp
// 检查 SDP 生成
connect(&m_rtcSender, &RtcRtpSender::onLocalSdpReady, [](const QString& sdp) {
    qDebug() << "Local SDP:" << sdp;
});

// 添加 STUN 服务器
// rtc::Configuration config;
// config.iceServers.emplace_back("stun:stun.l.google.com:19302");
```

### Q5: 编码文件太大 - 码率过高

**原因**：未正确设置码率限制

**解决**：
```cpp
// 代码中明确设置码率（bits/second）
m_codecCtx->bit_rate = 2500000;  // 2.5 Mbps = 312.5 KB/s
```

## 贡献指南

### 开发流程

1. **Fork** 本仓库
2. **创建分支** (`git checkout -b feature/YourFeature`)
3. **提交更改** (`git commit -am 'Add YourFeature'`)
4. **推送分支** (`git push origin feature/YourFeature`)
5. **提交 Pull Request**

### 代码规范

- **命名**：驼峰式命名，成员变量加 `m_` 前缀
- **格式**：4 空格缩进，最多 100 列宽度
- **注释**：关键逻辑必须有中英文注释
- **测试**：新功能必须包含单元测试

### 提交信息格式

```
[类型] 简短描述

详细描述（可选）

关闭问题：#123
```

**类型**：
- `feat`: 新功能
- `fix`: 修复 Bug
- `docs`: 文档更新
- `style`: 代码风格
- `refactor`: 重构
- `test`: 测试用例
- `chore`: 构建/配置

### 报告 Bug

提交 Issue 时请包含：
- 操作系统和版本
- Qt 和 FFmpeg 版本
- 错误日志（完整堆栈跟踪）
- 复现步骤

## 许可证

本项目采用 **MIT License**，详见 [LICENSE](./LICENSE) 文件。

### 第三方库许可

- **Qt 6**：LGPL v3 / Commercial
- **FFmpeg**：LGPL v2.1+ / GPL v2+ (取决于构建选项)
- **libdatachannel**：MPL-2.0
- **OpenSSL**：Apache 2.0

---

## 相关链接

- 📚 [Qt 官方文档](https://doc.qt.io/)
- 🎥 [FFmpeg 官方文档](https://ffmpeg.org/documentation.html)
- 🌐 [WebRTC 标准](https://www.w3.org/TR/webrtc/)
- 💬 [讨论区](https://github.com/WanzhouW/BytesScreenShare/discussions)
- 🐛 [报告 Issue](https://github.com/WanzhouW/BytesScreenShare/issues)

---

**最后更新**：2025 年 12 月

**维护者**：[@WanzhouW](https://github.com/WanzhouW)

**致谢**：感谢所有贡献者的支持和反馈！
