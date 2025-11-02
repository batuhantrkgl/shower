#include "mediaplayer.h"
#include "mediacache.h"
#include "logger.h"
#include "qt6compat.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPixmap>
#include <QDebug>
#include <QUrl>
#include <QScreen>
#include <QGuiApplication>
#include <QWidget>
#include <QApplication>
#include <QPainter>
#include <QFont>
#include <QMediaMetaData>

MediaPlayer::MediaPlayer(QVideoWidget *videoOutput, QLabel *imageLabel, QStackedLayout *layout, QObject *parent)
    : QObject(parent)
    , m_videoOutput(videoOutput)
    , m_imageLabel(imageLabel)
    , m_layout(layout)
    , m_mediaCache(nullptr)
    , m_isPlaying(false)
    , m_fadeDuration(300)
    , m_transitionsEnabled(true)
    , m_isFading(false)
    , m_hwDecodeEnabled(false)
    , m_currentFps(0.0)
{
    // Initialize media player
    m_player = new QMediaPlayer(this);
    m_player->setVideoOutput(m_videoOutput);
    
    // Initialize audio output for Qt6 compatibility
    SETUP_AUDIO_OUTPUT(m_player);
    
    // Connect video player signals
    connect(m_player, &QMediaPlayer::playbackStateChanged, 
            this, &MediaPlayer::onVideoStateChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, 
            this, [this](QMediaPlayer::MediaStatus status) {
                emit mediaStatusChanged(status);
                if (status == QMediaPlayer::EndOfMedia) {
                    onVideoFinished();
                } else if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia) {
                    // Detect codec properties when media is loaded
                    detectMediaProperties();
                }
            });
    
    // Log any media player errors
    connect(m_player, MEDIAPLAYER_ERROR_SIGNAL,
            this, [this](QMediaPlayer::Error error, const QString &errorString){
                HANDLE_MEDIA_ERROR(error, errorString);
                LOG_ERROR_CAT(QString("Media error: %1").arg(errorString), "MediaPlayer");
                // Move to next item on error
                next();
            });
    
    // Initialize image timer
    m_imageTimer = new QTimer(this);
    m_imageTimer->setSingleShot(true);
    connect(m_imageTimer, &QTimer::timeout, this, &MediaPlayer::onImageTimerFinished);

    // Initialize screen capture timer (capture every 100ms for smooth display)
    m_screenTimer = new QTimer(this);
    connect(m_screenTimer, &QTimer::timeout, this, &MediaPlayer::onScreenCaptureTimer);

    // Initialize screen label
    m_screenLabel = new QLabel();
    m_screenLabel->setAlignment(Qt::AlignCenter);
    m_screenLabel->setScaledContents(true);
    m_layout->addWidget(m_screenLabel); // Add to layout at SCREEN_INDEX
    
    // Setup opacity effects for transitions
    m_videoOpacity = new QGraphicsOpacityEffect(m_videoOutput);
    m_videoOutput->setGraphicsEffect(m_videoOpacity);
    m_videoOpacity->setOpacity(1.0);
    
    m_imageOpacity = new QGraphicsOpacityEffect(m_imageLabel);
    m_imageLabel->setGraphicsEffect(m_imageOpacity);
    m_imageOpacity->setOpacity(1.0);
    
    m_screenOpacity = new QGraphicsOpacityEffect(m_screenLabel);
    m_screenLabel->setGraphicsEffect(m_screenOpacity);
    m_screenOpacity->setOpacity(1.0);
    
    // Setup fade animation
    m_fadeAnimation = new QPropertyAnimation(this);
    m_fadeAnimation->setDuration(m_fadeDuration);
    connect(m_fadeAnimation, &QPropertyAnimation::finished, 
            this, &MediaPlayer::onFadeInFinished);
    
    LOG_INFO_CAT("MediaPlayer initialized with transitions enabled", "MediaPlayer");
}

