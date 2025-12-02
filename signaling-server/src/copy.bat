@echo off
setlocal enabledelayedexpansion

:: Set source and destination directories
set "SOURCE_DIR=D:\Qiang\Documents\QtCode\BytesScreenShare\signaling-server\src"
set "DEST_DIR=D:\Qiang\Documents\QtCode\signaling-server"

:: Check if source directory exists
if not exist "%SOURCE_DIR%" (
    echo Error: Source directory does not exist!
    pause
    exit /b 1
)

:: Create destination directory if it doesn't exist
if not exist "%DEST_DIR%" (
    echo Creating destination directory: %DEST_DIR%
    mkdir "%DEST_DIR%"
)

:: Counter for copied files
set /a file_count=0

echo Starting file copy operation...

:: Copy .h files
for /r "%SOURCE_DIR%" %%f in (*.h) do (
    echo Copying: %%~nxf
    copy /y "%%f" "%DEST_DIR%\%%~nxf" >nul
    set /a file_count+=1
)

:: Copy .hpp files
for /r "%SOURCE_DIR%" %%f in (*.hpp) do (
    echo Copying: %%~nxf
    copy /y "%%f" "%DEST_DIR%\%%~nxf" >nul
    set /a file_count+=1
)

:: Copy .cpp files
for /r "%SOURCE_DIR%" %%f in (*.cpp) do (
    echo Copying: %%~nxf
    copy /y "%%f" "%DEST_DIR%\%%~nxf" >nul
    set /a file_count+=1
)

for /r "%SOURCE_DIR%" %%f in (*.ui) do (
    echo Copying: %%~nxf
    copy /y "%%f" "%DEST_DIR%\%%~nxf" >nul
    set /a file_count+=1
)

for /r "%SOURCE_DIR%" %%f in (*.qrc) do (
    echo Copying: %%~nxf
    copy /y "%%f" "%DEST_DIR%\%%~nxf" >nul
    set /a file_count+=1
)

echo.
echo Operation completed! Total files copied: %file_count%
echo Source: %SOURCE_DIR%
echo Destination: %DEST_DIR%
echo.

pause