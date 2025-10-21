#!/usr/bin/env python3
"""
Simple test script for playlist generation functionality
No external dependencies required
"""

import json
import os
import sys

# Add the server directory to the path so we can import server modules
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))


def test_playlist_generation():
    """Test the playlist generation from media directory"""
    print("Testing playlist generation...")

    # Import after adding to path
    try:
        from server import generate_playlist_from_media_dir, MEDIA_DIR, PLAYLIST_FILE
    except ImportError as e:
        print(f"Error importing server module: {e}")
        return False

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
            file_path = os.path.join(MEDIA_DIR, f)
            file_size = os.path.getsize(file_path)
            print(f"  - {f} ({file_size} bytes)")
    else:
        print("Media directory does not exist!")
        return False

    # Generate playlist
    try:
        playlist_data = generate_playlist_from_media_dir()
        print(f"\nGenerated playlist with {len(playlist_data['items'])} items:")

        for i, item in enumerate(playlist_data["items"], 1):
            print(f"  {i}. Type: {item['type']}")
            print(f"     URL: {item['url']}")
            print(f"     Duration: {item['duration']}ms")
            print(f"     Muted: {item['muted']}")
            print()

        # Save to file for inspection
        output_file = "test_playlist_output.json"
        with open(output_file, "w") as f:
            json.dump(playlist_data, f, indent=2)
        print(f"Playlist saved to {output_file}")

        # Show the JSON content
        print("\nGenerated JSON:")
        print("-" * 40)
        print(json.dumps(playlist_data, indent=2))
        print("-" * 40)

        return True

    except Exception as e:
        print(f"Error generating playlist: {e}")
        import traceback

        traceback.print_exc()
        return False


def show_filename_patterns():
    """Show how filenames affect playlist generation"""
    print("\nFilename Pattern Detection:")
    print("=" * 50)

    patterns = {
        "Images": {
            "default": "5 seconds",
            "quick/short": "2 seconds",
            "long/schedule": "10 seconds",
            "banner/logo": "3 seconds",
        },
        "Videos": {
            "default": "unmuted",
            "mute/silent/background": "muted",
            "sound/audio/announcement": "unmuted",
        },
    }

    for media_type, pattern_dict in patterns.items():
        print(f"\n{media_type}:")
        for pattern, result in pattern_dict.items():
            print(f"  - Contains '{pattern}' → {result}")


def analyze_current_files():
    """Analyze current media files and predict playlist"""
    print("\nCurrent Media File Analysis:")
    print("=" * 50)

    try:
        from server import MEDIA_DIR
    except ImportError:
        print("Cannot import server module")
        return

    if not os.path.exists(MEDIA_DIR):
        print("Media directory does not exist")
        return

    image_extensions = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp"}
    video_extensions = {".mp4", ".avi", ".mov", ".wmv", ".flv", ".webm", ".mkv"}

    media_files = []
    for filename in os.listdir(MEDIA_DIR):
        if filename.startswith("."):
            continue
        filepath = os.path.join(MEDIA_DIR, filename)
        if os.path.isfile(filepath):
            media_files.append(filename)

    media_files.sort()

    if not media_files:
        print("No media files found")
        return

    print(f"Found {len(media_files)} files:")

    for filename in media_files:
        file_ext = os.path.splitext(filename)[1].lower()

        if file_ext in image_extensions:
            # Predict duration
            duration = 5000  # default
            if "quick" in filename.lower() or "short" in filename.lower():
                duration = 2000
            elif "long" in filename.lower() or "schedule" in filename.lower():
                duration = 10000
            elif "banner" in filename.lower() or "logo" in filename.lower():
                duration = 3000

            print(f"  📷 {filename}")
            print(f"     → Image, {duration}ms duration")

        elif file_ext in video_extensions:
            # Predict mute setting
            muted = False
            if (
                "mute" in filename.lower()
                or "silent" in filename.lower()
                or "background" in filename.lower()
            ):
                muted = True

            print(f"  🎬 {filename}")
            print(f"     → Video, {'muted' if muted else 'with sound'}")

        else:
            print(f"  ❓ {filename}")
            print(f"     → Unknown type (extension: {file_ext})")


def main():
    print("Simple Playlist Generation Test")
    print("=" * 50)

    # Show pattern explanations
    show_filename_patterns()

    # Analyze current files
    analyze_current_files()

    # Test playlist generation
    print("\n" + "=" * 50)
    if test_playlist_generation():
        print("\n✅ Playlist generation test PASSED")
        print("\nTo use this playlist:")
        print("1. Start the server: python server.py")
        print("2. Test the endpoint: GET http://localhost:8080/api/media/playlist")
        print("3. The client will automatically fetch and play the playlist")
    else:
        print("\n❌ Playlist generation test FAILED")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
