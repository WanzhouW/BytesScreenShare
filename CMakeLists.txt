cmake_minimum_required(VERSION 3.16) # CMake install : https://cmake.org/download/
project(shared_screen LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "H:/qt6/6.8.3/msvc2022_64") # Qt Kit Dir
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 兼容 Qt6 / Qt5 的查找写法
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Multimedia MultimediaWidgets WebSockets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Gui Widgets Multimedia MultimediaWidgets WebSockets Network)

aux_source_directory(./src srcs)

# Specify MSVC UTF-8 encoding   
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# ---- libdatachannel ----
add_subdirectory(third_party/libdatachannel)

add_executable(shared_screen
    src/main.cpp

    src/ui/shared_screen.cpp
    src/ui/shared_screen.ui

    src/signaling/WsSignalingClient.cpp
    src/rtc/PeerConnectionManager.cpp
)

# ---- 头文件搜索路径 ----
target_include_directories(shared_screen PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/signaling
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rtc
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libdatachannel/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Multimedia
        Qt${QT_VERSION_MAJOR}::WebSockets
        Qt${QT_VERSION_MAJOR}::MultimediaWidgets
        datachannel
)

# # ========================================
# # Windows: 自动复制 Qt Multimedia 插件和 FFmpeg DLL
# # ========================================
# if(WIN32)
#     # 获取 Qt 安装路径
#     get_target_property(QT_QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
#     get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
#     get_filename_component(QT_INSTALL_DIR "${QT_BIN_DIR}" DIRECTORY)
    
#     set(QT_PLUGINS_DIR "${QT_INSTALL_DIR}/plugins")
#     set(QT_BIN_PATH "${QT_INSTALL_DIR}/bin")
    
#     message(STATUS "Qt 安装目录: ${QT_INSTALL_DIR}")
#     message(STATUS "Qt 插件目录: ${QT_PLUGINS_DIR}")
#     message(STATUS "Qt Bin 目录: ${QT_BIN_PATH}")
    
#     # 创建 multimedia 插件目录
#     add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E make_directory 
#             "$<TARGET_FILE_DIR:${PROJECT_NAME}>/multimedia"
#         COMMENT "创建 multimedia 插件目录"
#     )
    
#     # 根据构建类型复制对应版本的插件
#     set(FFMPEG_PLUGIN_DEBUG "${QT_PLUGINS_DIR}/multimedia/ffmpegmediaplugind.dll")
#     set(FFMPEG_PLUGIN_RELEASE "${QT_PLUGINS_DIR}/multimedia/ffmpegmediaplugin.dll")
    
#     # 检查并复制 Debug 版本插件
#     if(EXISTS "${FFMPEG_PLUGIN_DEBUG}")
#         message(STATUS "找到 FFmpeg Debug 插件: ${FFMPEG_PLUGIN_DEBUG}")
#         add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#             COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                 "${FFMPEG_PLUGIN_DEBUG}"
#                 "$<TARGET_FILE_DIR:${PROJECT_NAME}>/multimedia/"
#             COMMENT "复制 FFmpeg Debug 插件"
#             CONFIGURATIONS Debug
#         )
#     else()
#         message(WARNING "未找到 FFmpeg Debug 插件: ${FFMPEG_PLUGIN_DEBUG}")
#     endif()
    
#     # 检查并复制 Release 版本插件
#     if(EXISTS "${FFMPEG_PLUGIN_RELEASE}")
#         message(STATUS "找到 FFmpeg Release 插件: ${FFMPEG_PLUGIN_RELEASE}")
#         add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#             COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                 "${FFMPEG_PLUGIN_RELEASE}"
#                 "$<TARGET_FILE_DIR:${PROJECT_NAME}>/multimedia/"
#             COMMENT "复制 FFmpeg Release 插件"
#             CONFIGURATIONS Release RelWithDebInfo MinSizeRel
#         )
#     else()
#         message(WARNING "未找到 FFmpeg Release 插件: ${FFMPEG_PLUGIN_RELEASE}")
#     endif()
    
#     # 复制 FFmpeg 核心库
#     file(GLOB FFMPEG_DLLS 
#         "${QT_BIN_PATH}/avcodec-*.dll"
#         "${QT_BIN_PATH}/avformat-*.dll"
#         "${QT_BIN_PATH}/avutil-*.dll"
#         "${QT_BIN_PATH}/swscale-*.dll"
#         "${QT_BIN_PATH}/swresample-*.dll"
#         "${QT_BIN_PATH}/avdevice-*.dll"
#         "${QT_BIN_PATH}/avfilter-*.dll"
#         "${QT_BIN_PATH}/postproc-*.dll"
#     )
    
