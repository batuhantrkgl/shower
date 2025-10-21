#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>
#include <QTimer>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QLabel>
#include <QStackedLayout>
#include <QAudioOutput>
#include <QString>
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

    QMediaPlayer *m_player;
    QVideoWidget *m_videoOutput;
    QLabel *m_imageLabel;
    QStackedLayout *m_layout;
    
    MediaPlaylist m_playlist;
    QTimer *m_imageTimer;
    
    bool m_isPlaying;
    
    // Widget indices for stacked layout
    static const int VIDEO_INDEX = 0;
    static const int IMAGE_INDEX = 1;
};

#endif // MEDIAPLAYER_H