void MediaPlayer::setPlaylist(const MediaPlaylist &playlist)
{
    stop();
    m_playlist = playlist;
    m_playlist.currentIndex = 0;
    LOG_INFO_CAT(QString("Playlist set with %1 items").arg(m_playlist.items.size()), "MediaPlayer");
}

void MediaPlayer::setMediaCache(MediaCache *cache)
{
    m_mediaCache = cache;
    if (m_mediaCache) {
        connect(m_mediaCache, &MediaCache::prefetchComplete,
                this, &MediaPlayer::onPrefetchComplete);
        LOG_INFO_CAT("Media cache connected", "MediaPlayer");
    }
}

void MediaPlayer::play()
{
    if (!m_playlist.hasItems()) {
        qDebug() << "Cannot play: playlist is empty";
        return;
    }
    
    m_isPlaying = true;
    playCurrentItem();
}

void MediaPlayer::stop()
{
    m_isPlaying = false;
    m_player->stop();
    m_imageTimer->stop();
    m_screenTimer->stop();
}

void MediaPlayer::next()
{
    if (!m_playlist.hasItems()) {
        return;
    }
    
    // Check if next item is a video - skip transitions for videos
    int nextIndex = (m_playlist.currentIndex + 1) % m_playlist.items.size();
    const MediaItem &currentItem = m_playlist.getCurrentItem();
    const MediaItem &nextItem = m_playlist.items[nextIndex];
    bool skipTransition = (currentItem.type == "video" || nextItem.type == "video");
    
    // Start fade out before switching (only for image-to-image transitions)
    if (m_transitionsEnabled && !m_isFading && !skipTransition) {
        fadeOut();
        // The actual transition will happen in onFadeOutFinished()
        return;
    }
    
    // For instant transitions (involving videos), reset all opacities to 1.0 FIRST
    if (skipTransition) {
        m_videoOpacity->setOpacity(1.0);
        m_imageOpacity->setOpacity(1.0);
        m_screenOpacity->setOpacity(1.0);
    }
    
    // Only stop if NOT transitioning to a video (to avoid blank screen)
    if (nextItem.type != "video") {
        m_player->stop();
    }
    m_imageTimer->stop();
    m_screenTimer->stop();
    
    // Move to next item
    m_playlist.moveToNext();
    
    // Prefetch the next item after this one
    prefetchNextItem();
    
    if (m_isPlaying) {
        playCurrentItem();
    }
}

void MediaPlayer::playCurrentItem()
{
    if (!m_playlist.hasItems()) {
        qDebug() << "Cannot play current item: playlist is empty";
        return;
    }
    
    MediaItem currentItem = m_playlist.getCurrentItem();
    LOG_INFO_CAT(QString("Playing: %1 %2").arg(currentItem.type).arg(currentItem.url), "MediaPlayer");
    
    emit mediaChanged(currentItem);
    
    if (currentItem.type == "video") {
        // Check cache first
        QString mediaUrl = currentItem.url;
        if (m_mediaCache && (mediaUrl.startsWith("http://") || mediaUrl.startsWith("https://"))) {
            QString cachedPath = m_mediaCache->getCachedPath(mediaUrl);
            if (!cachedPath.isEmpty()) {
                mediaUrl = "file://" + cachedPath;
                LOG_INFO_CAT(QString("Using cached video: %1").arg(cachedPath), "MediaPlayer");
            }
        }
        
        // Properly reset the media player to avoid "partial file" errors on replay
        m_player->stop();
        
        // Clear the current source completely before setting a new one
        #ifdef QT6_OR_LATER
            m_player->setSource(QUrl());  // Clear source in Qt6
        #else
            m_player->setMedia(QMediaContent());  // Clear media in Qt5
        #endif
        
        // Reset position
        m_player->setPosition(0);
        
        // Now set the new source
        SET_MEDIA_SOURCE(m_player, createUrl(mediaUrl));
        
        // Set mute state
        SET_AUDIO_MUTED(m_player, currentItem.muted);
        COMPAT_DEBUG("Video muted:" << currentItem.muted);
        
        // Show video widget AFTER loading source, BEFORE playing
        showVideo();
        
        m_player->play();
        
        // Note: detectMediaProperties() is called when media status changes to LoadedMedia
        
    } else if (currentItem.type == "image") {
        showImage();
        loadImage(currentItem.url);
        startImageTimer(currentItem.duration);
    } else if (currentItem.type == "screen") {
        showScreen();
        // Start screen capture timer (capture every 100ms)
        m_screenTimer->start(100);
    } else {
        qDebug() << "Unknown media type:" << currentItem.type;
        next(); // Skip unknown types
    }
}

