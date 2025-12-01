cmake_minimum_required(VERSION 3.16)
project(ScreenShareClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 让 CMake 自动处理 Qt 的 moc / uic / rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ================= Qt 路径关键设置 =================
# 这里填你安装 Qt6 对应 msvc2022_64 的根目录
# 注意用 / 或者把 \ 写成 \\ 都可以，这里用 / 最省事
set(CMAKE_PREFIX_PATH "H:/qt6/6.8.3/msvc2022_64")

# 如果以后你换了安装路径，只要改这一行就行
# ===================================================

# ---- Qt6 ----
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

# ---- libdatachannel ----
# 确保目录存在： BytesScreenShare/third_party/libdatachannel/CMakeLists.txt
add_subdirectory(third_party/libdatachannel)

# ---- 源文件显式列出来（跟你现在结构一一对应）----
add_executable(ScreenShareClient
    src/main.cpp

    src/ui/shared_screen.cpp
    src/ui/shared_screen.ui

    src/signaling/TcpSignalingClient.cpp
    src/rtc/PeerConnectionManager.cpp
)

# ---- 头文件搜索路径 ----
target_include_directories(ScreenShareClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/signaling
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rtc
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libdatachannel/include
)

# ---- 链接 Qt 和 libdatachannel ----
target_link_libraries(ScreenShareClient PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    datachannel          # libdatachannel 生成的目标名，默认就是这个
)
