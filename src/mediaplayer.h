#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>
#include <QTimer>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QLabel>
#include <QStackedLayout>
#include <QScreen>
#include <QGuiApplication>
#include <QString>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include "qt6compat.h"
#include "networkclient.h"

class MediaCache;

class MediaPlayer : public QObject
{
    Q_OBJECT

public:
    explicit MediaPlayer(QVideoWidget *videoOutput, QLabel *imageLabel, QStackedLayout *layout, QObject *parent = nullptr);
    
    void setPlaylist(const MediaPlaylist &playlist);
    void setMediaCache(MediaCache *cache);
    void play();
    void stop();
    void next();
    void rescaleCurrentImage();
    
    // Transition settings
    void setFadeDuration(int ms) { m_fadeDuration = ms; }
    void enableTransitions(bool enable) { m_transitionsEnabled = enable; }
    
    // Diagnostics
    QString getCurrentCodec() const { return m_currentCodec; }
    bool isHardwareDecodeEnabled() const { return m_hwDecodeEnabled; }
    QString getCurrentResolution() const { return m_currentResolution; }
    qreal getCurrentFps() const { return m_currentFps; }

signals:
    void mediaChanged(const MediaItem &item);
    void playlistFinished();
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void codecDetected(const QString &codec, bool hwDecode);

private slots:
    void onImageTimerFinished();
    void onVideoFinished();
    void onVideoStateChanged(QMediaPlayer::PlaybackState state);
    void onScreenCaptureTimer();
    void onFadeOutFinished();
    void onFadeInFinished();
    void onPrefetchComplete(const QString &url, bool success);

private:
    void playCurrentItem();
    void showVideo();
    void showImage();
    void showScreen();
    void loadImage(const QString &url);
    void startImageTimer(int durationMs);
    void scaleAndSetImage(const QPixmap &originalPixmap);
    void captureScreen();
    void fadeOut();
    void fadeIn();
    void prefetchNextItem();
    void detectMediaProperties();
    void detectImageProperties(const QString &url);

    QMediaPlayer *m_player;
    QVideoWidget *m_videoOutput;
    QLabel *m_imageLabel;
    QLabel *m_screenLabel;
    QStackedLayout *m_layout;
    MediaCache *m_mediaCache;
    
    MediaPlaylist m_playlist;
    QTimer *m_imageTimer;
    QTimer *m_screenTimer;
    QPixmap m_currentImage; // Store original image for rescaling
    
    bool m_isPlaying;
    
    // Transition effects
    QGraphicsOpacityEffect *m_videoOpacity;
    QGraphicsOpacityEffect *m_imageOpacity;
    QGraphicsOpacityEffect *m_screenOpacity;
    QPropertyAnimation *m_fadeAnimation;
    int m_fadeDuration;
    bool m_transitionsEnabled;
    bool m_isFading;
    bool m_waitingForVideoToLoad;  // Flag to delay showing video during fade
    
    // Diagnostics
    QString m_currentCodec;
    bool m_hwDecodeEnabled;
    QString m_currentResolution;
    qreal m_currentFps;
    
    // Widget indices for stacked layout (using compatibility constants)
    static const int VIDEO_INDEX = VIDEO_WIDGET_INDEX;
    static const int IMAGE_INDEX = IMAGE_WIDGET_INDEX;
    static const int SCREEN_INDEX = SCREEN_WIDGET_INDEX;
};

#endif // MEDIAPLAYER_H