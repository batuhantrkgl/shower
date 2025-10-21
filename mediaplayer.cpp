#include "mediaplayer.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPixmap>
#include <QDebug>
#include <QUrl>
#include <QAudioOutput>

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
    QAudioOutput *audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(audioOutput);
    
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
    connect(m_player, &QMediaPlayer::errorOccurred, 
            this, [=](QMediaPlayer::Error error, const QString &errorString){
                qDebug() << "MediaPlayer Error:" << error << errorString;
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
        m_player->setSource(QUrl(currentItem.url));
        
        // Set mute state
        QAudioOutput *audioOutput = m_player->audioOutput();
        if (audioOutput) {
            audioOutput->setMuted(currentItem.muted);
            qDebug() << "Video muted:" << currentItem.muted;
        }
        
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
    m_layout->setCurrentIndex(VIDEO_INDEX);
}

void MediaPlayer::showImage()
{
    m_layout->setCurrentIndex(IMAGE_INDEX);
}

void MediaPlayer::loadImage(const QString &url)
{
    // If it's a network URL, download the image
    if (url.startsWith("http://") || url.startsWith("https://")) {
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        QNetworkRequest request(QUrl(url));
        QNetworkReply *reply = manager->get(request);
        
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray imageData = reply->readAll();
                QPixmap pixmap;
                if (pixmap.loadFromData(imageData)) {
                    m_imageLabel->setPixmap(pixmap);
                } else {
                    qDebug() << "Failed to load image from network data";
                    // Keep the current image or fallback
                }
            } else {
                qDebug() << "Failed to download image:" << reply->errorString();
                // Keep the current image or fallback
            }
            reply->deleteLater();
        });
    } else {
        // Local file path
        QPixmap pixmap(url);
        if (!pixmap.isNull()) {
            m_imageLabel->setPixmap(pixmap);
        } else {
            qDebug() << "Failed to load local image:" << url;
            // Keep the current image or fallback
        }
    }
}

void MediaPlayer::startImageTimer(int durationMs)
{
    if (durationMs <= 0) {
        qDebug() << "Invalid image duration:" << durationMs;
        durationMs = 5000; // Default to 5 seconds
    }
    
    qDebug() << "Starting image timer for" << durationMs << "ms";
    m_imageTimer->start(durationMs);
}

void MediaPlayer::onImageTimerFinished()
{
    qDebug() << "Image timer finished, moving to next";
    next();
}

void MediaPlayer::onVideoFinished()
{
    qDebug() << "Video finished, moving to next";
    next();
}

void MediaPlayer::onVideoStateChanged(QMediaPlayer::PlaybackState state)
{
    qDebug() << "Video state changed:" << state;
    
    if (state == QMediaPlayer::StoppedState) {
        // Video was stopped (could be due to error or end of media)
        // The mediaStatusChanged signal will handle EndOfMedia case
    }
}