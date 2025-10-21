#include "videowidget.h"
#include "mediaplayer.h"
#include <QtMultimedia>
#include <QtMultimediaWidgets>
#include <QLabel>
#include <QStackedLayout>
#include <QPixmap>
#include <QDebug>

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent)
{
    // Use a black background, no borders or rounded corners
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);
    setStyleSheet("border: none;");

    // --- Video Widget for Videos ---
    m_videoOutput = new QVideoWidget(this);

    // --- Fallback Image Label ---
    m_fallbackLabel = new QLabel(this);
    m_fallbackLabel->setAlignment(Qt::AlignCenter);
    m_fallbackLabel->setScaledContents(true); // Scale image to fill the widget
    QPixmap fallbackPixmap("media/default.jpeg");
    if(fallbackPixmap.isNull()) {
        qDebug() << "ERROR: Could not load fallback image from media/default.jpeg";
        m_fallbackLabel->setText("Fallback image not found!");
    } else {
        m_fallbackLabel->setPixmap(fallbackPixmap);
    }

    // --- Layout to Switch Between Video and Image ---
    m_mainLayout = new QStackedLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(m_videoOutput);   // Index 0
    m_mainLayout->addWidget(m_fallbackLabel); // Index 1
    setLayout(m_mainLayout);

    // IMPORTANT: Start by showing the fallback image
    m_mainLayout->setCurrentWidget(m_fallbackLabel);

    // --- Initialize Media Player ---
    m_mediaPlayer = new MediaPlayer(m_videoOutput, m_fallbackLabel, m_mainLayout, this);
    connect(m_mediaPlayer, &MediaPlayer::mediaChanged, this, &VideoWidget::onMediaChanged);
}

void VideoWidget::onPlaylistReceived(const MediaPlaylist &playlist)
{
    qDebug() << "Playlist received with" << playlist.items.size() << "items";
    m_mediaPlayer->setPlaylist(playlist);
    m_mediaPlayer->play();
}

void VideoWidget::onMediaChanged(const MediaItem &item)
{
    qDebug() << "Current media changed to:" << item.type << item.url;
    // The MediaPlayer handles switching between video and image views
}

void VideoWidget::onNetworkError(const QString &error)
{
    qDebug() << "VideoWidget received network error:" << error;
    m_mediaPlayer->stop();
    m_mainLayout->setCurrentWidget(m_fallbackLabel); // Show the fallback image on any error
}
