#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>
#include <QTimer>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QLabel>
#include <QStackedLayout>
#include <QString>
#include "qt6compat.h"
#include "networkclient.h"

class MediaPlayer : public QObject
{
    Q_OBJECT

public:
    explicit MediaPlayer(QVideoWidget *videoOutput, QLabel *imageLabel, QStackedLayout *layout, QObject *parent = nullptr);
    
    void setPlaylist(const MediaPlaylist &playlist);
    void play();
    void stop();
    void next();
    void rescaleCurrentImage();

signals:
    void mediaChanged(const MediaItem &item);
    void playlistFinished();

private slots:
    void onImageTimerFinished();
    void onVideoFinished();
    void onVideoStateChanged(QMediaPlayer::PlaybackState state);

private:
    void playCurrentItem();
    void showVideo();
    void showImage();
    void loadImage(const QString &url);
    void startImageTimer(int durationMs);
    void scaleAndSetImage(const QPixmap &originalPixmap);

    QMediaPlayer *m_player;
    QVideoWidget *m_videoOutput;
    QLabel *m_imageLabel;
    QStackedLayout *m_layout;
    
    MediaPlaylist m_playlist;
    QTimer *m_imageTimer;
    QPixmap m_currentImage; // Store original image for rescaling
    
    bool m_isPlaying;
    
    // Widget indices for stacked layout (using compatibility constants)
    static const int VIDEO_INDEX = VIDEO_WIDGET_INDEX;
    static const int IMAGE_INDEX = IMAGE_WIDGET_INDEX;
};

#endif // MEDIAPLAYER_H