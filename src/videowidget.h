#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QResizeEvent>
#include "networkclient.h" // For the MediaItem struct

// Forward declarations
class QVideoWidget;
class QLabel;
class QStackedLayout;
class MediaPlayer;

class VideoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = nullptr);

public slots:
    // Slot to handle successfully fetched playlist from the server
    void onPlaylistReceived(const MediaPlaylist &playlist);
    // Slot to handle any network failure
    void onNetworkError(const QString &error);
    // Slot to handle media changes in the playlist
    void onMediaChanged(const MediaItem &item);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QVideoWidget *m_videoOutput;
    QLabel *m_fallbackLabel;
    QStackedLayout *m_mainLayout;
    MediaPlayer *m_mediaPlayer;
};

#endif // VIDEOWIDGET_H
