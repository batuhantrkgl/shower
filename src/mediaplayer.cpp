#include "mediaplayer.h"
#include "qt6compat.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPixmap>
#include <QDebug>
#include <QUrl>

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
}

void MediaPlayer::next()
{
    if (!m_playlist.hasItems()) {
        return;
    }
    
    // Stop current playback
    m_player->stop();
    m_imageTimer->stop();
    
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
        SET_MEDIA_SOURCE(m_player, createUrl(currentItem.url));
        
        // Set mute state
        SET_AUDIO_MUTED(m_player, currentItem.muted);
        COMPAT_DEBUG("Video muted:" << currentItem.muted);
        
        m_player->play();
    } else if (currentItem.type == "image") {
        showImage();
        loadImage(currentItem.url);
        startImageTimer(currentItem.duration);
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