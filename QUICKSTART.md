# Quick Start Guide

Get VideoTimeline running in 5 minutes!

## Step 1: Install Qt6

### Linux (Ubuntu/Debian)
```bash
sudo apt install qt6-base-dev qt6-multimedia-dev qt6-network-dev build-essential
```

### macOS
```bash
brew install qt6
```

### Windows
Download from https://www.qt.io/download-open-source

## Step 2: Build

```bash
# Build client
cd build && bash build.sh

# Build server  
cd ../server && make
```

## Step 3: Add Media Files

```bash
# Copy your media to server
cp your-images.jpg server/media/
cp your-videos.mp4 server/media/
```

## Step 4: Run

**Terminal 1 (Server):**
```bash
cd server
./server
```

**Terminal 2 (Client):**
```bash
cd out
./VideoTimeline --auto
```

## Done! 🎉

The client will auto-discover the server and start displaying your media.

---

## Tips

- **Video naming**: Name with `mute`, `silent`, or `background` for muted videos
- **Image timing**: Use `quick`, `short`, `long`, or `schedule` in filenames for smart durations
- **Full playlist control**: Edit `server/data/playlist.json` to customize order and timing

## Need Help?

See [SETUP.md](SETUP.md) for detailed instructions and troubleshooting.

