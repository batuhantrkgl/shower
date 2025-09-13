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

### GET /api/media/current
Returns the current media to display.

**Response:**
```json
{
  "type": "video|image",
  "url": "/media/current.mp4",
  "duration": 30000
}
```

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
