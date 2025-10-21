#!/usr/bin/env python3
"""
VideoTimeline Server
Simple HTTP server to serve schedule data and media files
"""

import json
import os
import http.server
import socketserver
from datetime import datetime, time
import urllib.parse
import shutil
import logging
import subprocess
import sys
import netifaces # You need to install this: sudo pip install netifaces
from PIL import Image, ImageDraw, ImageFont # For generating a default image

# --- Configuration ---
PORT = 8080
DATA_DIR = 'data'
MEDIA_DIR = 'media'
DEFAULT_SCHEDULE_FILE = os.path.join(DATA_DIR, 'schedule.json')
CURRENT_MEDIA_FILE = os.path.join(DATA_DIR, 'current_media.json')
PLAYLIST_FILE = os.path.join(DATA_DIR, 'playlist.json')
DEFAULT_MEDIA_FILENAME = 'default.jpg'
DEFAULT_MEDIA_PATH = os.path.join(MEDIA_DIR, DEFAULT_MEDIA_FILENAME)

# --- Network Configuration ---
RESERVE_IP = '192.168.32.52' # The IP address you want to reserve
RESERVE_IP_SUBNET = '24'     # The subnet in CIDR notation (24 is 255.255.255.0)

# --- Logging Setup ---
logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# --- Helper Functions ---
def load_json_file(filepath, default_data=None):
    """Loads a JSON file, returning default_data if not found or invalid."""
    if os.path.exists(filepath):
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
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
        with open(filepath, 'w', encoding='utf-8') as f:
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
    image_extensions = {'.jpg', '.jpeg', '.png', '.gif', '.bmp', '.webp'}
    video_extensions = {'.mp4', '.avi', '.mov', '.wmv', '.flv', '.webm', '.mkv'}

    # Get all media files and sort them alphabetically
    media_files = []
    for filename in os.listdir(MEDIA_DIR):
        if filename.startswith('.'):  # Skip hidden files
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
            if 'quick' in filename.lower() or 'short' in filename.lower():
                duration = 2000  # 2 seconds
            elif 'long' in filename.lower() or 'schedule' in filename.lower():
                duration = 10000  # 10 seconds
            elif 'banner' in filename.lower() or 'logo' in filename.lower():
                duration = 3000  # 3 seconds

            playlist_items.append({
                "type": "image",
                "url": f"/media/{filename}",
                "duration": duration,
                "muted": False  # Not applicable for images, but keep for consistency
            })

        elif file_ext in video_extensions:
            # Video defaults
            muted = False  # Default to unmuted

            # Smart mute detection based on filename
            if 'mute' in filename.lower() or 'silent' in filename.lower() or 'background' in filename.lower():
                muted = True
            elif 'sound' in filename.lower() or 'audio' in filename.lower() or 'announcement' in filename.lower():
                muted = False

            playlist_items.append({
                "type": "video",
                "url": f"/media/{filename}",
                "duration": -1,  # Full video duration
                "muted": muted
            })

    # If no media files found, create a default entry
    if not playlist_items:
        # Ensure default image exists
        if not os.path.exists(DEFAULT_MEDIA_PATH):
            generate_default_image(DEFAULT_MEDIA_PATH)

        playlist_items.append({
            "type": "image",
            "url": f"/media/{DEFAULT_MEDIA_FILENAME}",
            "duration": 5000,
            "muted": False
        })

    playlist_data = {"items": playlist_items}

    # Save the generated playlist
    save_json_file(PLAYLIST_FILE, playlist_data)

    logger.info(f"Generated playlist with {len(playlist_items)} items")
    return playlist_data

def generate_default_image(output_path):
    """Generates a simple default placeholder image."""
    try:
        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        img = Image.new('RGB', (640, 480), color = (73, 109, 137))
        d = ImageDraw.Draw(img)
        try:
            # Try to use a common font or fall back
            font = ImageFont.truetype("arial.ttf", 40)
        except IOError:
            font = ImageFont.load_default()
            logger.warning("Could not find arial.ttf, using default font for placeholder image.")
        d.text((50,200), "No Media Available", fill=(255,255,255), font=font)
        img.save(output_path)
        logger.info(f"Generated default placeholder image at {output_path}")
    except Exception as e:
        logger.error(f"Failed to generate default image at {output_path}: {e}")