void MediaPlayer::showVideo()
{
    m_layout->setCurrentIndex(VIDEO_WIDGET_INDEX);
}

void MediaPlayer::showImage()
{
    m_layout->setCurrentIndex(IMAGE_WIDGET_INDEX);
}

void MediaPlayer::showScreen()
{
    m_layout->setCurrentIndex(SCREEN_INDEX);
}

void MediaPlayer::loadImage(const QString &url)
{
    // Check cache first for network URLs
    if (m_mediaCache && (url.startsWith("http://") || url.startsWith("https://"))) {
        QString cachedPath = m_mediaCache->getCachedPath(url);
        if (!cachedPath.isEmpty()) {
            // Load from cache
            QPixmap pixmap(cachedPath);
            if (!pixmap.isNull()) {
                m_currentImage = pixmap;
                scaleAndSetImage(pixmap);
                LOG_INFO_CAT(QString("Using cached image: %1").arg(cachedPath), "MediaPlayer");
                
                // Detect image properties for cached image
                detectImageProperties(url);
                return;
            }
        }
    }
    
    // If it's a network URL, download the image
    if (url.startsWith("http://") || url.startsWith("https://")) {
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        QNetworkRequest request = createNetworkRequest(url);
        QNetworkReply *reply = manager->get(request);
        
        connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray imageData = reply->readAll();
                QPixmap pixmap;
                if (pixmap.loadFromData(imageData)) {
                    m_currentImage = pixmap;
                    scaleAndSetImage(pixmap);
                    
                    // Cache the image
                    if (m_mediaCache) {
                        m_mediaCache->cacheFile(url, imageData);
                    }
                    
                    // Detect image properties after loading
                    detectImageProperties(url);
                } else {
                    COMPAT_DEBUG("Failed to load image from network data");
                    LOG_ERROR_CAT("Failed to load image from network data", "MediaPlayer");
                }
            } else {
                HANDLE_NETWORK_ERROR(reply->error(), reply->errorString());
                LOG_ERROR_CAT(QString("Network error loading image: %1").arg(reply->errorString()), "MediaPlayer");
            }
            reply->deleteLater();
        });
    } else {
        // Local file path - handle both absolute and relative paths
        QString imagePath = convertMediaPath(url);
        
        QPixmap pixmap(imagePath);
        if (!pixmap.isNull()) {
            m_currentImage = pixmap;
            scaleAndSetImage(pixmap);
            COMPAT_DEBUG("Loaded local image:" << imagePath);
            
            // Detect image properties after loading
            detectImageProperties(url);
        } else {
            COMPAT_DEBUG("Failed to load local image:" << imagePath);
            LOG_ERROR_CAT(QString("Failed to load local image: %1").arg(imagePath), "MediaPlayer");
        }
    }
}

void MediaPlayer::startImageTimer(int durationMs)
{
    if (durationMs <= 0) {
        COMPAT_DEBUG("Invalid image duration:" << durationMs);
        durationMs = 5000; // Default to 5 seconds
    }
    
    COMPAT_DEBUG("Starting image timer for" << durationMs << "ms");
    startSingleShotTimer(m_imageTimer, durationMs);
}

void MediaPlayer::onImageTimerFinished()
{
    COMPAT_DEBUG("Image timer finished, moving to next");
    next();
}

