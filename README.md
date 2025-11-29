程序已经编译好了  找到build/Debug 下的shared_screen.exe  双击可执行

增加了加入会议的窗口 点击加入会议 进入主页面  增加了加入会议的按钮的实现函数 后序的逻辑接着写即可   摄像头功能已正常调用  录制功能基本已实现 但屏幕捕获功能为实现   各个函数都写了点注释    各个按钮的点击函数的逻辑大家往下接下写即可  前面的就不要删改了 
----------------------------------------------------------------------------------------------------------
修改： 1.删除了图标名字的中文部分  只保留英文部分  防止编译错误   图标文件夹在build/icons
       2.在shared_screen.cpp下更新图标名称
----------------------------------------------------------------------------------------------------------
大家下载后再CMakeLists.txt里将set(CMAKE_PREFIX_PATH "d:/Qt/6.8.3/msvc2022_64") # Qt Kit Dir  设置为自己的qt路径 

若在运行中遇到缺少dll文件 就去下载的qt下的bin文件夹里找到缺少的文件 复制到build下  或 在终端debug文件夹下运行命令
H:/qt6/6.8.3/msvc2022_64/bin/windeployqt.exe shared_screen.exe,前面的是你自己的qt目录位置。然后
就可以运行。

摄像头若遇到问题  在CMakeLists.txt   注释这行代码 WIN32 # If you need a terminal for debug, please comment this statement  把下面的注释打开 可以看一下打印信息 找到问题     在shared_screen.cpp 文件中 取消注释diagnoseMultimediaSupport调用的地方  会在命令提示符中显示一些信息  各个函数中 qDebug() 也是打印信息 需要的话也可以打开注释   修改了CMakeLists.txt文件后会自动配置  若未配置 快捷键Ctrl+shift+p（vscode编译器）  点击CMake:配置 

我用的编译器是vscode  修改了代码重新编译  编译的快捷键为 Ctrl+shift+B 选择CMake:清理重新生成

出现以下信息表示成功，可以重新执行exe文件。  
    正在执行任务: CMake: 清理重新生成 

    工作区为 /d:/shared_screen
    正在忽略此任务中定义的目标。
    已启动 清理...
    D:\Qt\Tools\CMake_64\bin\cmake.EXE --build d:/shared_screen/build --config Debug --target clean -j 8 --
    MSBuild version 17.14.19+164abd434 for .NET Framework

    已成功完成 清理。
    已启动 生成...
    D:\Qt\Tools\CMake_64\bin\cmake.EXE --build d:/shared_screen/build --config Debug --target ALL_BUILD -j 8 --
    MSBuild version 17.14.19+164abd434 for .NET Framework

    1>Checking Build System
    Automatic MOC and UIC for target shared_screen
    Building Custom Rule D:/shared_screen/CMakeLists.txt
    mocs_compilation_Debug.cpp
    main.cpp
    shared_screen.cpp
    正在生成代码...
    shared_screen.vcxproj -> D:\shared_screen\build\Debug\shared_screen.exe
    已成功完成 生成。
    *  终端将被任务重用，按任意键关闭。


若是图标不显示 可在shared_screen.cpp下用到图标的地方换成绝对路径试试  图标文件夹在build/icons