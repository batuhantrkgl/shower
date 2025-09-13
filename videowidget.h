#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include "networkclient.h" // For the MediaInfo struct

// Forward declarations
class QMediaPlayer;
class QVideoWidget;
class QLabel;
class QStackedLayout;

class VideoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = nullptr);

public slots:
    // Slot to handle successfully fetched media info from the server
    void onMediaReceived(const MediaInfo &media);
    // Slot to handle any network failure
    void onNetworkError(const QString &error);

private:
    QMediaPlayer *m_player;
    QVideoWidget *m_videoOutput;
    QLabel *m_fallbackLabel;
    QStackedLayout *m_mainLayout;
};

#endif // VIDEOWIDGET_H