void MediaPlayer::scaleAndSetImage(const QPixmap &originalPixmap)
{
    if (originalPixmap.isNull()) {
        return;
    }
    
    // Get the available size for the image
    QSize labelSize = m_imageLabel->size();
    
    // If the label doesn't have a size yet (e.g., during initialization), 
    // use a reasonable default or the parent widget size
    if (labelSize.width() <= 0 || labelSize.height() <= 0) {
        if (m_imageLabel->parentWidget()) {
            labelSize = m_imageLabel->parentWidget()->size();
        } else {
            labelSize = QSize(800, 600); // Fallback size
        }
    }
    
    // Scale the image to fit within the available space while maintaining aspect ratio
    QPixmap scaledPixmap = originalPixmap.scaled(labelSize, 
                                               Qt::KeepAspectRatio, 
                                               Qt::SmoothTransformation);
    
    m_imageLabel->setPixmap(scaledPixmap);
}

void MediaPlayer::rescaleCurrentImage()
{
    if (!m_currentImage.isNull()) {
        scaleAndSetImage(m_currentImage);
    }
}

void MediaPlayer::onVideoFinished()
{
    COMPAT_DEBUG("Video finished, moving to next");
    next();
}

void MediaPlayer::onVideoStateChanged(QMediaPlayer::PlaybackState state)
{
    COMPAT_DEBUG("Video state changed:" << state);
    
    if (state == QMediaPlayer::StoppedState) {
        // Video was stopped (could be due to error or end of media)
        // The mediaStatusChanged signal will handle EndOfMedia case
    }
}

void MediaPlayer::onScreenCaptureTimer()
{
    captureScreen();
}

