#include "mediaplayer.h"
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

MediaPlayer::MediaPlayer(QVideoWidget *videoOutput, QLabel *imageLabel, QStackedLayout *layout, QObject *parent)
    : QObject(parent)
    , m_videoOutput(videoOutput)
    , m_imageLabel(imageLabel)
    , m_layout(layout)
    , m_isPlaying(false)
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
                if (status == QMediaPlayer::EndOfMedia) {
                    onVideoFinished();
                }
            });
    
    // Log any media player errors
    connect(m_player, MEDIAPLAYER_ERROR_SIGNAL,
            this, [this](QMediaPlayer::Error error, const QString &errorString){
                HANDLE_MEDIA_ERROR(error, errorString);
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
}

void MediaPlayer::setPlaylist(const MediaPlaylist &playlist)
{
    stop();
    m_playlist = playlist;
    m_playlist.currentIndex = 0;
    qDebug() << "Playlist set with" << m_playlist.items.size() << "items";
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
    
    // Stop current playback
    m_player->stop();
    m_imageTimer->stop();
    m_screenTimer->stop();
    
    // Move to next item
    m_playlist.moveToNext();
    
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
    qDebug() << "Playing item:" << currentItem.type << currentItem.url << "duration:" << currentItem.duration;
    
    emit mediaChanged(currentItem);
    
    if (currentItem.type == "video") {
        showVideo();
        
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
        SET_MEDIA_SOURCE(m_player, createUrl(currentItem.url));
        
        // Set mute state
        SET_AUDIO_MUTED(m_player, currentItem.muted);
        COMPAT_DEBUG("Video muted:" << currentItem.muted);
        
        m_player->play();
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
    // If it's a network URL, download the image
    if (url.startsWith("http://") || url.startsWith("https://")) {
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        QNetworkRequest request = createNetworkRequest(url);
        QNetworkReply *reply = manager->get(request);
        
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray imageData = reply->readAll();
                QPixmap pixmap;
                if (pixmap.loadFromData(imageData)) {
                    m_currentImage = pixmap;
                    scaleAndSetImage(pixmap);
                } else {
                    COMPAT_DEBUG("Failed to load image from network data");
                    // Keep the current image or fallback
                }
            } else {
                HANDLE_NETWORK_ERROR(reply->error(), reply->errorString());
                // Keep the current image or fallback
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
        } else {
            COMPAT_DEBUG("Failed to load local image:" << imagePath);
            // Keep the current image or fallback
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