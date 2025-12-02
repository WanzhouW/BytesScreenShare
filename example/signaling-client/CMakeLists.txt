cmake_minimum_required(VERSION 3.19)
project(signaling-client LANGUAGES CXX)

# 设置 Qt 安装路径
# 如果使用VS Code 或其他IDE 自动构建的时候需要修改此路径
set(CMAKE_PREFIX_PATH "f:/Qt/6.8.3/msvc2022_64")

# 启用自动化功能和 C++ 标准
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找 Qt6 所需模块
find_package(Qt6 REQUIRED COMPONENTS Core Network WebSockets Widgets)

# 定义源文件和头文件
set(SRCS
    main.cpp
)

set(HEADERS
    TestClient.hpp
)

# 定义 UI 文件
set(UIS
    TestClient.ui
)
qt_wrap_ui(UI_SOURCES ${UIS})

# 定义资源文件
set(RESOURCES
    TestClient.qrc
)
qt_add_resources(RESOURCE_SOURCES ${RESOURCES})

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SRCS} ${HEADERS} ${UI_SOURCES} ${RESOURCE_SOURCES})

# 链接 Qt6 模块
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    Qt6::Widgets
)

# 设置头文件包含路径
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 添加自定义命令，在构建后运行 windeployqt
if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_PREFIX_PATH}/bin/windeployqt.exe $<TARGET_FILE:${PROJECT_NAME}>
        COMMENT "Running windeployqt to deploy Qt dependencies..."
    )
endif()