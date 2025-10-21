# VideoTimeline Server API Specification

## Base URL
`http://192.168.32.52:8080`

## Endpoints

### GET /api/schedule
Returns the current school schedule.

**Response:**
```json
{
  "school_start": "08:50",
  "school_end": "15:55",
  "blocks": [
    {
      "start_time": "08:50",
      "end_time": "09:30",
      "name": "Ders 1",
      "type": "lesson"
    },
    {
      "start_time": "09:30",
      "end_time": "09:40",
      "name": "Teneffüs",
      "type": "break"
    }
  ]
}
```

### GET /api/media/playlist
Returns the media playlist to display in sequence.

**Response:**
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

**Field Descriptions:**
- `type`: "video" or "image"
- `url`: Relative or absolute URL to the media file
- `duration`: For images, duration in milliseconds. For videos, use -1 for full duration
- `muted`: For videos, whether to play muted. Ignored for images

### GET /media/{filename}
Serves media files (videos, images).

**Response:** Binary file content

### POST /api/schedule (Server only)
Updates the schedule.

**Request Body:**
```json
{
  "school_start": "08:50",
  "school_end": "15:55",
  "blocks": [...]
}
```

### POST /api/media/upload (Server only)
Uploads new media file.

**Request:** Multipart form with file upload
