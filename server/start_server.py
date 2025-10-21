#!/usr/bin/env python3
"""
Simple VideoTimeline Server Launcher
This version removes problematic dependencies and focuses on core functionality
"""

import json
import os
import http.server
import socketserver
from datetime import datetime
import urllib.parse
import shutil
import logging

# --- Configuration ---
PORT = 8080
DATA_DIR = "data"
MEDIA_DIR = "media"
DEFAULT_SCHEDULE_FILE = os.path.join(DATA_DIR, "schedule.json")
CURRENT_MEDIA_FILE = os.path.join(DATA_DIR, "current_media.json")
PLAYLIST_FILE = os.path.join(DATA_DIR, "playlist.json")
DEFAULT_MEDIA_FILENAME = "default.jpg"
DEFAULT_MEDIA_PATH = os.path.join(MEDIA_DIR, DEFAULT_MEDIA_FILENAME)

# --- Logging Setup ---
logging.basicConfig(
    level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)


# --- Helper Functions ---
def load_json_file(filepath, default_data=None):
    """Loads a JSON file, returning default_data if not found or invalid."""
    if os.path.exists(filepath):
        try:
            with open(filepath, "r", encoding="utf-8") as f:
                return json.load(f)
        except json.JSONDecodeError:
            logger.error(f"Error decoding JSON from {filepath}. Using default data.")
        except IOError as e:
            logger.error(f"Error reading file {filepath}: {e}. Using default data.")
    logger.info(f"File {filepath} not found or inaccessible. Using default data.")
    return default_data


def save_json_file(filepath, data):
    """Saves data to a JSON file."""
    try:
        os.makedirs(os.path.dirname(filepath), exist_ok=True)
        with open(filepath, "w", encoding="utf-8") as f:
            json.dump(data, f, ensure_ascii=False, indent=2)
        return True
    except IOError as e:
        logger.error(f"Error writing to file {filepath}: {e}")
        return False


def generate_playlist_from_media_dir():
    """
    Automatically generate a playlist by scanning the media directory.
    Returns a playlist structure with smart defaults for durations and mute settings.
    """
    playlist_items = []

    if not os.path.exists(MEDIA_DIR):
        logger.warning(f"Media directory {MEDIA_DIR} does not exist")
        return {"items": []}

    # Supported media file extensions
    image_extensions = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp"}
    video_extensions = {".mp4", ".avi", ".mov", ".wmv", ".flv", ".webm", ".mkv"}

    # Get all media files and sort them alphabetically
    media_files = []
    for filename in os.listdir(MEDIA_DIR):
        if filename.startswith("."):  # Skip hidden files
            continue
        filepath = os.path.join(MEDIA_DIR, filename)
        if os.path.isfile(filepath):
            media_files.append(filename)

    media_files.sort()  # Sort alphabetically for consistent ordering

    for filename in media_files:
        file_ext = os.path.splitext(filename)[1].lower()

        if file_ext in image_extensions:
            # Image defaults
            duration = 5000  # 5 seconds default for images

            # Smart duration based on filename
            if "quick" in filename.lower() or "short" in filename.lower():
                duration = 2000  # 2 seconds
            elif "long" in filename.lower() or "schedule" in filename.lower():
                duration = 10000  # 10 seconds
            elif "banner" in filename.lower() or "logo" in filename.lower():
                duration = 3000  # 3 seconds

            playlist_items.append(
                {
                    "type": "image",
                    "url": f"/media/{filename}",
                    "duration": duration,
                    "muted": False,  # Not applicable for images, but keep for consistency
                }
            )

        elif file_ext in video_extensions:
            # Video defaults
            muted = False  # Default to unmuted

            # Smart mute detection based on filename
            if (
                "mute" in filename.lower()
                or "silent" in filename.lower()
                or "background" in filename.lower()
            ):
                muted = True
            elif (
                "sound" in filename.lower()
                or "audio" in filename.lower()
                or "announcement" in filename.lower()
            ):
                muted = False

            playlist_items.append(
                {
                    "type": "video",
                    "url": f"/media/{filename}",
                    "duration": -1,  # Full video duration
                    "muted": muted,
                }
            )

    # If no media files found, create a default entry
    if not playlist_items:
        playlist_items.append(
            {
                "type": "image",
                "url": f"/media/{DEFAULT_MEDIA_FILENAME}",
                "duration": 5000,
                "muted": False,
            }
        )

    playlist_data = {"items": playlist_items}

    # Save the generated playlist
    save_json_file(PLAYLIST_FILE, playlist_data)

    logger.info(f"Generated playlist with {len(playlist_items)} items")
    return playlist_data


