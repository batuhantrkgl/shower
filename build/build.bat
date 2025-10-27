@echo off
REM Batch build script for VideoTimeline Client (Windows)

echo Building VideoTimeline Client...

REM Check for Qt
where qmake6 >nul 2>&1
if %errorlevel% equ 0 (
    set QMAKE=qmake6
    echo Using qmake6
    goto :build
)

where qmake >nul 2>&1
if %errorlevel% equ 0 (
    set QMAKE=qmake
    echo Using qmake
    goto :build
)

echo Error: qmake not found in PATH!
echo Please add Qt to your PATH:
echo   set PATH=^%PATH^%;C:\Qt\6.x.x\mingw_64\bin
exit /b 1

:build
if not exist "..\src" (
    echo Error: src directory not found!
    exit /b 1
)

REM Clean
if exist "..\src\Makefile" (
    echo Cleaning previous build...
    cd ..\src
    mingw32-make clean >nul 2>&1
    cd ..\build
)

REM Generate Makefile
echo Generating Makefile...
cd ..\src
%QMAKE% VideoTimeline.pro

if not exist "Makefile" (
    echo Error: Failed to generate Makefile!
    cd ..\build
    exit /b 1
)

REM Build
echo Compiling...
mingw32-make

cd ..\build

REM Check result
if exist "..\out\VideoTimeline.exe" (
    echo.
    echo ✓ Build successful!
    echo Executable: out\VideoTimeline.exe
) else (
    echo.
    echo ✗ Build failed!
    exit /b 1
)

