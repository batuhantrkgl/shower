# VideoTimeline Compilation Guide

This guide helps you resolve compilation issues, especially Qt6 compatibility problems.

## Quick Fix for Current Error

### Error: `cannot convert 'QNetworkRequest(QUrl)' to 'const QNetworkRequest&'`

**Solution**: The project now includes Qt6 compatibility fixes. Use the updated files:

1. **Use the compatibility header**: All Qt-specific code now uses `qt6compat.h`
2. **Updated MediaPlayer**: Fixed network request creation for Qt6
3. **Build scripts**: Added universal build support

## Build Methods (Recommended Order)

### Method 1: Universal Build Script (Recommended)
```bash
chmod +x build.sh
./build.sh check    # Check environment
./build.sh          # Auto-build
```

### Method 2: CMake (Modern, Cross-platform)
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j4
```

### Method 3: qmake (Traditional Qt)
```bash
qmake VideoTimeline.pro
make -j4
```

## Qt Version Compatibility

### Qt6 (Recommended)
- **Supported**: Qt 6.2.0+
- **Features**: All modern multimedia features
- **Build**: Use CMake or updated qmake

### Qt5 (Legacy Support)
- **Supported**: Qt 5.15+
- **Limitations**: Some multimedia features may differ
- **Build**: Use qmake with compatibility layer

## Common Compilation Errors & Solutions

### 1. QNetworkRequest Constructor Error
**Error**: `cannot convert 'QNetworkRequest(QUrl)' to 'const QNetworkRequest&'`

**Solution**:
```cpp
// OLD (Qt5 style):
QNetworkRequest request(QUrl(url));

// NEW (Qt6 compatible):
QNetworkRequest request = createNetworkRequest(url);
```

### 2. QMediaPlayer Error Signal
**Error**: `errorOccurred` signal not found

**Solution**: Use compatibility macro:
```cpp
// OLD:
connect(player, &QMediaPlayer::errorOccurred, ...);

// NEW:
connect(player, MEDIAPLAYER_ERROR_SIGNAL, ...);
```

### 3. QAudioOutput Missing
**Error**: QAudioOutput not found

**Solution**: Use compatibility setup:
```cpp
// OLD:
QAudioOutput *audio = new QAudioOutput(this);
player->setAudioOutput(audio);

// NEW:
SETUP_AUDIO_OUTPUT(player);
```

### 4. Missing Headers
**Error**: Various Qt headers not found

**Solution**: Include compatibility header first:
```cpp
#include "qt6compat.h"  // Must be first Qt include
#include <QMediaPlayer>
// ... other includes
```

## Platform-Specific Issues

### Linux (ARM64/Raspberry Pi)
```bash
# Install Qt6 development packages
sudo apt update
sudo apt install qt6-base-dev qt6-multimedia-dev

# Build with specific Qt version
export QT_SELECT=6
qmake VideoTimeline.pro
make -j$(nproc)
```

### Windows (MinGW)
```bash
# Use Qt Creator or command line
qmake VideoTimeline.pro
mingw32-make -j4

# Or use CMake with specific generator
cmake .. -G "MinGW Makefiles"
cmake --build . -j4
```

### macOS
```bash
# Use Homebrew Qt or official installer
qmake VideoTimeline.pro
make -j$(sysctl -n hw.ncpu)

# Or CMake
cmake .. -DCMAKE_PREFIX_PATH=/usr/local/opt/qt6
cmake --build . -j$(sysctl -n hw.ncpu)
```

## Dependency Requirements

### Required Qt Modules
- Qt6Core / Qt5Core
- Qt6Widgets / Qt5Widgets  
- Qt6Multimedia / Qt5Multimedia
- Qt6MultimediaWidgets / Qt5MultimediaWidgets
- Qt6Network / Qt5Network

### Install Commands

#### Ubuntu/Debian
```bash
# Qt6
sudo apt install qt6-base-dev qt6-multimedia-dev qt6-tools-dev

