# Setup Guide for VideoTimeline

Complete installation and setup instructions for Linux, macOS, and Windows.

## Prerequisites

All platforms require:
- **Qt6** (Core, Network, Multimedia, Widgets modules)
- **C++17 compatible compiler**
- **CMake** (optional, for alternative build method)

## Linux Setup

### Ubuntu/Debian

```bash
# Install Qt6 and development tools
sudo apt update
sudo apt install -y qt6-base-dev qt6-multimedia-dev qt6-network-dev \
  build-essential cmake git

# Clone or navigate to the project
cd shower

# Build the client
cd build
bash build.sh

# Build the server
cd ../server
make

# Run the server
./server

# In another terminal, run the client
cd ../out
./VideoTimeline --auto
```

### Fedora/RHEL

```bash
# Install Qt6
sudo dnf install -y qt6-qtbase-devel qt6-qtmultimedia-devel \
  qt6-qtnetwork-devel cmake gcc-c++ git

# Build (same as Ubuntu)
cd build && bash build.sh
cd ../server && make
```

### Arch Linux

```bash
# Install Qt6
sudo pacman -S qt6-base qt6-multimedia qt6-network cmake base-devel git

# Build
cd build && bash build.sh
cd ../server && make
```

### Running as a Service (Linux)

Create `/etc/systemd/system/videotimeline.service`:

```ini
[Unit]
Description=VideoTimeline Display
After=network.target

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/shower
ExecStart=/home/pi/shower/out/VideoTimeline --auto
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Then:
```bash
sudo systemctl enable videotimeline
sudo systemctl start videotimeline
```

---

## macOS Setup

### Using Homebrew

```bash
# Install Qt6
brew install qt6

# Install Xcode command line tools (if needed)
xcode-select --install

# Build the client
cd build
bash build.sh

# Build the server
cd ../server
make
```

### Without Homebrew

```bash
# Download Qt6 from qt.io
# Install Qt 6.x for macOS

# Set Qt6 path (add to ~/.zshrc or ~/.bash_profile)
export PATH="/Users/$(whoami)/Qt/6.x.x/macos/bin:$PATH"

# Build
cd build && bash build.sh
cd ../server && make
```

### Running on macOS

```bash
# Server
./server

# Client (in another terminal)
./out/VideoTimeline.app/Contents/MacOS/VideoTimeline --auto
```

---

## Windows Setup

### Using Qt6 Installer

1. **Download Qt6** from https://www.qt.io/download-open-source
   - Install Qt 6.x.x
   - Install MSVC 2019 64-bit or MinGW 64-bit

2. **Install CMake** from https://cmake.org/download/

3. **Open PowerShell** or **Command Prompt**

4. **Set Qt6 path**:
   ```powershell
   $env:Path += ";C:\Qt\6.x.x\msvc2019_64\bin"
   ```

5. **Build**:
   ```powershell
   cd build
   bash build.sh
   
   cd ..\server
   make
   ```

### Using vcpkg (Alternative)

```powershell
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install Qt6
.\vcpkg install qt6-base:x64-windows
.\vcpkg install qt6-multimedia:x64-windows
.\vcpkg install qt6-network:x64-windows

# Build
cd ..\shower\build
cmake .. -DCMAKE_TOOLCHAIN_FILE=..\..\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build .
```

### Running on Windows

```powershell
# Server (in one terminal)
cd server
.\server.exe

# Client (in another terminal)
cd out
.\VideoTimeline.exe --auto
```

---

## Alternative: Docker Setup

### Dockerfile

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
  qt6-base-dev qt6-multimedia-dev qt6-network-dev \
  build-essential cmake \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Build
WORKDIR /app/src
qmake VideoTimeline.pro
make

WORKDIR /app/server
qmake server.pro
make

# Run
CMD ["./server"]
```

### Build & Run

```bash
docker build -t videotimeline .
docker run -p 8080:8080 -v $(pwd)/server/media:/app/server/media videotimeline
```

---

## Quick Start (All Platforms)

### 1. Install Dependencies

**Linux:**
```bash
sudo apt install qt6-base-dev qt6-multimedia-dev build-essential
```

**macOS:**
```bash
brew install qt6
```

**Windows:**
- Download Qt from qt.io

### 2. Build Everything

```bash
# Build client
cd build
bash build.sh

# Build server
cd ../server
make
```

### 3. Add Media Files

```bash
# Copy your media files
cp *.jpg *.mp4 *.png server/media/
```

### 4. Run

```bash
# Terminal 1: Start server
cd server
./server

# Terminal 2: Start client
cd out
./VideoTimeline --auto
```

---

## Troubleshooting

### Qt Not Found

**Linux:**
```bash
export PATH="/usr/lib/qt6/bin:$PATH"
```

**macOS:**
```bash
export PATH="/usr/local/opt/qt6/bin:$PATH"
```

**Windows:**
```powershell
$env:Path += ";C:\Qt\6.x.x\msvc2019_64\bin"
```

### Build Errors

**Check Qt version:**
```bash
qmake --version
# Should show Qt 6.x.x
```

**Clean build:**
```bash
cd src
make clean
rm -rf out/*
make
```

### Missing Modules

Ensure all required modules are installed:

- `qt6-base`
- `qt6-multimedia`
- `qt6-network`
- `qt6-widgets`

### Permission Errors (Linux)

```bash
sudo chown -R $USER:$USER /usr/local/bin
```

---

## Platform-Specific Notes

### Linux
- Tested on Ubuntu 20.04+, Debian 11+, Fedora 35+
- Requires X11 or Wayland for display
- Fullscreen works best with KMS/GVT

### macOS
- Tested on macOS 11+
- Requires Xcode Command Line Tools
- Fullscreen needs accessibility permissions

### Windows
- Tested on Windows 10/11
- Requires Windows SDK
- Fullscreen requires admin on some systems

---

## Verification

After setup, verify everything works:

```bash
# Check server
curl http://localhost:8080/api/schedule

# Check playlist
curl http://localhost:8080/api/media/playlist

# Server should return JSON
```

If you get JSON responses, you're ready to go! 🎉