# --- API Endpoints Handlers ---
class APIHandlers:
    """A collection of methods to handle specific API endpoints."""

    def __init__(self, server_instance):
        self.server = server_instance
        self._default_schedule_data = {
            "school_start": "08:50",
            "school_end": "15:55",
            "blocks": [
                {
                    "start_time": "08:50",
                    "end_time": "09:30",
                    "name": "Ders 1",
                    "type": "lesson",
                },
                {
                    "start_time": "09:30",
                    "end_time": "09:40",
                    "name": "Teneffüs",
                    "type": "break",
                },
                {
                    "start_time": "09:40",
                    "end_time": "10:20",
                    "name": "Ders 2",
                    "type": "lesson",
                },
                {
                    "start_time": "10:20",
                    "end_time": "10:30",
                    "name": "Teneffüs",
                    "type": "break",
                },
                {
                    "start_time": "10:30",
                    "end_time": "11:10",
                    "name": "Ders 3",
                    "type": "lesson",
                },
                {
                    "start_time": "11:10",
                    "end_time": "11:20",
                    "name": "Teneffüs",
                    "type": "break",
                },
                {
                    "start_time": "11:20",
                    "end_time": "12:00",
                    "name": "Ders 4",
                    "type": "lesson",
                },
                {
                    "start_time": "12:00",
                    "end_time": "12:45",
                    "name": "Öğle Arası",
                    "type": "lunch",
                },
                {
                    "start_time": "12:45",
                    "end_time": "13:25",
                    "name": "Ders 5",
                    "type": "lesson",
                },
                {
                    "start_time": "13:25",
                    "end_time": "13:35",
                    "name": "Teneffüs",
                    "type": "break",
                },
                {
                    "start_time": "13:35",
                    "end_time": "14:15",
                    "name": "Ders 6",
                    "type": "lesson",
                },
                {
                    "start_time": "14:15",
                    "end_time": "14:25",
                    "name": "Teneffüs",
                    "type": "break",
                },
                {
                    "start_time": "14:25",
                    "end_time": "15:05",
                    "name": "Ders 7",
                    "type": "lesson",
                },
                {
                    "start_time": "15:05",
                    "end_time": "15:15",
                    "name": "Teneffüs",
                    "type": "break",
                },
                {
                    "start_time": "15:15",
                    "end_time": "15:55",
                    "name": "Ders 8",
                    "type": "lesson",
                },
            ],
        }
        self._default_current_media_data = {
            "type": "image",
            "url": f"/media/{DEFAULT_MEDIA_FILENAME}",
            "duration": 30000,  # milliseconds
        }
        self._default_playlist_data = {
            "items": [
                {
                    "type": "image",
                    "url": f"/media/{DEFAULT_MEDIA_FILENAME}",
                    "duration": 5000,
                    "muted": False,
                }
            ]
        }

    def _send_json_response(self, data, status=200):
        self.server.send_response(status)
        self.server.send_header("Content-type", "application/json")
        self.server.send_header("Access-Control-Allow-Origin", "*")  # For CORS
        self.server.end_headers()
        self.server.wfile.write(json.dumps(data, ensure_ascii=False).encode("utf-8"))

    def _send_error_response(self, status, message):
        logger.error(f"Responding with error {status}: {message}")
        self._send_json_response({"error": message}, status)

    def get_schedule(self):
        """Return current school schedule"""
        schedule_data = load_json_file(
            DEFAULT_SCHEDULE_FILE, self._default_schedule_data
        )
        self._send_json_response(schedule_data)

    def get_current_media(self):
        """Return current media information"""
        media_data = load_json_file(
            CURRENT_MEDIA_FILE, self._default_current_media_data
        )
        self._send_json_response(media_data)

    def get_media_playlist(self):
        """Return current media playlist, auto-generating if needed"""
        try:
            # Try to load existing playlist
            playlist_data = load_json_file(PLAYLIST_FILE)

            # If no playlist exists or it's empty, generate one from media directory
            if not playlist_data or not playlist_data.get("items"):
                logger.info(
                    "No playlist found or playlist is empty, generating from media directory"
                )
                playlist_data = generate_playlist_from_media_dir()

            # Validate that all referenced media files still exist
            valid_items = []
            for item in playlist_data.get("items", []):
                url = item.get("url", "")
                if url.startswith("/media/"):
                    filename = url[len("/media/") :]
                    full_path = os.path.join(MEDIA_DIR, filename)
                    if os.path.exists(full_path):
                        valid_items.append(item)
                    else:
                        logger.warning(
                            f"Media file {filename} referenced in playlist but not found on disk"
                        )

            # If some items were removed, update the playlist
            if len(valid_items) != len(playlist_data.get("items", [])):
                if valid_items:
                    playlist_data = {"items": valid_items}
                    save_json_file(PLAYLIST_FILE, playlist_data)
                    logger.info(
                        f"Updated playlist, removed {len(playlist_data['items']) - len(valid_items)} invalid items"
                    )
                else:
                    # No valid items, regenerate playlist
                    logger.info(
                        "No valid items in playlist, regenerating from media directory"
                    )
                    playlist_data = generate_playlist_from_media_dir()

            self._send_json_response(playlist_data)

        except Exception as e:
            logger.exception("Error in get_media_playlist:")
            # Fallback to default playlist
            self._send_json_response(self._default_playlist_data)

    def regenerate_playlist(self):
        """Force regeneration of playlist from media directory"""
        try:
            playlist_data = generate_playlist_from_media_dir()
            self._send_json_response(
                {
                    "status": "success",
                    "message": f"Playlist regenerated with {len(playlist_data['items'])} items",
                    "playlist": playlist_data,
                }
            )
        except Exception as e:
            logger.exception("Error in regenerate_playlist:")
            self._send_error_response(500, f"Internal Server Error: {e}")


