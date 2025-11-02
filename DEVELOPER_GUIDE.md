# VideoTimeline Developer Quick Reference

## New Components Overview

### MediaCache
```cpp
// Usage in code
MediaCache *cache = new MediaCache(this);
cache->setMaxSize(4LL * 1024 * 1024 * 1024); // 4GB

// Check if cached
if (cache->isCached(url)) {
    QString path = cache->getCachedPath(url);
    // Use cached file
}

// Cache a file
cache->cacheFile(url, data);

// Prefetch
cache->prefetchUrl(nextUrl);

// Get statistics
CacheStats stats = cache->getStats();
qDebug() << "Hit rate:" << stats.hitRate() << "%";
```

### Logger
```cpp
// Set log level
Logger::instance().setLogLevel(LogLevel::Debug);

// Enable file logging
Logger::instance().enableFileLogging(true);

// Log messages
LOG_ERROR("Critical error occurred");
LOG_WARNING_CAT("Connection unstable", "Network");
LOG_INFO("Operation completed successfully");
LOG_DEBUG_CAT("Variable x = " << x, "Algorithm");
```

### DiagnosticsOverlay
```cpp
// Create and show
DiagnosticsOverlay *diag = new DiagnosticsOverlay(parent);
diag->show(); // Toggles visibility

// Update information
diag->setServerInfo(url, hostname, ping, connected);
diag->setMediaInfo(codec, hwDecode, resolution, fps);
diag->setCacheStats(cacheStats);
```

### MediaPlayer Transitions
```cpp
MediaPlayer *player = new MediaPlayer(videoOutput, imageLabel, layout, parent);

// Set cache for automatic caching
player->setMediaCache(cache);

// Configure transitions
player->setFadeDuration(300); // 300ms fade
player->enableTransitions(true);

// Transitions happen automatically on next()
player->next();
```

### NetworkClient Backoff
```cpp
NetworkClient *client = new NetworkClient(parent);

// Exponential backoff is automatic
// On connection loss:
// - Attempt 1: 1s wait
// - Attempt 2: 2s wait
// - Attempt 3: 4s wait
// - ...
// - Max: 60s wait

// Manually reset backoff (e.g., on user action)
// Add to networkclient.h: void resetBackoff();
```

## Key Signals and Slots

### MediaCache Signals
```cpp
signals:
    void cacheUpdated();
    void prefetchComplete(const QString &url, bool success);
```

### StatusBar Signals
```cpp
signals:
    void toggleDiagnostics();
    void logLevelChangeRequested(const QString &level);
```

### MediaPlayer Signals
```cpp
signals:
    void mediaChanged(const MediaItem &item);
    void playlistFinished();
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
```

### NetworkClient Signals
```cpp
signals:
    void serverDiscovered(const QString &serverUrl);
    void connectionStatusChanged(bool connected, const QString &serverUrl, const QString &hostname);
    void pingUpdated(int pingMs);
```

## Cache File Locations

```
~/.cache/VideoTimeline/media/         # Cached media files
~/.cache/VideoTimeline/media/index.json  # Cache metadata

~/.local/share/VideoTimeline/logs/    # Log files
~/.local/share/VideoTimeline/logs/app.log
```

## Build Commands

```bash
# Full rebuild
cd build
cmake .. && make -j$(nproc)

# Clean build
rm -rf build && mkdir build && cd build && cmake .. && make -j$(nproc)

# Install (optional)
sudo make install
```

## Testing

### Test Cache
```bash
# Run with small cache to test eviction
./VideoTimeline --auto --cache-size 2

# Monitor cache directory
watch -n 1 "ls -lh ~/.cache/VideoTimeline/media/"
```

### Test Logging
```bash
# Enable debug logging
./VideoTimeline --auto --log-level debug --log-file

# Tail logs in real-time
tail -f ~/.local/share/VideoTimeline/logs/app.log
```

### Test Transitions
```bash
# Create a playlist with mixed media types
# Watch for smooth fades between items
./VideoTimeline --network 10.1.1.1:3232
```

