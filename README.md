# ByteScreenShare

## 运行逻辑
点击exe文件后，产生登录界面。输入房间号点击“加入会议”按钮后，会尝试连接服务器，连接成功后会跳转主界面。
在主界面中点击“共享屏幕”时，会开始运行`srartP2P()`,当P2P建立成功后会自动执行sendMessage，具体看`on_btnShareScreenClicked()`

## 报错代码定位

- `    config.iceServers.emplace_back("stun:stun.l.google.com:19302"); // 公网 STUN
`
- `void shared_screen::setupDataChannel(shared_ptr<rtc::DataChannel> channel) `
都是libdatachannel库中的内容，所以怀疑是库的问题

## 项目结构
```
BBYTESSCREENSHARE/
├── src/
│   ├── icons/
│   ├── rtc/  (不重要可删)
│   │   ├── PeerConnectionManager.cpp
│   │   └── PeerConnectionManager.hpp
│   ├── signaling/  (不重要可删)
│   │   ├── WsSignalingClient.cpp
│   │   └── WsSignalingClient.hpp
│   ├── ui/
│   │   ├── shared_screen.cpp <--问题出在这里
│   │   ├── shared_screen.h
│   │   └── shared_screen.ui
│   └── main.cpp
├── third_party/
├── CMakeLists.txt
└── README.md


```