# --- HTTP Request Handler ---
class VideoTimelineHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        self.api_handlers = None
        super().__init__(*args, **kwargs)

    def setup(self):
        super().setup()
        self.api_handlers = APIHandlers(self)

    def do_GET(self):
        parsed_path = urllib.parse.urlparse(self.path)
        path = parsed_path.path

        if path == "/api/schedule":
            self.api_handlers.get_schedule()
        elif path == "/api/media/current":
            self.api_handlers.get_current_media()
        elif path == "/api/media/playlist":
            self.api_handlers.get_media_playlist()
        elif path == "/api/media/regenerate":
            self.api_handlers.regenerate_playlist()
        elif path.startswith("/media/"):
            self._handle_get_media_file(path)
        elif path == "/" or path == "/index.html":
            self._send_error_response(
                404, "No static frontend served from root. This is an API server."
            )
        else:
            self._send_error_response(404, "Not Found")

    def _send_error_response(self, status, message):
        """Helper to send JSON error responses."""
        self.api_handlers._send_error_response(status, message)

    def _handle_get_media_file(self, path):
        """Serve media files from the MEDIA_DIR."""
        # Remove /media/ prefix and get filename
        filename = path[len("/media/") :]
        # Prevent directory traversal attacks
        if ".." in filename or filename.startswith("/"):
            logger.warning(f"Attempted directory traversal: {path}")
            return self._send_error_response(403, "Forbidden")

        media_filepath = os.path.join(MEDIA_DIR, filename)
        if not os.path.exists(media_filepath):
            logger.info(f"Media file not found: {media_filepath}")
            return self._send_error_response(404, f"Media file '{filename}' not found")

        # Determine content type
        mime_types = {
            ".jpg": "image/jpeg",
            ".jpeg": "image/jpeg",
            ".png": "image/png",
            ".gif": "image/gif",
            ".bmp": "image/bmp",
            ".tiff": "image/tiff",
            ".mp4": "video/mp4",
            ".webm": "video/webm",
            ".ogg": "video/ogg",
            ".avi": "video/x-msvideo",
            ".mov": "video/quicktime",
        }
        ext = os.path.splitext(filename)[1].lower()
        content_type = mime_types.get(ext, "application/octet-stream")

        try:
            with open(media_filepath, "rb") as f:
                self.send_response(200)
                self.send_header("Content-type", content_type)
                self.send_header("Content-Length", str(os.fstat(f.fileno()).st_size))
                self.send_header("Access-Control-Allow-Origin", "*")
                self.end_headers()
                shutil.copyfileobj(f, self.wfile)
                logger.info(f"Served media file: {filename}")
        except IOError as e:
            logger.exception(f"Error serving media file {media_filepath}:")
            self._send_error_response(500, f"Error reading media file: {e}")


def ensure_directories():
    """Ensure required directories exist"""
    os.makedirs(DATA_DIR, exist_ok=True)
    os.makedirs(MEDIA_DIR, exist_ok=True)
    logger.info(f"Created directories: {DATA_DIR}, {MEDIA_DIR}")


def run_server(port=PORT):
    """Run the VideoTimeline server"""
    ensure_directories()

    logger.info(f"Starting VideoTimeline Server on port {port}")

    try:
        with socketserver.TCPServer(("", port), VideoTimelineHandler) as httpd:
            logger.info(f"VideoTimeline Server running on port {port}")
            logger.info(f"API endpoints available:")
            logger.info(f"  • GET http://localhost:{port}/api/schedule")
            logger.info(f"  • GET http://localhost:{port}/api/media/current")
            logger.info(f"  • GET http://localhost:{port}/api/media/playlist")
            logger.info(f"  • GET http://localhost:{port}/api/media/regenerate")
            logger.info(f"  • GET http://localhost:{port}/media/<filename>")
            logger.info("Press Ctrl+C to stop the server")

            try:
                httpd.serve_forever()
            except KeyboardInterrupt:
                logger.info("\nShutting down server...")
                httpd.shutdown()
                logger.info("Server stopped.")
    except OSError as e:
        if e.errno == 98:  # Address already in use
            logger.error(
                f"Port {port} is already in use. Try a different port or stop the existing server."
            )
        else:
            logger.error(f"Failed to start server: {e}")
        return False

    return True


if __name__ == "__main__":
    run_server()
