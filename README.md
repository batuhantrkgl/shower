# VideoTimeline - School Media Display System

A full-screen media display application for schools that shows schedules, images, and videos in a continuous loop.

## 🎯 Features

- **Full-screen media display** with automatic playlist looping
- **Auto server discovery** with `--auto` flag
- **Schedule timeline** showing current school period
- **Floating activity overlay** prominently displaying current activity (e.g., "Teneffüs")
- **Smart playlist system**:
  - Multiple images with customizable durations
  - Videos with mute/unmute support
  - Automatic looping when playlist ends
- **Server API** for remote management
- **Auto-generated playlists** from media folder

## 📁 Project Structure

```
shower/
├── src/                    # Client application source code
│   ├── *.cpp, *.h         # Source files
│   ├── VideoTimeline.pro  # Qt project file
│   ├── icons/             # UI icons
│   └── media/             # Default media files
├── videotimeline.sh       # Unified Master Script (run, build, server, install, fonts, media)
├── shower.sh              # Alias for videotimeline.sh
├── run.sh                 # Client runner wrapper -> videotimeline.sh run
├── run_hwaccel.sh         # GPU hardware acceleration runner wrapper
├── scripts/               # Scripts (all delegate to videotimeline.sh)
│   ├── build.sh          # Build wrapper
│   ├── install_linux.sh  # Universal Linux kiosk/systemd installer wrapper
│   └── install_rpi.sh    # Raspberry Pi wrapper
├── server/                # C++ HTTP server
│   ├── server.cpp        # Single-file HTTP server
│   ├── run.sh            # Server runner wrapper
│   ├── media/            # Media files folder
│   └── data/             # JSON data (playlists, schedules)
├── data/                  # Schedules, playlists, and media
└── README.md              # This file
```

## 🌟 Unified Master Script (`videotimeline.sh` / `shower.sh`)

All project scripts have been consolidated into **one master script**:

```bash
# 1. Run Client:
./videotimeline.sh run --auto                 # Auto-discover server & start playback
./videotimeline.sh run --hwaccel --nvidia     # Run with NVIDIA GPU acceleration

# 2. Manage Server:
./videotimeline.sh server                     # Run server in foreground
./videotimeline.sh server --detach            # Run server in background
./videotimeline.sh server --status            # Check server health
./videotimeline.sh server --kill              # Stop running servers

# 3. Build:
./videotimeline.sh build                      # Build client and server
./videotimeline.sh build --deps               # Auto-install build dependencies (all distros)
./videotimeline.sh build clean                # Clean build artifacts

# 4. Universal Linux Installation (all distros):
sudo ./videotimeline.sh install               # Full kiosk setup, systemd, autologin, fonts

# 5. Fonts:
./videotimeline.sh fonts                      # Install SF Pro Display fonts

# 6. Media Utilities:
./videotimeline.sh media reencode data/media/special  # Convert AV1/VP9 videos to H.264
```

*(Note: Traditional scripts like `run.sh`, `scripts/build.sh`, `scripts/install_linux.sh`, and `server/run.sh` remain fully operational as transparent wrappers.)*

## 🚀 Quick Start

### Installing Dependencies

**Automatic (Any Linux Distribution):**
```bash
./videotimeline.sh build --deps
```

**Ubuntu / Debian / Raspberry Pi OS:**
```bash
sudo apt install qt6-base-dev qt6-multimedia-dev build-essential cmake
```

**Fedora / RHEL / CentOS / Rocky:**
```bash
sudo dnf install qt6-qtbase-devel qt6-qtmultimedia-devel gcc-c++ cmake
```

**Arch Linux / Manjaro:**
```bash
sudo pacman -S qt6-base qt6-multimedia base-devel cmake
```

**openSUSE / SLES:**
```bash
sudo zypper install qt6-base-devel qt6-multimedia-devel gcc-c++ cmake
```

**Alpine Linux:**
```bash
apk add build-base cmake qt6-qtbase-dev qt6-qtmultimedia-dev
```

**macOS:**
```bash
brew install qt6 cmake
```

**Windows:**
Download Qt6 from https://www.qt.io/download-open-source or install via MSYS2: `pacman -S mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-qt6-multimedia`

### Building and Running

**Linux/macOS:**
```bash
# Client
cd build && bash build.sh

# Server  
cd server && make && ./server
```

**Windows (PowerShell):**
```powershell
# Client
cd build
.\build.ps1

# Server
cd server
.\build.ps1
.\server.exe

# Run Client
cd ..\out
.\VideoTimeline.exe --auto
```

**Run Client:**
```bash
cd out
./VideoTimeline --auto
```

### What Happens Next?

1. Server auto-generates playlist from `server/media/` folder
2. Client automatically discovers server on network
3. Media displays in fullscreen and loops continuously

