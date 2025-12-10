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
        list(APPEND CMAKE_PREFIX_PATH "D:\\cs_techs\\Qt\\6.8.3\\msvc2022_64")
    endif()
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Multimedia MultimediaWidgets WebSockets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Multimedia MultimediaWidgets WebSockets Network)

# MSVC 编码与标准行为
add_compile_options(
    "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
    "$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>"
    "$<$<CXX_COMPILER_ID:MSVC>:/permissive->"
   # "$<$<CXX_COMPILER_ID:MSVC>:/std:c++17>"
)

set(SRCS
    src/main.cpp
    src/ui/shared_screen.cpp
    src/signaling/WsSignalingClient.cpp
    src/rtc/PeerConnectionManager.cpp
    src/encoder/VideoEncoder.cpp
    src/Capture/ScreenCaptureService.cpp
)

set(HEADERS
    src/ui/shared_screen.h
    src/signaling/WsSignalingClient.hpp
    src/rtc/PeerConnectionManager.hpp
    src/encoder/VideoEncoder.h
    src/Capture/ScreenCaptureService.h
)

set(UIS
    src/ui/shared_screen.ui
)

qt_wrap_ui(UI_HEADERS ${UIS})

qt_add_resources(RESOURCES src/icons/icons.qrc)

add_executable(${PROJECT_NAME}
    # WIN32
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

set(THIRD_PART_ROOT "${CMAKE_SOURCE_DIR}/third_part") # 注意拼写 third_part
# ============================================================
# LibDataChannel 源码集成 (自动适配 Debug/Release)
# ============================================================

# 1. 告诉它 OpenSSL 在哪 (路径换成你自己的安装路径)
set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64") 

# 2. 关闭不必要的编译选项
set(NO_EXAMPLES ON CACHE BOOL "" FORCE)
set(NO_TESTS ON CACHE BOOL "" FORCE)

# 3. 加入子项目 (这一步会自动编译出 Debug 版的库！)
add_subdirectory("${CMAKE_SOURCE_DIR}/third_part/libdatachannel")

# 4. 链接到目标 (注意：名字叫 datachannel)
# 这里 CMake 会自动处理 .lib 路径，不用你操心
target_link_libraries(${PROJECT_NAME} PRIVATE datachannel)

# 5. OpenSSL 的 DLL 还是要复制的 (因为它们是外部的)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${OPENSSL_ROOT_DIR}/bin/libcrypto-3-x64.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${OPENSSL_ROOT_DIR}/bin/libssl-3-x64.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
)
# --------------------------------------
# FFmpeg 依赖
# --------------------------------------
set(FFMPEG_ROOT "${CMAKE_SOURCE_DIR}/third_part/ffmpeg" CACHE PATH "FFmpeg 安装根目录")
get_filename_component(FFMPEG_ROOT_ABS "${FFMPEG_ROOT}" ABSOLUTE)
message(STATUS "FFmpeg FFMPEG_ROOT_ABS=${FFMPEG_ROOT_ABS}")

# 添加头文件搜索路径
target_include_directories(${PROJECT_NAME} PUBLIC
    "${FFMPEG_ROOT_ABS}/include"
)

# 查找所需的 FFmpeg 库
find_library(AVCODEC_LIBRARY NAMES avcodec PATHS "${FFMPEG_ROOT_ABS}/lib" REQUIRED)
find_library(AVFORMAT_LIBRARY NAMES avformat PATHS "${FFMPEG_ROOT_ABS}/lib" REQUIRED)
find_library(AVUTIL_LIBRARY NAMES avutil PATHS "${FFMPEG_ROOT_ABS}/lib" REQUIRED)
find_library(SWSCALE_LIBRARY NAMES swscale PATHS "${FFMPEG_ROOT_ABS}/lib" REQUIRED)

message(STATUS "FFmpeg Libraries: ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY}")

# 链接 FFmpeg 库到项目
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${AVCODEC_LIBRARY}
    ${AVFORMAT_LIBRARY}
    ${AVUTIL_LIBRARY}
    ${SWSCALE_LIBRARY}
)

# =======================================================
# 智能复制编译出来的 libdatachannel DLL 到 exe 旁边
# =======================================================
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:datachannel>            # <--- 自动查找刚编译好的 DLL 路径
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/   # <--- 你的 exe 所在的路径
    COMMENT "正在复制源码编译生成的 datachannel.dll ..."
)

# FFmpeg DLL 复制
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FFMPEG_ROOT_ABS}/bin/avcodec-62.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FFMPEG_ROOT_ABS}/bin/avformat-62.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FFMPEG_ROOT_ABS}/bin/avutil-60.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FFMPEG_ROOT_ABS}/bin/swresample-6.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FFMPEG_ROOT_ABS}/bin/swscale-9.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
    COMMENT "复制 FFmpeg 运行时 DLLs 到输出目录"
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