# --- Network Management Functions ---
def check_and_reserve_ip(ip_to_reserve, subnet):
    """Checks if an IP is in use and assigns it as a secondary address if not."""
    logger.info(f"Attempting to reserve IP address: {ip_to_reserve}")

    # Step 1: Check if this machine already has the IP address
    for interface in netifaces.interfaces():
        try:
            addrs = netifaces.ifaddresses(interface)
            if netifaces.AF_INET in addrs:
                for addr_info in addrs[netifaces.AF_INET]:
                    if addr_info.get('addr') == ip_to_reserve:
                        logger.info(f"This device already has the IP {ip_to_reserve} on interface {interface}. No action needed.")
                        return True
        except Exception as e:
            logger.warning(f"Could not inspect interface {interface}: {e}")

    # Step 2: Check if another device on the network is using the IP
    logger.info(f"Pinging {ip_to_reserve} to see if it's in use by another device...")
    try:
        # -c 1: send 1 packet, -W 1: wait 1 second for a reply
        result = subprocess.run(['ping', '-c', '1', '-W', '1', ip_to_reserve], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if result.returncode == 0:
            logger.error(f"IP address {ip_to_reserve} is already in use by another device on the network. Cannot reserve.")
            return False
    except FileNotFoundError:
        logger.error("The 'ping' command was not found. Cannot check if IP is in use.")
        return False

    logger.info(f"IP {ip_to_reserve} appears to be free.")

    # Step 3: Add the IP address to the primary network interface
    if os.geteuid() != 0:
        logger.warning(f"This script is not running as root (sudo). Cannot add IP address {ip_to_reserve}.")
        logger.warning("Please run with 'sudo python3 server.py' to enable IP reservation.")
        return False

    try:
        # Find the default gateway interface (e.g., 'eth0' or 'wlan0')
        gws = netifaces.gateways()
        if 'default' not in gws or netifaces.AF_INET not in gws['default']:
             logger.error("Could not determine the default network interface. Cannot add IP.")
             return False
        interface = gws['default'][netifaces.AF_INET][1]

        logger.info(f"Adding IP {ip_to_reserve} to interface {interface}...")
        ip_with_subnet = f"{ip_to_reserve}/{subnet}"
        # Using the 'ip' command to add the address
        subprocess.run(['ip', 'addr', 'add', ip_with_subnet, 'dev', interface], check=True)
        logger.info(f"Successfully added IP address {ip_to_reserve} to {interface}.")
        return True
    except FileNotFoundError:
        logger.error("The 'ip' command was not found. Cannot add IP address.")
        return False
    except subprocess.CalledProcessError:
        logger.error(f"Failed to add IP address {ip_to_reserve}. This may require 'sudo' permissions.")
        return False
    except Exception as e:
        logger.error(f"An unexpected error occurred while adding IP: {e}")
        return False


# --- API Endpoints Handlers ---
class APIHandlers:
    """A collection of methods to handle specific API endpoints."""
    # ... (This class remains unchanged) ...
    def __init__(self, server_instance):
        self.server = server_instance
        self._default_schedule_data = {
            "school_start": "08:50",
            "school_end": "15:55",
            "blocks": [
                {"start_time": "08:50", "end_time": "09:30", "name": "Ders 1", "type": "lesson"},
                {"start_time": "09:30", "end_time": "09:40", "name": "Teneffüs", "type": "break"},
                {"start_time": "09:40", "end_time": "10:20", "name": "Ders 2", "type": "lesson"},
                {"start_time": "10:20", "end_time": "10:30", "name": "Teneffüs", "type": "break"},
                {"start_time": "10:30", "end_time": "11:10", "name": "Ders 3", "type": "lesson"},
                {"start_time": "11:10", "end_time": "11:20", "name": "Teneffüs", "type": "break"},
                {"start_time": "11:20", "end_time": "12:00", "name": "Ders 4", "type": "lesson"},
                {"start_time": "12:00", "end_time": "12:45", "name": "Öğle Arası", "type": "lunch"},
                {"start_time": "12:45", "end_time": "13:25", "name": "Ders 5", "type": "lesson"},
                {"start_time": "13:25", "end_time": "13:35", "name": "Teneffüs", "type": "break"},
                {"start_time": "13:35", "end_time": "14:15", "name": "Ders 6", "type": "lesson"},
                {"start_time": "14:15", "end_time": "14:25", "name": "Teneffüs", "type": "break"},
                {"start_time": "14:25", "end_time": "15:05", "name": "Ders 7", "type": "lesson"},
                {"start_time": "15:05", "end_time": "15:15", "name": "Teneffüs", "type": "break"},
                {"start_time": "15:15", "end_time": "15:55", "name": "Ders 8", "type": "lesson"}
            ]
        }
        self._default_current_media_data = {
            "type": "image",
            "url": f"/media/{DEFAULT_MEDIA_FILENAME}",
            "duration": 30000 # milliseconds
        }
        self._default_playlist_data = {
            "items": [
                {
                    "type": "image",
                    "url": f"/media/{DEFAULT_MEDIA_FILENAME}",
                    "duration": 5000,
                    "muted": False
                }
            ]
        }

    def _send_json_response(self, data, status=200):
        self.server.send_response(status)
        self.server.send_header('Content-type', 'application/json')
        self.server.send_header('Access-Control-Allow-Origin', '*') # For CORS
        self.server.end_headers()
        self.server.wfile.write(json.dumps(data, ensure_ascii=False).encode('utf-8'))

    def _send_error_response(self, status, message):
        logger.error(f"Responding with error {status}: {message}")
        self._send_json_response({"error": message}, status)

    def get_schedule(self):
        """Return current school schedule"""
        schedule_data = load_json_file(DEFAULT_SCHEDULE_FILE, self._default_schedule_data)
        self._send_json_response(schedule_data)

    def post_schedule(self):
        """Update schedule"""
        try:
            content_length = int(self.server.headers.get('Content-Length', 0))
            if content_length == 0:
                return self._send_error_response(400, "Request body is empty.")

            post_data_raw = self.server.rfile.read(content_length)
            schedule_data = json.loads(post_data_raw.decode('utf-8'))

            if save_json_file(DEFAULT_SCHEDULE_FILE, schedule_data):
                self._send_json_response({"status": "success", "message": "Schedule updated."})
            else:
                self._send_error_response(500, "Failed to save schedule data.")

        except json.JSONDecodeError:
            self._send_error_response(400, "Invalid JSON format.")
        except Exception as e:
            logger.exception("Error in post_schedule:")
            self._send_error_response(500, f"Internal Server Error: {e}")

    def get_current_media(self):
        """Return current media information"""
        media_data = load_json_file(CURRENT_MEDIA_FILE, self._default_current_media_data)
        # Ensure default media exists if it's referenced
        if media_data.get("url") == f"/media/{DEFAULT_MEDIA_FILENAME}" and not os.path.exists(DEFAULT_MEDIA_PATH):
            generate_default_image(DEFAULT_MEDIA_PATH)
        self._send_json_response(media_data)

    def post_current_media(self):
        """Update current media (e.g., which file to play next)"""
        try:
            content_length = int(self.server.headers.get('Content-Length', 0))
            if content_length == 0:
                return self._send_error_response(400, "Request body is empty.")

            post_data_raw = self.server.rfile.read(content_length)
            media_config = json.loads(post_data_raw.decode('utf-8'))

            # Basic validation: ensure 'url' is provided and points to an existing file
            media_url = media_config.get('url')
            if not media_url or not media_url.startswith('/media/'):
                return self._send_error_response(400, "Invalid media URL format. Must start with '/media/'.")

            # Check if the media file actually exists on the server
            filename = media_url[len('/media/'):]
            full_media_path = os.path.join(MEDIA_DIR, filename)
            if not os.path.exists(full_media_path):
                return self._send_error_response(404, f"Referenced media file '{filename}' not found on server.")

            if save_json_file(CURRENT_MEDIA_FILE, media_config):
                self._send_json_response({"status": "success", "message": "Current media updated."})
            else:
                self._send_error_response(500, "Failed to save current media data.")

        except json.JSONDecodeError:
            self._send_error_response(400, "Invalid JSON format.")
        except Exception as e:
            logger.exception("Error in post_current_media:")
            self._send_error_response(500, f"Internal Server Error: {e}")

        def get_media_playlist(self):
            """Return current media playlist, auto-generating if needed"""
            try:
                # Try to load existing playlist
                playlist_data = load_json_file(PLAYLIST_FILE)

                # If no playlist exists or it's empty, generate one from media directory
                if not playlist_data or not playlist_data.get('items'):
                    logger.info("No playlist found or playlist is empty, generating from media directory")
                    playlist_data = generate_playlist_from_media_dir()

                # Validate that all referenced media files still exist
                valid_items = []
                for item in playlist_data.get('items', []):
                    url = item.get('url', '')
                    if url.startswith('/media/'):
                        filename = url[len('/media/'):]
                        full_path = os.path.join(MEDIA_DIR, filename)
                        if os.path.exists(full_path):
                            valid_items.append(item)
                        else:
                            logger.warning(f"Media file {filename} referenced in playlist but not found on disk")

                # If some items were removed, update the playlist
                if len(valid_items) != len(playlist_data.get('items', [])):
                    if valid_items:
                        playlist_data = {"items": valid_items}
                        save_json_file(PLAYLIST_FILE, playlist_data)
                        logger.info(f"Updated playlist, removed {len(playlist_data['items']) - len(valid_items)} invalid items")
                    else:
                        # No valid items, regenerate playlist
                        logger.info("No valid items in playlist, regenerating from media directory")
                        playlist_data = generate_playlist_from_media_dir()

                self._send_json_response(playlist_data)

            except Exception as e:
                logger.exception("Error in get_media_playlist:")
                # Fallback to default playlist
                self._send_json_response(self._default_playlist_data)

        def post_media_playlist(self):
            """Update the media playlist"""
            try:
                content_length = int(self.server.headers.get('Content-Length', 0))
                if content_length == 0:
                    return self._send_error_response(400, "Request body is empty.")

                post_data_raw = self.server.rfile.read(content_length)
                playlist_data = json.loads(post_data_raw.decode('utf-8'))

                # Basic validation
                if not isinstance(playlist_data, dict) or 'items' not in playlist_data:
                    return self._send_error_response(400, "Invalid playlist format. Must have 'items' array.")

                items = playlist_data['items']
                if not isinstance(items, list):
                    return self._send_error_response(400, "Playlist 'items' must be an array.")

                # Validate each item
                for i, item in enumerate(items):
                    if not isinstance(item, dict):
                        return self._send_error_response(400, f"Item {i} must be an object.")

                    item_type = item.get('type')
                    if item_type not in ['image', 'video']:
                        return self._send_error_response(400, f"Item {i} has invalid type '{item_type}'. Must be 'image' or 'video'.")

                    url = item.get('url')
                    if not url or not isinstance(url, str):
                        return self._send_error_response(400, f"Item {i} must have a valid 'url' string.")

                    # Check if media file exists (for local URLs)
                    if url.startswith('/media/'):
                        filename = url[len('/media/'):]
                        full_path = os.path.join(MEDIA_DIR, filename)
                        if not os.path.exists(full_path):
                            return self._send_error_response(404, f"Item {i}: Media file '{filename}' not found.")

                if save_json_file(PLAYLIST_FILE, playlist_data):
                    self._send_json_response({"status": "success", "message": "Playlist updated."})
                else:
                    self._send_error_response(500, "Failed to save playlist data.")

            except json.JSONDecodeError:
                self._send_error_response(400, "Invalid JSON format.")
            except Exception as e:
                logger.exception("Error in post_media_playlist:")
                self._send_error_response(500, f"Internal Server Error: {e}")

        def regenerate_playlist(self):
            """Force regeneration of playlist from media directory"""
            try:
                playlist_data = generate_playlist_from_media_dir()
                self._send_json_response({
                    "status": "success",
                    "message": f"Playlist regenerated with {len(playlist_data['items'])} items",
                    "playlist": playlist_data
                })
            except Exception as e:
                logger.exception("Error in regenerate_playlist:")
                self._send_error_response(500, f"Internal Server Error: {e}")

        def post_media_upload(self):
        """Handle media upload (Multipart form data would be needed for a real solution)"""
        # A more complete implementation would use `cgi.FieldStorage` or a similar library
        # to parse multipart/form-data. For now, we'll keep it simple for demonstration.
        self._send_error_response(501, "Media upload not fully implemented (requires multipart form data parsing).")
        logger.warning("Attempted media upload, but it's not fully implemented.")

# --- HTTP Request Handler ---
class VideoTimelineHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        # ***** THE FIX IS HERE *****
        # Initialize our custom attributes FIRST.
        self.api_handlers = APIHandlers(self)
        # NOW, call the parent constructor which will start handling the request.
        super().__init__(*args, **kwargs)
        # ***** END OF FIX *****

    def do_GET(self):
        parsed_path = urllib.parse.urlparse(self.path)
        path = parsed_path.path

        if path == '/api/schedule':
            self.api_handlers.get_schedule()
        elif path == '/api/media/current':
            self.api_handlers.get_current_media()
        elif path == '/api/media/playlist':
            self.api_handlers.get_media_playlist()
        elif path == '/api/media/regenerate':
            self.api_handlers.regenerate_playlist()
        elif path.startswith('/media/'):
            self._handle_get_media_file(path)
        # Added a root redirect for convenience
        elif path == '/' or path == '/index.html':
            self._send_error_response(404, "No static frontend served from root. This is an API server.")
        else:
            self._send_error_response(404, "Not Found")

    def do_POST(self):
        parsed_path = urllib.parse.urlparse(self.path)
        path = parsed_path.path

        if path == '/api/schedule':
            self.api_handlers.post_schedule()
        elif path == '/api/media/current':
            self.api_handlers.post_current_media()
        elif path == '/api/media/playlist':
            self.api_handlers.post_media_playlist()
        elif path == '/api/media/upload':
            self.api_handlers.post_media_upload()
        else:
            self._send_error_response(404, "Not Found")

    def _send_error_response(self, status, message):
        """Helper to send JSON error responses."""
        self.api_handlers._send_error_response(status, message) # Access through api_handlers

    def _handle_get_media_file(self, path):
        """Serve media files from the MEDIA_DIR."""
        # Remove /media/ prefix and get filename
        filename = path[len('/media/'):]
        # Prevent directory traversal attacks
        if '..' in filename or filename.startswith('/'):
            logger.warning(f"Attempted directory traversal: {path}")
            return self._send_error_response(403, "Forbidden")

        media_filepath = os.path.join(MEDIA_DIR, filename)

        if not os.path.exists(media_filepath) or not os.path.isfile(media_filepath):
            logger.warning(f"Media file not found: {media_filepath}")
            return self._send_error_response(404, "Media file not found")

        # Determine content type more robustly
        mime_types = {
            '.jpg': 'image/jpeg', '.jpeg': 'image/jpeg', '.png': 'image/png',
            '.gif': 'image/gif', '.bmp': 'image/bmp', '.tiff': 'image/tiff',
            '.mp4': 'video/mp4', '.webm': 'video/webm', '.ogg': 'video/ogg',
            '.avi': 'video/x-msvideo', '.mov': 'video/quicktime'
        }
        ext = os.path.splitext(filename)[1].lower()
        content_type = mime_types.get(ext, 'application/octet-stream')

        try:
            with open(media_filepath, 'rb') as f:
                self.send_response(200)
                self.send_header('Content-type', content_type)
                self.send_header('Content-Length', str(os.fstat(f.fileno()).st_size))
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                shutil.copyfileobj(f, self.wfile)
                logger.info(f"Served media file: {filename}")
        except IOError as e:
            logger.exception(f"Error serving media file {media_filepath}:")
            self._send_error_response(500, f"Error reading media file: {e}")

def run_server(port=PORT):
    """Run the VideoTimeline server"""
    # --- Execute Startup Tasks ---
    os.makedirs(DATA_DIR, exist_ok=True)
    os.makedirs(MEDIA_DIR, exist_ok=True)

    if not os.path.exists(DEFAULT_MEDIA_PATH):
        generate_default_image(DEFAULT_MEDIA_PATH)

    load_json_file(DEFAULT_SCHEDULE_FILE, APIHandlers(None)._default_schedule_data)
    load_json_file(CURRENT_MEDIA_FILE, APIHandlers(None)._default_current_media_data)

    # --- NEW: IP Reservation Logic ---
    if RESERVE_IP:
        check_and_reserve_ip(RESERVE_IP, RESERVE_IP_SUBNET)
    # --- End of New Logic ---

    Handler = VideoTimelineHandler
    with socketserver.TCPServer(("0.0.0.0", port), Handler) as httpd:
        logger.info(f"VideoTimeline Server running on port {port}")
        logger.info(f"API available at: http://<your_ip>:{port}/api/schedule")
        if RESERVE_IP:
            logger.info(f"Server should also be accessible at: http://{RESERVE_IP}:{port}")
        logger.info("Press Ctrl+C to stop the server")

        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            logger.info("\nShutting down server...")
            httpd.shutdown()
            logger.info("Server stopped.")

if __name__ == "__main__":
    # Ensure dependencies are installed:
    # pip install Pillow netifaces
    run_server()
