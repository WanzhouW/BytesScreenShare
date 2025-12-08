cmake_minimum_required(VERSION 3.19)
project(shared_screen LANGUAGES CXX)

# --------------------------------------
# 基础设置
# --------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 允许通过环境变量或缓存自定义 Qt 路径，未指定时回退到本机默认路径
# set(CMAKE_PREFIX_PATH "xxx")
if(NOT DEFINED CMAKE_PREFIX_PATH)
    if(DEFINED ENV{QT_PATH})
        list(APPEND CMAKE_PREFIX_PATH "$ENV{QT_PATH}")
    else()
        list(APPEND CMAKE_PREFIX_PATH "f:/Qt/6.8.3/msvc2022_64")
    endif()
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Multimedia MultimediaWidgets WebSockets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Multimedia MultimediaWidgets WebSockets Network)

# MSVC 编码与标准行为
add_compile_options(
    "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
    "$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>"
    "$<$<CXX_COMPILER_ID:MSVC>:/permissive->"
    "$<$<CXX_COMPILER_ID:MSVC>:/std:c++17>"
)

set(SRCS
    src/main.cpp
    src/ui/shared_screen.cpp
    src/signaling/WsSignalingClient.cpp
    src/rtc/PeerConnectionManager.cpp
)

set(HEADERS
    src/ui/shared_screen.h
    src/signaling/WsSignalingClient.hpp
    src/rtc/PeerConnectionManager.hpp
)

set(UIS
    src/ui/shared_screen.ui
)

qt_wrap_ui(UI_HEADERS ${UIS})

qt_add_resources(RESOURCES src/icons/icons.qrc)

add_executable(${PROJECT_NAME}
    WIN32
    ${SRCS}
    ${HEADERS}
    ${UI_HEADERS}
    ${RESOURCES}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Multimedia
        Qt${QT_VERSION_MAJOR}::MultimediaWidgets
        Qt${QT_VERSION_MAJOR}::WebSockets
        Qt${QT_VERSION_MAJOR}::Network
)

# --------------------------------------
# libdatachannel (rtc) 依赖
# --------------------------------------
set(RTC_ROOT "./third_part" CACHE PATH "libdatachannel install 根目录")
get_filename_component(RTC_ROOT_ABS "${RTC_ROOT}" ABSOLUTE)
message(STATUS "libdatachannel RTC_ROOT_ABS=${RTC_ROOT_ABS}")

list(APPEND CMAKE_PREFIX_PATH "${RTC_ROOT_ABS}")
message(STATUS "加入 CMAKE_PREFIX_PATH: ${RTC_ROOT_ABS}")

find_package(LibDataChannel CONFIG REQUIRED PATHS "${RTC_ROOT_ABS}" NO_DEFAULT_PATH)

target_link_libraries(${PROJECT_NAME} PRIVATE LibDataChannel::LibDataChannel)

# 运行时 DLL（已手动确认名称），统一复制到可执行目录（无检查）
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${RTC_ROOT_ABS}/bin/datachannel.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
    COMMENT "复制 libdatachannel 运行时 DLL 到输出目录"
)

# # ========================================
# # Windows: 自动复制 Qt Multimedia 插件和 FFmpeg DLL
# # ========================================
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt.exe
    PATHS "${CMAKE_PREFIX_PATH}/bin" NO_DEFAULT_PATH)

    if(NOT WINDEPLOYQT_EXECUTABLE)
        # 回退到常见 Qt 安装路径
        find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt.exe
            HINTS "F:/Qt/6.8.3/msvc2022_64/bin" NO_DEFAULT_PATH)
    endif()

    if(WINDEPLOYQT_EXECUTABLE)
        message(STATUS "使用 windeployqt: ${WINDEPLOYQT_EXECUTABLE}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:${PROJECT_NAME}>"
            COMMENT "Running windeployqt to deploy Qt dependencies..."
            VERBATIM
        )
    else()
        message(WARNING "windeployqt 未找到，跳过自动部署 Qt 依赖。请手动运行 windeployqt 或将 Qt bin 添加到 CMAKE_PREFIX_PATH/bin")
    endif()

    # 打印最终输出信息
    message(STATUS "${PROJECT_NAME} generated successfully.")
endif()