#     if(FFMPEG_DLLS)
#         message(STATUS "找到 FFmpeg 库文件:")
#         foreach(DLL_FILE ${FFMPEG_DLLS})
#             get_filename_component(DLL_NAME "${DLL_FILE}" NAME)
#             message(STATUS "  - ${DLL_NAME}")
#         endforeach()
        
#         add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#             COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                 ${FFMPEG_DLLS}
#                 "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
#             COMMENT "复制 FFmpeg 库文件到输出目录"
#         )
#     else()
#         message(WARNING "未找到 FFmpeg DLL 文件!")
#         message(WARNING "请检查路径: ${QT_BIN_PATH}/av*.dll")
#     endif()
    
#     # 复制 Qt 运行时 DLL (根据构建类型)
#     set(QT_DLLS_DEBUG
#         "${QT_BIN_PATH}/Qt6Cored.dll"
#         "${QT_BIN_PATH}/Qt6Guid.dll"
#         "${QT_BIN_PATH}/Qt6Widgetsd.dll"
#         "${QT_BIN_PATH}/Qt6Multimediad.dll"
#         "${QT_BIN_PATH}/Qt6MultimediaWidgetsd.dll"
#         "${QT_BIN_PATH}/Qt6Networkd.dll"
#     )
    
#     set(QT_DLLS_RELEASE
#         "${QT_BIN_PATH}/Qt6Core.dll"
#         "${QT_BIN_PATH}/Qt6Gui.dll"
#         "${QT_BIN_PATH}/Qt6Widgets.dll"
#         "${QT_BIN_PATH}/Qt6Multimedia.dll"
#         "${QT_BIN_PATH}/Qt6MultimediaWidgets.dll"
#         "${QT_BIN_PATH}/Qt6Network.dll"
#     )
    
#     add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_if_different
#             ${QT_DLLS_DEBUG}
#             "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
#         COMMENT "复制 Qt Debug 运行时库"
#         CONFIGURATIONS Debug
#     )
    
#     add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_if_different
#             ${QT_DLLS_RELEASE}
#             "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
#         COMMENT "复制 Qt Release 运行时库"
#         CONFIGURATIONS Release RelWithDebInfo MinSizeRel
#     )
    
#     # 复制平台插件 (必需，也需要区分 Debug/Release)
#     if(EXISTS "${QT_PLUGINS_DIR}/platforms/qwindowsd.dll")
#         add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#             COMMAND ${CMAKE_COMMAND} -E make_directory 
#                 "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms"
#             COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                 "${QT_PLUGINS_DIR}/platforms/qwindowsd.dll"
#                 "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms/"
#             COMMENT "复制 Qt 平台插件 (Debug)"
#             CONFIGURATIONS Debug
#         )
#     endif()
    
#     if(EXISTS "${QT_PLUGINS_DIR}/platforms/qwindows.dll")
#         add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#             COMMAND ${CMAKE_COMMAND} -E make_directory 
#                 "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms"
#             COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                 "${QT_PLUGINS_DIR}/platforms/qwindows.dll"
#                 "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms/"
#             COMMENT "复制 Qt 平台插件 (Release)"
#             CONFIGURATIONS Release RelWithDebInfo MinSizeRel
#         )
#     endif()
    
#     # 复制样式插件 (可选)
#     if(EXISTS "${QT_PLUGINS_DIR}/styles/qwindowsvistastyle.dll")
#         add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#             COMMAND ${CMAKE_COMMAND} -E make_directory 
#                 "$<TARGET_FILE_DIR:${PROJECT_NAME}>/styles"
#             COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                 "${QT_PLUGINS_DIR}/styles/qwindowsvistastyle.dll"
#                 "$<TARGET_FILE_DIR:${PROJECT_NAME}>/styles/"
#             COMMENT "复制 Qt 样式插件"
#         )
#     endif()
    
#     # 打印最终输出信息
#     message(STATUS "====================================")
#     message(STATUS "构建完成后将自动复制以下文件:")
#     message(STATUS "1. FFmpeg 多媒体插件 -> multimedia/")
#     message(STATUS "2. FFmpeg 核心库 -> 根目录")
#     message(STATUS "3. Qt 运行时库 -> 根目录")
#     message(STATUS "4. Qt 平台插件 -> platforms/")
#     message(STATUS "====================================")
# endif()