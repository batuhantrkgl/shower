#!/usr/bin/env python3
"""
Simple server test script for VideoTimeline Server
Tests the playlist endpoint without external dependencies
"""

import json
import http.client
import urllib.parse
import sys
import time

# Server configuration
SERVER_HOST = "localhost"
SERVER_PORT = 8080


def test_connection():
    """Test basic connection to server"""
    print("Testing server connection...")
    try:
        conn = http.client.HTTPConnection(SERVER_HOST, SERVER_PORT, timeout=5)
        conn.request("GET", "/")
        response = conn.getresponse()
        conn.close()
        print("✓ Server is responding")
        return True
    except Exception as e:
        print(f"✗ Cannot connect to server: {e}")
        return False


def test_playlist_endpoint():
    """Test the playlist endpoint"""
    print("\nTesting /api/media/playlist endpoint...")
    try:
        conn = http.client.HTTPConnection(SERVER_HOST, SERVER_PORT, timeout=10)
        conn.request("GET", "/api/media/playlist")
        response = conn.getresponse()

        if response.status == 200:
            data = response.read().decode("utf-8")
            playlist = json.loads(data)

            print("✓ Playlist endpoint successful")
            print(f"  Status: {response.status}")
            print(f"  Items count: {len(playlist.get('items', []))}")

            # Show playlist contents
            items = playlist.get("items", [])
            if items:
                print("\n  Playlist contents:")
                for i, item in enumerate(items, 1):
                    item_type = item.get("type", "unknown")
                    url = item.get("url", "no url")

                    if item_type == "image":
                        duration = item.get("duration", 0)
                        print(f"    {i}. Image: {url} ({duration}ms)")
                    elif item_type == "video":
                        muted = item.get("muted", False)
                        print(
                            f"    {i}. Video: {url} ({'muted' if muted else 'with sound'})"
                        )
                    else:
                        print(f"    {i}. Unknown: {url}")
            else:
                print("  ⚠️  Playlist is empty")

            conn.close()
            return True

        else:
            print(f"✗ Playlist endpoint failed: HTTP {response.status}")
            data = response.read().decode("utf-8")
            print(f"  Response: {data}")
            conn.close()
            return False

    except Exception as e:
        print(f"✗ Error testing playlist endpoint: {e}")
        return False


def test_regenerate_endpoint():
    """Test the playlist regeneration endpoint"""
    print("\nTesting /api/media/regenerate endpoint...")
    try:
        conn = http.client.HTTPConnection(SERVER_HOST, SERVER_PORT, timeout=10)
        conn.request("GET", "/api/media/regenerate")
        response = conn.getresponse()

        if response.status == 200:
            data = response.read().decode("utf-8")
            result = json.loads(data)

            print("✓ Regenerate endpoint successful")
            print(f"  Status: {response.status}")
            print(f"  Message: {result.get('message', 'no message')}")

            # Show regenerated playlist info
            playlist = result.get("playlist", {})
            items = playlist.get("items", [])
            if items:
                print(f"  Regenerated {len(items)} items")

            conn.close()
            return True

        else:
            print(f"✗ Regenerate endpoint failed: HTTP {response.status}")
            data = response.read().decode("utf-8")
            print(f"  Response: {data}")
            conn.close()
            return False

    except Exception as e:
        print(f"✗ Error testing regenerate endpoint: {e}")
        return False


def test_media_file_serving():
    """Test serving media files"""
    print("\nTesting media file serving...")

    # Test files that should exist based on our test setup
    test_files = [
        "/media/default.jpg",
        "/media/welcome_quick.jpg",
        "/media/schedule_long.png",
    ]

    success_count = 0

    for file_path in test_files:
        try:
            conn = http.client.HTTPConnection(SERVER_HOST, SERVER_PORT, timeout=5)
            conn.request("GET", file_path)
            response = conn.getresponse()

            if response.status == 200:
                content_length = response.getheader("Content-Length", "0")
                content_type = response.getheader("Content-Type", "unknown")
                print(f"  ✓ {file_path}: {content_length} bytes, {content_type}")
                success_count += 1
            else:
                print(f"  ✗ {file_path}: HTTP {response.status}")

            conn.close()

        except Exception as e:
            print(f"  ✗ {file_path}: Error - {e}")

    print(f"\n  Successfully served {success_count}/{len(test_files)} test files")
    return success_count > 0


def test_schedule_endpoint():
    """Test the schedule endpoint for completeness"""
    print("\nTesting /api/schedule endpoint...")
    try:
        conn = http.client.HTTPConnection(SERVER_HOST, SERVER_PORT, timeout=5)
        conn.request("GET", "/api/schedule")
        response = conn.getresponse()

        if response.status == 200:
            data = response.read().decode("utf-8")
            schedule = json.loads(data)

            print("✓ Schedule endpoint successful")
            print(
                f"  School hours: {schedule.get('school_start', 'unknown')} - {schedule.get('school_end', 'unknown')}"
            )
            print(f"  Blocks: {len(schedule.get('blocks', []))}")

            conn.close()
            return True

        else:
            print(f"✗ Schedule endpoint failed: HTTP {response.status}")
            conn.close()
            return False

    except Exception as e:
        print(f"✗ Error testing schedule endpoint: {e}")
        return False


def show_server_info():
    """Show server connection info"""
    print("VideoTimeline Server Test")
    print("=" * 50)
    print(f"Server: http://{SERVER_HOST}:{SERVER_PORT}")
    print(f"Testing endpoints:")
    print(f"  • GET /api/media/playlist")
    print(f"  • GET /api/media/regenerate")
    print(f"  • GET /api/schedule")
    print(f"  • GET /media/<filename>")
    print()


def main():
    """Run all server tests"""
    show_server_info()

    # Test basic connection first
    if not test_connection():
        print("\n❌ Server is not running or not accessible")
        print("\nTo start the server:")
        print("  python server.py")
        print("\nThen run this test again:")
        print("  python test_server.py")
        return 1

    # Run all tests
    tests = [
        ("Playlist Endpoint", test_playlist_endpoint),
        ("Regenerate Endpoint", test_regenerate_endpoint),
        ("Media File Serving", test_media_file_serving),
        ("Schedule Endpoint", test_schedule_endpoint),
    ]

    passed = 0
    total = len(tests)

    for test_name, test_func in tests:
        print("-" * 50)
        if test_func():
            passed += 1
        else:
            print(f"  ⚠️  {test_name} test failed")

    # Summary
    print("\n" + "=" * 50)
    print("TEST SUMMARY")
    print(f"Passed: {passed}/{total}")

    if passed == total:
        print("🎉 ALL TESTS PASSED!")
        print("\nThe server is working correctly with playlist functionality.")
        print("\nExample playlist JSON format:")
        print("""
{
  "items": [
    {
      "type": "image",
      "url": "/media/welcome_quick.jpg",
      "duration": 2000,
      "muted": false
    },
    {
      "type": "video",
      "url": "/media/announcement_sound.mp4",
      "duration": -1,
      "muted": false
    }
  ]
}""")
        return 0
    else:
        print(f"❌ {total - passed} tests failed")
        print("\nCheck server logs and ensure all media files exist.")
        return 1


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("\n\nTest interrupted by user")
        sys.exit(0)
