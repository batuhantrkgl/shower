#!/usr/bin/env python3
"""
Test script for playlist generation functionality
"""

import json
import os
import sys
import requests
import time

# Add the server directory to the path so we can import server modules
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from server import generate_playlist_from_media_dir, MEDIA_DIR, PLAYLIST_FILE


def test_playlist_generation():
    """Test the playlist generation from media directory"""
    print("Testing playlist generation...")

    # Show current media files
    print(f"\nMedia directory: {MEDIA_DIR}")
    if os.path.exists(MEDIA_DIR):
        media_files = [
            f
            for f in os.listdir(MEDIA_DIR)
            if os.path.isfile(os.path.join(MEDIA_DIR, f))
        ]
        print(f"Found {len(media_files)} media files:")
        for f in sorted(media_files):
            print(f"  - {f}")
    else:
        print("Media directory does not exist!")
        return False

    # Generate playlist
    try:
        playlist_data = generate_playlist_from_media_dir()
        print(f"\nGenerated playlist with {len(playlist_data['items'])} items:")

        for i, item in enumerate(playlist_data["items"], 1):
            print(f"  {i}. {item['type']}: {item['url']}")
            print(f"     Duration: {item['duration']}ms, Muted: {item['muted']}")

        # Save to file for inspection
        with open("test_playlist_output.json", "w") as f:
            json.dump(playlist_data, f, indent=2)
        print(f"\nPlaylist saved to test_playlist_output.json")

        return True

    except Exception as e:
        print(f"Error generating playlist: {e}")
        return False


def test_server_api():
    """Test the server API endpoints"""
    server_url = "http://localhost:8080"

    print(f"\nTesting server API at {server_url}...")

    try:
        # Test playlist endpoint
        response = requests.get(f"{server_url}/api/media/playlist", timeout=5)
        if response.status_code == 200:
            playlist_data = response.json()
            print(f"✓ GET /api/media/playlist successful")
            print(f"  Returned {len(playlist_data.get('items', []))} playlist items")

            for i, item in enumerate(playlist_data.get("items", []), 1):
                print(
                    f"    {i}. {item.get('type', 'unknown')}: {item.get('url', 'no url')}"
                )
                print(
                    f"       Duration: {item.get('duration', 'unknown')}ms, Muted: {item.get('muted', 'unknown')}"
                )
        else:
            print(f"✗ GET /api/media/playlist failed: {response.status_code}")
            return False

        # Test regenerate endpoint
        response = requests.get(f"{server_url}/api/media/regenerate", timeout=5)
        if response.status_code == 200:
            result = response.json()
            print(f"✓ GET /api/media/regenerate successful")
            print(f"  Message: {result.get('message', 'no message')}")
        else:
            print(f"✗ GET /api/media/regenerate failed: {response.status_code}")

        return True

    except requests.exceptions.ConnectionError:
        print("✗ Could not connect to server. Make sure the server is running.")
        return False
    except Exception as e:
        print(f"✗ Error testing server API: {e}")
        return False


def show_expected_playlist():
    """Show what the playlist should look like based on current media files"""
    print("\nExpected playlist behavior based on filename patterns:")

    patterns = {
        "welcome_quick.jpg": "Image, 2 seconds (quick)",
        "announcement_sound.mp4": "Video, full duration, unmuted (sound)",
        "background_mute.mp4": "Video, full duration, muted (background + mute)",
        "schedule_long.png": "Image, 10 seconds (schedule + long)",
        "default.jpg": "Image, 5 seconds (default)",
    }

    for filename, description in patterns.items():
        filepath = os.path.join(MEDIA_DIR, filename)
        exists = "✓" if os.path.exists(filepath) else "✗"
        print(f"  {exists} {filename}: {description}")


def main():
    print("Playlist Generation Test")
    print("=" * 50)

    # Show expected behavior
    show_expected_playlist()

    # Test playlist generation
    if test_playlist_generation():
        print("\n✓ Playlist generation test passed")
    else:
        print("\n✗ Playlist generation test failed")
        return 1

    # Ask if user wants to test server
    try:
        test_server = input("\nTest server API? (y/n): ").lower().startswith("y")
    except KeyboardInterrupt:
        print("\nTest interrupted")
        return 0

    if test_server:
        if test_server_api():
            print("\n✓ Server API test passed")
        else:
            print("\n✗ Server API test failed")
            print("Make sure to start the server first: python server.py")

    print("\nTest completed!")
    return 0


if __name__ == "__main__":
    sys.exit(main())
