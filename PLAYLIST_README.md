# Playlist Media Player

This document explains the new playlist functionality that allows the media player to handle multiple images and videos in sequence.

## Overview

The application now supports playlist-based media playback where the server can specify a sequence of media items to be played. Each item in the playlist can be:

- **Images** with specific display durations (e.g., 2 seconds, 10 seconds)
- **Videos** that play to completion, with mute/unmute control

When the playlist finishes, it automatically loops back to the first item.

## Server API Changes

### New Endpoint: `/api/media/playlist`

The server should now provide a playlist endpoint instead of the single media endpoint:

```json
{
  "items": [
    {
      "type": "image",
      "url": "/media/image1.jpg",
      "duration": 2000,
      "muted": false
    },
    {
      "type": "video",
      "url": "/media/video1.mp4", 
      "duration": -1,
      "muted": false
    },
    {
      "type": "video",
      "url": "/media/video2.mp4",
      "duration": -1,
      "muted": true
    },
    {
      "type": "image",
      "url": "/media/image2.jpg",
      "duration": 10000,
      "muted": false
    }
  ]
}
```

### Field Descriptions

- `type`: Either "image" or "video"
- `url`: Path to the media file (relative or absolute URL)
- `duration`: 
  - For images: Duration in milliseconds to display the image
  - For videos: Use -1 to play the full video duration
- `muted`: 
  - For videos: Whether to play with sound (false) or muted (true)
  - For images: Ignored

## Example Playlist Sequence

Based on your requirements, here's how the server data would look:

```json
{
  "items": [
    {
      "type": "image",
      "url": "/media/slide1.jpg",
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
    },
    {
      "type": "image",
      "url": "/media/schedule.png",
      "duration": 10000,
      "muted": false
    }
  ]
}
```

This would play:
1. Image for 2 seconds
2. Video with sound (full duration)
3. Video without sound (full duration) 
4. Image for 10 seconds
5. Loop back to item 1

## Architecture Changes

### New Components

1. **MediaPlayer Class** (`mediaplayer.h/cpp`)
   - Handles playlist logic and media switching
   - Manages timers for image durations
   - Controls video playback and muting
   - Automatically advances to next item

2. **Updated MediaItem Struct**
   - Extended to include mute settings
   - Support for both images and videos

3. **MediaPlaylist Struct** 
   - Container for playlist items
   - Tracks current position
   - Provides next/loop functionality

### Updated Components

1. **NetworkClient**
   - Now fetches `/api/media/playlist` instead of `/api/media/current`
   - Parses playlist JSON format
   - Emits `playlistReceived` signal

2. **VideoWidget**
   - Uses MediaPlayer for playlist management
   - Handles both video and image display
   - Automatic switching between media types

## Client Behavior

1. **Startup**: Client fetches initial playlist from server
2. **Playback**: Automatically starts playing the first item
3. **Image Display**: Shows image for specified duration, then moves to next
4. **Video Playback**: Plays video to completion (muted or unmuted), then moves to next
5. **Loop**: When playlist ends, starts over from the first item
6. **Error Handling**: On network errors, shows fallback image and continues

## Testing

Use the provided `example_playlist.json` file to test the playlist functionality. The server should return this format when the client requests `/api/media/playlist`.

## Migration from Single Media

The old `/api/media/current` endpoint is no longer used. Server implementations should:

1. Create the new `/api/media/playlist` endpoint
2. Convert single media responses to single-item playlists
3. Add duration and mute settings as needed

## Future Enhancements

- Priority playlists for urgent announcements
- Dynamic playlist updates without restart
- Transition effects between media items
- Scheduled playlist changes based on time of day