#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include "md3colors.h"
#include "networkclient.h"

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    void setVideoUrl(const QString &url);

public slots:
    void playPause();
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onMediaReceived(const MediaInfo &media);
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);

protected:
    void paintEvent(QPaintEvent *event) override;
    void drawPlayIcon(QPainter &painter, const QRect &rect);
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();
    void updatePlayButton();
    
    QMediaPlayer *m_mediaPlayer;
    QVideoWidget *m_videoWidget;
    QVBoxLayout *m_layout;
    QLabel *m_statusLabel;
    QPushButton *m_playButton;
    QString m_videoUrl;
    bool m_hasVideo;

};

#endif // VIDEOWIDGET_H