## 🎬 Media Playlist System

### How It Works

1. Server scans `server/media/` folder
2. Automatically creates a playlist with smart detection
3. Client fetches and plays playlist
4. Loops automatically when finished

### Filename-Based Detection

**Images:**
- `*quick*` or `*short*` → 2 seconds
- `*long*` or `*schedule*` → 10 seconds
- `*banner*` or `*logo*` → 3 seconds
- Default → 5 seconds

**Videos:**
- `*mute*`, `*silent*`, or `*background*` → Muted
- `*sound*`, `*audio*`, or `*announcement*` → Unmuted
- Default → Unmuted

### Example Playlist

```json
{
  "items": [
    {
      "type": "image",
      "url": "/media/welcome.jpg",
      "duration": 2000,
      "muted": false
    },
    {
      "type": "video",
      "url": "/media/announcement.mp4",
      "duration": -1,
      "muted": false
    },
    {
      "type": "video",
      "url": "/media/background.mp4",
      "duration": -1,
      "muted": true
    }
  ]
}
```

## 🌐 Server API

### Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/schedule` | GET | Get school schedule |
| `/api/media/playlist` | GET | Get media playlist |
| `/api/media/regenerate` | GET | Regenerate playlist from media folder |
| `/api/schedule` | POST | Update schedule |
| `/api/media/playlist` | POST | Update playlist |

### Auto Server Discovery

The client can automatically find the server on your network:

```bash
./VideoTimeline --auto
```

**Discovery Strategy:**
1. Tries common IPs (router, typical devices)
2. Scans local subnet
3. Falls back to common private networks
4. Uses localhost as last resort

## 🔧 Configuration

### Manual Playlist Editing

Edit `server/data/playlist.json` to customize:
- Media order
- Durations
- Mute settings

### Schedule Configuration

Edit `server/data/schedule.json` to set school schedule.

### Service Installation (Raspberry Pi)

```bash
sudo cp videotimeline.service /etc/systemd/system/
sudo systemctl enable videotimeline.service
sudo systemctl start videotimeline.service
```

## 📝 Client Commands

```bash
# Display version
./VideoTimeline --version

# Auto-discover and connect to server
./VideoTimeline --auto

# Use specific server
./VideoTimeline --network 192.168.1.100:3232

# Test UI scaling with different DPI (for testing on different resolutions)
./VideoTimeline --dpi 144  # Test with 144 DPI (150% scaling)
./VideoTimeline --dpi 192  # Test with 192 DPI (200% scaling)

# Use default connection
./VideoTimeline
```

## 🏗️ Building from Source

### Requirements

- Qt6 (or Qt5 with compatibility)
- C++ compiler
- CMake or qmake
- Python 3 (for server)

### Build Process

1. Install dependencies:
   ```bash
   # Qt6
   sudo apt install qt6-base-dev qt6-multimedia-dev
   
   # Server dependencies
   cd server
   pip install -r requirements.txt
   ```

2. Build:
   ```bash
   cd build
   ./build.sh
   ```

3. Run:
   ```bash
   ./out/VideoTimeline
   ```

## 🎨 Architecture

### Client Components

- **MainWindow**: Full-screen window management
- **VideoWidget**: Media display (video/image switching)
- **TimelineWidget**: Status bar showing current activity, time remaining, and next activity
- **ActivityOverlay**: Floating overlay displaying current activity (e.g., "Teneffüs", "Ders 1") positioned above the timeline
- **NetworkClient**: Server communication
- **MediaPlayer**: Playlist management and playback

### Server Components

- **server.py**: HTTP server with API endpoints
- **Auto-playlist generation**: Scans media folder
- **Smart defaults**: Duration and mute detection

## 📦 Dependencies

**Client:**
- Qt6 (qt6-base, qt6-multimedia)

**Server:**
- C++17 compiler
- Qt6 (qt6-core, qt6-network)

## 🔍 Troubleshooting

**Client can't find server:**
- Check server is running: `./server` (in server directory)
- Try `--auto` flag
- Check firewall settings

**Media not playing:**
- Check file formats are supported
- Verify media files exist in `server/media/`
- Check server logs for errors

**Build fails:**
- Ensure Qt6 is installed
- Check CMakeLists.txt paths are correct
- Try cleaning and rebuilding

## 📄 License

Made by @Batuhantrkgl  
Written in C++ using the Qt Framework

## 🛠️ Development

### Adding New Features

1. Client changes: Modify files in `src/`
2. Server changes: Modify `server/server.py`
3. Rebuild: `cd build && ./build.sh`

### Testing

```bash
# Test server
cd server
python3 test_server.py

# Test playlist generation
python3 test_playlist.py
```

---

**Version:** See `./VideoTimeline --version`  
**Build Date:** See `./VideoTimeline --version`

