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

class VideoTimelineHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=os.path.dirname(__file__), **kwargs)
    
    def do_GET(self):
        parsed_path = urllib.parse.urlparse(self.path)
        
        if parsed_path.path == '/api/schedule':
            self.handle_get_schedule()
        elif parsed_path.path == '/api/media/current':
            self.handle_get_current_media()
        elif parsed_path.path.startswith('/media/'):
            self.handle_get_media_file(parsed_path.path)
        else:
            # Serve static files
            #super().do_GET()
            self.send_error(404, "Not Found")

    
    def do_POST(self):
        parsed_path = urllib.parse.urlparse(self.path)
        
        if parsed_path.path == '/api/schedule':
            self.handle_post_schedule()
        elif parsed_path.path == '/api/media/upload':
            self.handle_post_media()
        else:
            self.send_error(404, "Not Found")
    
    def handle_get_schedule(self):
        """Return current school schedule"""
        try:
            schedule_file = 'data/schedule.json'
            if os.path.exists(schedule_file):
                with open(schedule_file, 'r', encoding='utf-8') as f:
                    schedule_data = json.load(f)
            else:
                # Default Turkish schedule
                schedule_data = {
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
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(schedule_data, ensure_ascii=False).encode('utf-8'))
            
        except Exception as e:
            self.send_error(500, f"Internal Server Error: {e}")
    
    def handle_get_current_media(self):
        """Return current media information"""
        try:
            media_file = 'data/current_media.json'
            if os.path.exists(media_file):
                with open(media_file, 'r', encoding='utf-8') as f:
                    media_data = json.load(f)
            else:
                # Default: no media
                media_data = {
                    "type": "image",
                    "url": "/media/default.jpg",
                    "duration": 30000
                }
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(media_data).encode('utf-8'))
            
        except Exception as e:
            self.send_error(500, f"Internal Server Error: {e}")
    
    def handle_get_media_file(self, path):
        """Serve media files"""
        # Remove /media/ prefix and get filename
        filename = path[7:]  # Remove '/media/'
        media_path = os.path.join('media', filename)
        
        if os.path.exists(media_path) and os.path.isfile(media_path):
            # Determine content type
            if filename.lower().endswith(('.jpg', '.jpeg')):
                content_type = 'image/jpeg'
            elif filename.lower().endswith('.png'):
                content_type = 'image/png'
            elif filename.lower().endswith('.mp4'):
                content_type = 'video/mp4'
            elif filename.lower().endswith('.avi'):
                content_type = 'video/avi'
            else:
                content_type = 'application/octet-stream'
            
            # Serve the file
            with open(media_path, 'rb') as f:
                self.send_response(200)
                self.send_header('Content-type', content_type)
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                shutil.copyfileobj(f, self.wfile)
        else:
            self.send_error(404, "Media file not found")
    
    def handle_post_schedule(self):
        """Update schedule"""
        try:
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            schedule_data = json.loads(post_data.decode('utf-8'))
            
            # Save schedule
            os.makedirs('data', exist_ok=True)
            with open('data/schedule.json', 'w', encoding='utf-8') as f:
                json.dump(schedule_data, f, ensure_ascii=False, indent=2)
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({"status": "success"}).encode('utf-8'))
            
        except Exception as e:
            self.send_error(500, f"Internal Server Error: {e}")
    
    def handle_post_media(self):
        """Handle media upload"""
        try:
            # This is a simple implementation - in production you'd want proper multipart parsing
            self.send_response(501)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({"error": "Upload not implemented yet"}).encode('utf-8'))
            
        except Exception as e:
            self.send_error(500, f"Internal Server Error: {e}")

def run_server(port=8080):
    """Run the VideoTimeline server"""
    
    # Create necessary directories
    os.makedirs('data', exist_ok=True)
    os.makedirs('media', exist_ok=True)
    
    # Create a default image if it doesn't exist
    default_image_path = 'media/default.jpg'
    if not os.path.exists(default_image_path):
        # Create a simple placeholder file
        with open(default_image_path, 'w') as f:
            f.write("# Default placeholder image\n")
    
    with socketserver.TCPServer(("", port), VideoTimelineHandler) as httpd:
        print(f"VideoTimeline Server running on port {port}")
        print(f"API available at: http://0.0.0.0:{port}/api/")
        print("Press Ctrl+C to stop the server")
        
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down server...")
            httpd.shutdown()

if __name__ == "__main__":
    run_server()