void MediaPlayer::captureScreen()
{
    qDebug() << "Attempting screen capture...";

    // Get the primary screen
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        qDebug() << "ERROR: No primary screen found for screen capture";
        // Fallback: create a placeholder image
        QPixmap placeholder(800, 600);
        placeholder.fill(Qt::black);
        QPainter painter(&placeholder);
        painter.setPen(Qt::white);
        painter.setFont(QFont("SF Pro Display", 24));
        painter.drawText(placeholder.rect(), Qt::AlignCenter, "Screen Capture\nNot Available");
        m_screenLabel->setPixmap(placeholder.scaled(m_screenLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        return;
    }

    qDebug() << "Primary screen found:" << screen->name() << "size:" << screen->size();

    // Check display server type
    QString sessionType = qgetenv("XDG_SESSION_TYPE");
    bool isWayland = (sessionType == "wayland");

    qDebug() << "Display server type:" << sessionType;

    // On Wayland, screen capture is restricted by design for security
    if (isWayland) {
        qDebug() << "Wayland detected - screen capture not supported with Qt";

        // Create helpful error message for Wayland
        QPixmap placeholder(800, 600);
        placeholder.fill(Qt::darkBlue);
        QPainter painter(&placeholder);
        painter.setPen(Qt::white);
        painter.setFont(QFont("SF Pro Display", 16));

        QString waylandMessage =
            "Screen Mirroring Unavailable\n\n"
            "This application is running on Wayland,\n"
            "which restricts screen capture for security.\n\n"
            "To enable screen mirroring:\n"
            "• Log out and select X11/Xorg session\n"
            "• Or use a different display manager\n\n"
            "Wayland security prevents Qt applications\n"
            "from capturing the screen without special\n"
            "permissions or portal APIs.";

        painter.drawText(placeholder.rect(), Qt::AlignCenter, waylandMessage);
        m_screenLabel->setPixmap(placeholder.scaled(m_screenLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        return;
    }

    // Try screen capture (should work on X11)
    QPixmap screenshot = screen->grabWindow(0);
    if (screenshot.isNull()) {
        qDebug() << "Screen capture failed on X11";

        // Create error placeholder for X11 issues
        QPixmap placeholder(800, 600);
        placeholder.fill(Qt::darkRed);
        QPainter painter(&placeholder);
        painter.setPen(Qt::white);
        painter.setFont(QFont("SF Pro Display", 16));

        QString x11Message =
            "Screen Capture Failed\n\n"
            "Running on X11 but capture failed.\n"
            "Possible causes:\n"
            "• Missing X11 permissions\n"
            "• Compositor restrictions\n"
            "• Display access issues\n\n"
            "Check X11 configuration.";

        painter.drawText(placeholder.rect(), Qt::AlignCenter, x11Message);
        m_screenLabel->setPixmap(placeholder.scaled(m_screenLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        return;
    }

    qDebug() << "Screen captured successfully, size:" << screenshot.size();

    // Scale the screenshot to fit the label while maintaining aspect ratio
    QSize labelSize = m_screenLabel->size();
    if (labelSize.width() <= 0 || labelSize.height() <= 0) {
        if (m_screenLabel->parentWidget()) {
            labelSize = m_screenLabel->parentWidget()->size();
        } else {
            labelSize = QSize(800, 600); // Fallback size
        }
    }

    QPixmap scaledScreenshot = screenshot.scaled(labelSize,
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation);

    m_screenLabel->setPixmap(scaledScreenshot);
    qDebug() << "Screen displayed on label, scaled to:" << scaledScreenshot.size();
}

void MediaPlayer::fadeOut()
{
    if (m_isFading) return;
    
    m_isFading = true;
    LOG_DEBUG_CAT("Starting fade out", "MediaPlayer");
    
    // Determine which widget is currently visible
    QGraphicsOpacityEffect *effect = nullptr;
    int currentIndex = m_layout->currentIndex();
    
    if (currentIndex == VIDEO_INDEX) {
        effect = m_videoOpacity;
    } else if (currentIndex == IMAGE_INDEX) {
        effect = m_imageOpacity;
    } else if (currentIndex == SCREEN_INDEX) {
        effect = m_screenOpacity;
    }
    
    if (effect) {
        m_fadeAnimation->setTargetObject(effect);
        m_fadeAnimation->setPropertyName("opacity");
        m_fadeAnimation->setStartValue(1.0);
        m_fadeAnimation->setEndValue(0.0);
        m_fadeAnimation->setDuration(m_fadeDuration);
        
        disconnect(m_fadeAnimation, &QPropertyAnimation::finished, 
                   this, &MediaPlayer::onFadeInFinished);
        connect(m_fadeAnimation, &QPropertyAnimation::finished, 
                this, &MediaPlayer::onFadeOutFinished);
        
        m_fadeAnimation->start();
    } else {
        // No effect, just transition directly
        m_isFading = false;
        onFadeOutFinished();
    }
}

void MediaPlayer::fadeIn()
{
    LOG_DEBUG_CAT("Starting fade in", "MediaPlayer");
    
    // Determine which widget is now visible
    QGraphicsOpacityEffect *effect = nullptr;
    int currentIndex = m_layout->currentIndex();
    
    if (currentIndex == VIDEO_INDEX) {
        effect = m_videoOpacity;
    } else if (currentIndex == IMAGE_INDEX) {
        effect = m_imageOpacity;
    } else if (currentIndex == SCREEN_INDEX) {
        effect = m_screenOpacity;
    }
    
    if (effect) {
        // Opacity should already be 0 from onFadeOutFinished
        m_fadeAnimation->setTargetObject(effect);
        m_fadeAnimation->setPropertyName("opacity");
        m_fadeAnimation->setStartValue(0.0);
        m_fadeAnimation->setEndValue(1.0);
        m_fadeAnimation->setDuration(m_fadeDuration);
        
        disconnect(m_fadeAnimation, &QPropertyAnimation::finished, 
                   this, &MediaPlayer::onFadeOutFinished);
        connect(m_fadeAnimation, &QPropertyAnimation::finished, 
                this, &MediaPlayer::onFadeInFinished);
        
        m_fadeAnimation->start();
    } else {
        m_isFading = false;
    }
}

void MediaPlayer::onFadeOutFinished()
{
    LOG_DEBUG_CAT("Fade out finished", "MediaPlayer");
    
    // Stop current playback
    m_player->stop();
    m_imageTimer->stop();
    m_screenTimer->stop();
    
    // Move to next item
    m_playlist.moveToNext();
    
    // Prefetch the next item after this one
    prefetchNextItem();
    
    if (m_isPlaying) {
        // Pre-set the NEXT widget's opacity to 0 before switching
        // Don't touch the current widget to avoid flicker
        if (m_transitionsEnabled) {
            const MediaItem &nextItem = m_playlist.getCurrentItem();
            if (nextItem.type == "video") {
                m_videoOpacity->setOpacity(0.0);
            } else if (nextItem.type == "image") {
                m_imageOpacity->setOpacity(0.0);
            } else if (nextItem.type == "screen") {
                m_screenOpacity->setOpacity(0.0);
            }
        }
        
        playCurrentItem();
        
        // Start fade in
        if (m_transitionsEnabled) {
            fadeIn();
        } else {
            m_isFading = false;
        }
    } else {
        m_isFading = false;
    }
}

void MediaPlayer::onFadeInFinished()
{
    LOG_DEBUG_CAT("Fade in finished", "MediaPlayer");
    m_isFading = false;
}

void MediaPlayer::prefetchNextItem()
{
    if (!m_mediaCache || !m_playlist.hasItems()) {
        return;
    }
    
    // Calculate next index
    int nextIndex = (m_playlist.currentIndex + 1) % m_playlist.items.size();
    
    if (nextIndex < m_playlist.items.size()) {
        const MediaItem &nextItem = m_playlist.items[nextIndex];
        
        // Only prefetch network URLs for videos and images
        if ((nextItem.type == "video" || nextItem.type == "image") &&
            (nextItem.url.startsWith("http://") || nextItem.url.startsWith("https://"))) {
            
            LOG_DEBUG_CAT(QString("Prefetching next item: %1").arg(nextItem.url), "MediaPlayer");
            m_mediaCache->prefetchUrl(nextItem.url);
        }
    }
}

void MediaPlayer::onPrefetchComplete(const QString &url, bool success)
{
    if (success) {
        LOG_INFO_CAT(QString("Prefetch successful: %1").arg(url), "MediaPlayer");
    } else {
        LOG_WARNING_CAT(QString("Prefetch failed: %1").arg(url), "MediaPlayer");
    }
}

void MediaPlayer::detectMediaProperties()
{
    // Detect codec and hardware acceleration
    // Note: QMediaPlayer doesn't provide direct access to codec info in Qt5/6
    // This is a placeholder for future enhancement or platform-specific implementation
    
    m_currentCodec = "unknown";
    m_hwDecodeEnabled = false;
    
    // Try to get video size and frame rate
    #ifdef QT6_OR_LATER
        // Qt6 has better metaData support
        QMediaMetaData metadata = m_player->metaData();
        if (metadata.value(QMediaMetaData::Key::Resolution).isValid()) {
            QSize resolution = metadata.value(QMediaMetaData::Key::Resolution).toSize();
            m_currentResolution = QString("%1x%2").arg(resolution.width()).arg(resolution.height());
        }
        if (metadata.value(QMediaMetaData::Key::VideoFrameRate).isValid()) {
            m_currentFps = metadata.value(QMediaMetaData::Key::VideoFrameRate).toReal();
        }
    #else
        // Qt5 metadata access
        if (m_player->isMetaDataAvailable()) {
            QSize resolution = m_player->metaData("Resolution").toSize();
            if (resolution.isValid()) {
                m_currentResolution = QString("%1x%2").arg(resolution.width()).arg(resolution.height());
            }
            m_currentFps = m_player->metaData("VideoFrameRate").toReal();
        }
    #endif
    
    // Check for VAAPI or other hardware acceleration hints
    // GStreamer will use VAAPI automatically if available
    QString mediaBackend = qgetenv("QT_MEDIA_BACKEND");
    QString vaDriver = qgetenv("LIBVA_DRIVER_NAME");
    QString vdpauDriver = qgetenv("VDPAU_DRIVER");
    
    // Assume hardware decode if using GStreamer with VAAPI driver
    if (mediaBackend == "gstreamer" && !vaDriver.isEmpty()) {
        m_hwDecodeEnabled = true;
        m_currentCodec = QString("H.264 (VAAPI-%1)").arg(vaDriver);
    } 
    // Or if backend explicitly mentions hardware decode
    else if (mediaBackend.contains("vaapi") || mediaBackend.contains("nvdec") || mediaBackend.contains("vdpau")) {
        m_hwDecodeEnabled = true;
        m_currentCodec = mediaBackend;
    }
    // Or if VDPAU is configured
    else if (!vdpauDriver.isEmpty()) {
        m_hwDecodeEnabled = true;
        m_currentCodec = QString("H.264 (VDPAU-%1)").arg(vdpauDriver);
    }
    // Otherwise assume software decode
    else if (mediaBackend == "ffmpeg" || mediaBackend == "gstreamer") {
        m_currentCodec = QString("H.264 (%1-sw)").arg(mediaBackend);
        m_hwDecodeEnabled = false;
    }
    
    LOG_DEBUG_CAT(QString("Media properties - Codec: %1, HW: %2, Resolution: %3, FPS: %4")
        .arg(m_currentCodec)
        .arg(m_hwDecodeEnabled ? "Yes" : "No")
        .arg(m_currentResolution)
        .arg(m_currentFps), "MediaPlayer");
    
    // Emit codec information for status bar
    emit codecDetected(m_currentCodec, m_hwDecodeEnabled);
}

void MediaPlayer::detectImageProperties(const QString &url)
{
    // Detect image format from URL or cached file
    QString imagePath = url;
    
    // If it's a network URL, try to get from cache
    if (m_mediaCache && (url.startsWith("http://") || url.startsWith("https://"))) {
        QString cachedPath = m_mediaCache->getCachedPath(url);
        if (!cachedPath.isEmpty()) {
            imagePath = cachedPath;
        }
    } else if (!url.startsWith("http")) {
        imagePath = convertMediaPath(url);
    }
    
    // Detect image format from file extension or data
    QString format = "Image";
    if (imagePath.endsWith(".png", Qt::CaseInsensitive)) {
        format = "PNG";
    } else if (imagePath.endsWith(".jpg", Qt::CaseInsensitive) || imagePath.endsWith(".jpeg", Qt::CaseInsensitive)) {
        format = "JPEG";
    } else if (imagePath.endsWith(".gif", Qt::CaseInsensitive)) {
        format = "GIF";
    } else if (imagePath.endsWith(".webp", Qt::CaseInsensitive)) {
        format = "WebP";
    } else if (imagePath.endsWith(".bmp", Qt::CaseInsensitive)) {
        format = "BMP";
    } else if (imagePath.endsWith(".svg", Qt::CaseInsensitive)) {
        format = "SVG";
    }
    
    // Get image size if available
    if (!m_currentImage.isNull()) {
        m_currentResolution = QString("%1x%2").arg(m_currentImage.width()).arg(m_currentImage.height());
    } else {
        m_currentResolution = "unknown";
    }
    
    m_currentCodec = format;
    m_hwDecodeEnabled = false; // Images don't use hardware decode
    m_currentFps = 0.0;
    
    LOG_DEBUG_CAT(QString("Image properties - Format: %1, Resolution: %2")
        .arg(m_currentCodec)
        .arg(m_currentResolution), "MediaPlayer");
    
    // Emit codec information for status bar
    emit codecDetected(m_currentCodec, m_hwDecodeEnabled);
}
