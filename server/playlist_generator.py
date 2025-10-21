#!/usr/bin/env python3
"""
Standalone playlist generator for VideoTimeline Server
This module can generate playlists from media directories without external dependencies
"""

import json
import os
import logging

# Setup logging
logging.basicConfig(
    level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)

# Configuration
DATA_DIR = "data"
MEDIA_DIR = "media"
PLAYLIST_FILE = os.path.join(DATA_DIR, "playlist.json")
DEFAULT_MEDIA_FILENAME = "default.jpg"


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


def generate_playlist_from_media_dir(media_dir=None, output_file=None):
    """
    Automatically generate a playlist by scanning the media directory.
    Returns a playlist structure with smart defaults for durations and mute settings.
    """
    if media_dir is None:
        media_dir = MEDIA_DIR
    if output_file is None:
        output_file = PLAYLIST_FILE

    playlist_items = []

    if not os.path.exists(media_dir):
        logger.warning(f"Media directory {media_dir} does not exist")
        return {"items": []}

    # Supported media file extensions
    image_extensions = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp"}
    video_extensions = {".mp4", ".avi", ".mov", ".wmv", ".flv", ".webm", ".mkv"}

    # Get all media files and sort them alphabetically
    media_files = []
    for filename in os.listdir(media_dir):
        if filename.startswith("."):  # Skip hidden files
            continue
        filepath = os.path.join(media_dir, filename)
        if os.path.isfile(filepath):
            media_files.append(filename)

    media_files.sort()  # Sort alphabetically for consistent ordering

    logger.info(f"Found {len(media_files)} files in {media_dir}")

    for filename in media_files:
        file_ext = os.path.splitext(filename)[1].lower()

        if file_ext in image_extensions:
            # Image defaults
            duration = 5000  # 5 seconds default for images

            # Smart duration based on filename keywords
            filename_lower = filename.lower()
            if "quick" in filename_lower or "short" in filename_lower:
                duration = 2000  # 2 seconds
            elif "long" in filename_lower or "schedule" in filename_lower:
                duration = 10000  # 10 seconds
            elif "banner" in filename_lower or "logo" in filename_lower:
                duration = 3000  # 3 seconds

            playlist_items.append(
                {
                    "type": "image",
                    "url": f"/media/{filename}",
                    "duration": duration,
                    "muted": False,  # Not applicable for images, but keep for consistency
                }
            )

            logger.info(f"Added image: {filename} ({duration}ms)")

        elif file_ext in video_extensions:
            # Video defaults
            muted = False  # Default to unmuted

            # Smart mute detection based on filename keywords
            filename_lower = filename.lower()
            if (
                "mute" in filename_lower
                or "silent" in filename_lower
                or "background" in filename_lower
            ):
                muted = True
            elif (
                "sound" in filename_lower
                or "audio" in filename_lower
                or "announcement" in filename_lower
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

            logger.info(
                f"Added video: {filename} ({'muted' if muted else 'with sound'})"
            )

        else:
            logger.warning(f"Unknown file type, skipping: {filename}")

    # If no media files found, create a default entry
    if not playlist_items:
        logger.info("No media files found, creating default playlist")
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
    if save_json_file(output_file, playlist_data):
        logger.info(f"Playlist saved to {output_file}")
    else:
        logger.error(f"Failed to save playlist to {output_file}")

    logger.info(f"Generated playlist with {len(playlist_items)} items")
    return playlist_data


def analyze_media_files(media_dir=None):
    """
    Analyze media files and show what the playlist would look like
    """
    if media_dir is None:
        media_dir = MEDIA_DIR

    if not os.path.exists(media_dir):
        print(f"Media directory {media_dir} does not exist")
        return

    image_extensions = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp"}
    video_extensions = {".mp4", ".avi", ".mov", ".wmv", ".flv", ".webm", ".mkv"}

    media_files = []
    for filename in os.listdir(media_dir):
        if filename.startswith("."):
            continue
        filepath = os.path.join(media_dir, filename)
        if os.path.isfile(filepath):
            media_files.append(filename)

    media_files.sort()

    if not media_files:
        print("No media files found")
        return

    print(f"\nAnalyzing {len(media_files)} files in {media_dir}:")
    print("-" * 60)

    for filename in media_files:
        file_ext = os.path.splitext(filename)[1].lower()
        file_size = os.path.getsize(os.path.join(media_dir, filename))

        if file_ext in image_extensions:
            # Predict duration
            duration = 5000  # default
            duration_reason = "default"

            filename_lower = filename.lower()
            if "quick" in filename_lower or "short" in filename_lower:
                duration = 2000
                duration_reason = "quick/short keyword"
            elif "long" in filename_lower or "schedule" in filename_lower:
                duration = 10000
                duration_reason = "long/schedule keyword"
            elif "banner" in filename_lower or "logo" in filename_lower:
                duration = 3000
                duration_reason = "banner/logo keyword"

            print(f"📷 {filename}")
            print(f"   Type: Image ({file_size:,} bytes)")
            print(f"   Duration: {duration}ms ({duration_reason})")
            print()

        elif file_ext in video_extensions:
            # Predict mute setting
            muted = False
            mute_reason = "default unmuted"

            filename_lower = filename.lower()
            if (
                "mute" in filename_lower
                or "silent" in filename_lower
                or "background" in filename_lower
            ):
                muted = True
                mute_reason = "mute/silent/background keyword"
            elif (
                "sound" in filename_lower
                or "audio" in filename_lower
                or "announcement" in filename_lower
            ):
                muted = False
                mute_reason = "sound/audio/announcement keyword"

            print(f"🎬 {filename}")
            print(f"   Type: Video ({file_size:,} bytes)")
            print(f"   Audio: {'Muted' if muted else 'With sound'} ({mute_reason})")
            print()

        else:
            print(f"❓ {filename}")
            print(f"   Type: Unknown (extension: {file_ext})")
            print(f"   Size: {file_size:,} bytes")
            print()


def show_playlist_rules():
    """Show the rules for automatic playlist generation"""
    print("Automatic Playlist Generation Rules")
    print("=" * 50)

    print("\n🖼️  IMAGE DURATION RULES:")
    print("   • Default: 5 seconds")
    print("   • Contains 'quick' or 'short': 2 seconds")
    print("   • Contains 'long' or 'schedule': 10 seconds")
    print("   • Contains 'banner' or 'logo': 3 seconds")

    print("\n🎬 VIDEO AUDIO RULES:")
    print("   • Default: Unmuted (with sound)")
    print("   • Contains 'mute', 'silent', or 'background': Muted")
    print("   • Contains 'sound', 'audio', or 'announcement': Unmuted")

    print("\n📁 SUPPORTED FILE TYPES:")
    print("   • Images: .jpg, .jpeg, .png, .gif, .bmp, .webp")
    print("   • Videos: .mp4, .avi, .mov, .wmv, .flv, .webm, .mkv")

    print("\n📋 PLAYLIST BEHAVIOR:")
    print("   • Files are sorted alphabetically")
    print("   • Videos play for full duration")
    print("   • Images display for specified duration")
    print("   • Playlist loops automatically when finished")


def test_playlist_generation(media_dir=None):
    """Test the playlist generation and show results"""
    print("Testing Playlist Generation")
    print("=" * 50)

    # Show rules
    show_playlist_rules()

    # Analyze current files
    print("\n" + "=" * 50)
    analyze_media_files(media_dir)

    # Generate playlist
    print("=" * 50)
    print("GENERATING PLAYLIST...")
    print("-" * 30)

    try:
        playlist_data = generate_playlist_from_media_dir(media_dir)

        print(
            f"\n✅ SUCCESS: Generated playlist with {len(playlist_data['items'])} items"
        )
        print("\nPlaylist contents:")

        for i, item in enumerate(playlist_data["items"], 1):
            print(f"\n{i}. {item['type'].upper()}: {item['url']}")
            if item["type"] == "image":
                print(f"   Duration: {item['duration']}ms")
            else:  # video
                print(f"   Audio: {'Muted' if item['muted'] else 'With sound'}")

        # Save readable output
        output_file = "test_playlist_output.json"
        with open(output_file, "w") as f:
            json.dump(playlist_data, f, indent=2)
        print(f"\n📄 Full playlist saved to: {output_file}")

        return True

    except Exception as e:
        print(f"\n❌ ERROR: {e}")
        import traceback

        traceback.print_exc()
        return False


if __name__ == "__main__":
    import sys

    if len(sys.argv) > 1:
        if sys.argv[1] == "analyze":
            analyze_media_files()
        elif sys.argv[1] == "rules":
            show_playlist_rules()
        elif sys.argv[1] == "generate":
            generate_playlist_from_media_dir()
            print("Playlist generated!")
        else:
            print("Usage: python playlist_generator.py [analyze|rules|generate]")
    else:
        # Run full test
        if test_playlist_generation():
            print("\n🎉 Playlist generation test PASSED!")
            print("\nTo use with the server:")
            print("1. Start server: python server.py")
            print("2. Test endpoint: GET http://localhost:8080/api/media/playlist")
        else:
            print("\n💥 Playlist generation test FAILED!")
            sys.exit(1)
