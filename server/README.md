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
- `GET /api/media/regenerate` - Force regenerate playlist
- `GET /api/media/toggle-auto-regenerate` - Toggle auto-regeneration on/off
- `POST /api/schedule` - Update schedule
- `POST /api/media/playlist` - Update playlist
- `GET /media/:filename` - Serve media files

## Auto-Playlist Generation

The server can automatically scan the `media/` folder and create playlists with smart defaults:
- Images: Duration based on filename (`*quick*` → 2s, `*long*` → 10s, default 5s)
- Videos: Mute status based on filename (`*mute*` → muted, else unmuted)

### Auto-Regeneration Control

The playlist includes an `auto_regenerate` boolean field that controls whether the playlist should be automatically updated when media files change:

- `"auto_regenerate": true` - Playlist will be regenerated when media files are added/removed/modified
- `"auto_regenerate": false` - Playlist will be preserved even when media files change

You can toggle this setting using:
- `GET /api/media/toggle-auto-regenerate` - Toggles the current setting
- `POST /api/media/playlist` - Update the playlist with your desired `auto_regenerate` value

This allows you to:
1. Set custom durations and preserve them when adding new media files
2. Manually curate playlists without them being overwritten
3. Still force regeneration when needed via `/api/media/regenerate`

## Requirements

- Qt6 (Core, Network modules)
- C++17 compiler