# Qt5 (fallback)
sudo apt install qtbase5-dev qtmultimedia5-dev qttools5-dev
```

#### CentOS/RHEL/Fedora
```bash
# Qt6
sudo dnf install qt6-qtbase-devel qt6-qtmultimedia-devel

# Qt5 (fallback)
sudo dnf install qt5-qtbase-devel qt5-qtmultimedia-devel
```

#### Windows
- Download Qt installer from qt.io
- Select Qt 6.x with MinGW or MSVC
- Include Multimedia components

#### macOS
```bash
# Homebrew
brew install qt6

# Or download from qt.io
```

## Build Environment Check

### Verify Qt Installation
```bash
# Check Qt version
qmake -v
qmake6 -v  # If available

# Check modules
find /usr -name "Qt*Multimedia*" 2>/dev/null
```

### Check Build Tools
```bash
# Required tools
which make || which mingw32-make || which nmake
which cmake
which g++ || which clang++
```

## Troubleshooting Specific Errors

### Error: `Project ERROR: Unknown module(s) in QT: multimedia`
**Solution**: Install Qt multimedia development packages
```bash
# Ubuntu/Debian
sudo apt install qtmultimedia5-dev  # Qt5
sudo apt install qt6-multimedia-dev  # Qt6

# Build again
qmake clean && qmake && make
```

### Error: `mediaplayer.h: No such file or directory`
**Solution**: Ensure all source files are in the same directory
```bash
ls -la *.h *.cpp
# Should show: mediaplayer.h, mediaplayer.cpp, qt6compat.h
```

### Error: Build succeeds but executable crashes
**Solution**: Check Qt runtime libraries
```bash
# Linux: Check library dependencies
ldd ./out/VideoTimeline

# Windows: Ensure Qt DLLs are in PATH or app directory
# macOS: Check with otool -L
```

## Clean Build Process

### Complete Clean Build
```bash
# Method 1: Use build script
./build.sh clean
./build.sh

# Method 2: Manual clean
make clean 2>/dev/null || true
rm -rf build/ out/ Makefile* .qmake.stash
qmake VideoTimeline.pro
make -j4
```

## Advanced Build Options

### Debug Build
```bash
# qmake
qmake CONFIG+=debug VideoTimeline.pro
make

# CMake
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

### Static Build
```bash
# qmake (if Qt supports static)
qmake CONFIG+=static VideoTimeline.pro
make

# CMake
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
cmake --build .
```

## Testing Compilation

### Verify Build Success
```bash
# Check executable exists
ls -la out/VideoTimeline*

# Test basic functionality
./out/VideoTimeline --version

# Check dependencies (Linux)
ldd ./out/VideoTimeline | grep -i qt
```

### Quick Functionality Test
```bash
# Start server in background
cd server && python3 start_server.py &

# Test client connection
sleep 2
curl http://localhost:8080/api/media/playlist

# Stop server
pkill -f start_server.py
```

## Getting Help

### Collect Debug Information
```bash
# System info
uname -a
qt-config --version 2>/dev/null || qmake -v

# Build info
./build.sh check

# Error logs
make 2>&1 | tee build.log
```

### Common Solutions Summary
1. **Use compatibility header**: `#include "qt6compat.h"`
2. **Install complete Qt dev packages**: multimedia + widgets + base
3. **Use build script**: `./build.sh` for automatic detection
4. **Check Qt version**: Prefer Qt 6.2+ or Qt 5.15+
5. **Clean build**: Remove all artifacts and rebuild

### Still Having Issues?
1. Check this guide for your specific error
2. Verify all dependencies are installed
3. Try different build method (CMake vs qmake)
4. Use the universal build script for automatic handling
5. Check Qt documentation for your platform

## Success Indicators

When compilation succeeds, you should see:
- ✅ Executable created in `out/` directory
- ✅ No linker errors about Qt libraries
- ✅ Application starts without missing library errors
- ✅ Basic UI displays (even if no media)

The playlist functionality will work once the server is running and media files are available.