### Test Network Resilience
```bash
# Start normally
./VideoTimeline --auto --log-level info

# Stop server and observe:
# - Backoff timing in logs
# - Offline mode indicator
# - Cached content continues playing

# Restart server and observe:
# - Automatic reconnection
# - Backoff reset
```

### Test Diagnostics
```bash
# Run and press F12
./VideoTimeline --auto

# Check:
# - All network info populated
# - Cache stats updating
# - Media properties shown
```

## Common Issues and Solutions

### Cache not working
- Check permissions on `~/.cache/VideoTimeline/`
- Verify cache size is reasonable (2-8 GB)
- Check disk space

### Transitions not smooth
- Increase fade duration: modify `m_fadeDuration` in MediaPlayer
- Check GPU/video driver support
- Disable transitions temporarily: `player->enableTransitions(false)`

### Logs not created
- Check `~/.local/share/VideoTimeline/logs/` permissions
- Ensure `--log-file` flag is used
- Verify disk space

### Diagnostics overlay not showing
- Press F12 (check keyboard works)
- Right-click status bar and select "Toggle Diagnostics"
- Check window focus

## Performance Tuning

### Cache Size
```cpp
// Adjust based on available disk space
// and typical media file sizes
cache->setMaxSize(8LL * 1024 * 1024 * 1024); // 8GB for lots of videos
```

### Prefetch Timing
```cpp
// In MediaPlayer::prefetchNextItem()
// Adjust when to prefetch (currently on every item change)
// Could add delay or only prefetch for slow networks
```

### Backoff Parameters
```cpp
// In NetworkClient constructor
m_currentBackoffMs = 1000;  // Initial delay
const int MAX_BACKOFF_MS = 60000;  // Max delay
const int BACKOFF_MULTIPLIER = 2;  // Growth rate

// For faster reconnection:
m_currentBackoffMs = 500;  // 0.5s initial
MAX_BACKOFF_MS = 30000;    // 30s max
```

### Fade Duration
```cpp
// In MediaPlayer constructor
m_fadeDuration = 300;  // 300ms (default)

// Faster transitions:
m_fadeDuration = 150;  // 150ms

// Smoother, slower:
m_fadeDuration = 500;  // 500ms
```

## Debugging Tips

### Enable Maximum Logging
```bash
./VideoTimeline --auto --log-level debug --log-file
```

### Watch Network Traffic
```bash
# Monitor reconnection attempts
./VideoTimeline --auto 2>&1 | grep -i "reconnect\|backoff"
```

### Check Cache Statistics
```cpp
// Add to periodic timer in MainWindow
CacheStats stats = m_mediaCache->getStats();
LOG_INFO(QString("Cache: %1% hit rate, %2/%3 items, %4 MB")
    .arg(stats.hitRate(), 0, 'f', 1)
    .arg(stats.hits).arg(stats.misses)
    .arg(stats.totalSize / (1024 * 1024)));
```

### Profile Media Loading
```cpp
// Add timing around media operations
QElapsedTimer timer;
timer.start();
// ... operation ...
LOG_DEBUG(QString("Operation took %1 ms").arg(timer.elapsed()));
```

## Integration Examples

### Add Custom Status Indicator
```cpp
// In StatusBar::setupUI()
QLabel *customLabel = new QLabel("Custom");
layout->addWidget(customLabel);

// Update in slot
void StatusBar::setCustomInfo(const QString &info) {
    customLabel->setText(info);
}
```

### Add Diagnostic Field
```cpp
// In DiagnosticsOverlay::setupUI()
gridLayout->addWidget(new QLabel("Custom Field:"), row, 0);
m_customLabel = new QLabel("--");
gridLayout->addWidget(m_customLabel, row++, 1);

// Update
void DiagnosticsOverlay::setCustomInfo(const QString &info) {
    m_customLabel->setText(info);
}
```

### Custom Cache Policy
```cpp
// Extend MediaCache
class SmartCache : public MediaCache {
    // Override evictLRU() to implement custom eviction
    // E.g., keep frequently accessed items longer
};
```
