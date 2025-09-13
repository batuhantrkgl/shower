#!/usr/bin/env python3
"""
Setup script for the VideoTimeline Server.

This script creates the necessary directories and default data files
so that the server can start without loading from its internal defaults.
"""

import os
import json

# --- Configuration (should match server.py) ---
DATA_DIR = 'data'
MEDIA_DIR = 'media'
DEFAULT_SCHEDULE_FILE = os.path.join(DATA_DIR, 'schedule.json')
CURRENT_MEDIA_FILE = os.path.join(DATA_DIR, 'current_media.json')
DEFAULT_MEDIA_FILENAME = 'default.jpg'

def create_default_files():
    """Creates directories and default JSON files."""
    print("--- VideoTimeline Server Setup ---")

    # 1. Create necessary directories
    try:
        print(f"Ensuring directory '{DATA_DIR}' exists...")
        os.makedirs(DATA_DIR, exist_ok=True)
        print(f"Ensuring directory '{MEDIA_DIR}' exists...")
        os.makedirs(MEDIA_DIR, exist_ok=True)
    except OSError as e:
        print(f"Error creating directories: {e}")
        return

    # 2. Define the default schedule data
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

    # 3. Define the default current media data
    current_media_data = {
        "type": "image",
        "url": f"/media/{DEFAULT_MEDIA_FILENAME}",
        "duration": 30000  # milliseconds
    }

    # 4. Write schedule.json if it doesn't exist
    if not os.path.exists(DEFAULT_SCHEDULE_FILE):
        print(f"Creating default schedule file at '{DEFAULT_SCHEDULE_FILE}'...")
        try:
            with open(DEFAULT_SCHEDULE_FILE, 'w', encoding='utf-8') as f:
                json.dump(schedule_data, f, ensure_ascii=False, indent=2)
            print(" -> Success.")
        except IOError as e:
            print(f" -> Error writing file: {e}")
    else:
        print(f"File '{DEFAULT_SCHEDULE_FILE}' already exists. Skipping.")

    # 5. Write current_media.json if it doesn't exist
    if not os.path.exists(CURRENT_MEDIA_FILE):
        print(f"Creating default current media file at '{CURRENT_MEDIA_FILE}'...")
        try:
            with open(CURRENT_MEDIA_FILE, 'w', encoding='utf-8') as f:
                json.dump(current_media_data, f, ensure_ascii=False, indent=2)
            print(" -> Success.")
        except IOError as e:
            print(f" -> Error writing file: {e}")
    else:
        print(f"File '{CURRENT_MEDIA_FILE}' already exists. Skipping.")
    
    print("\nSetup complete. You can now run the server.")

if __name__ == "__main__":
    create_default_files()
