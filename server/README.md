# VideoTimeline Server

A simple HTTP server written in C++ using Qt for serving media playlists and schedules.

## Building

### Using Make

```bash
cd server
make
```

### Using CMake

```bash
cd server
mkdir build
cd build
cmake ..
make
```

## Running

```bash
./server [PORT]
```

Default port is 8080. The server will:
- Create `data/` and `media/` directories if they don't exist
- Generate default schedule in `data/schedule.json`
- Auto-generate playlist from files in `media/` folder

## Endpoints

- `GET /api/schedule` - Get schedule
- `GET /api/media/playlist` - Get playlist
- `GET /api/media/regenerate` - Regenerate playlist
- `POST /api/schedule` - Update schedule
- `POST /api/media/playlist` - Update playlist
- `GET /media/:filename` - Serve media files

## Auto-Playlist Generation

The server automatically scans the `media/` folder and creates playlists with smart defaults:
- Images: Duration based on filename (`*quick*` → 2s, `*long*` → 10s, default 5s)
- Videos: Mute status based on filename (`*mute*` → muted, else unmuted)

## Requirements

- Qt6 (Core, Network modules)
- C++17 compiler

