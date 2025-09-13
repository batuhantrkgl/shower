#include "videowidget.h"
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

    // --- Media Player for Videos ---
    m_player = new QMediaPlayer(this);
    m_videoOutput = new QVideoWidget(this);
    m_player->setVideoOutput(m_videoOutput);
    
    // Log any media player errors
    connect(m_player, &QMediaPlayer::errorOccurred, this, [=](QMediaPlayer::Error error, const QString &errorString){
        qDebug() << "MediaPlayer Error:" << error << errorString;
    });

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
}

void VideoWidget::onMediaReceived(const MediaInfo &media)
{
    qDebug() << "Media received:" << media.type << media.url;
    if (media.type == "video" && !media.url.isEmpty()) {
        m_player->setSource(QUrl(media.url));
        m_mainLayout->setCurrentWidget(m_videoOutput); // Show the video player
        m_player->play(); // Autoplay!
    } else {
        // If media type is not video or URL is empty, show fallback
        onNetworkError("Invalid or non-video media received");
    }
}

void VideoWidget::onNetworkError(const QString &error)
{
    qDebug() << "VideoWidget received network error:" << error;
    m_player->stop();
    m_mainLayout->setCurrentWidget(m_fallbackLabel); // Show the fallback image on any error
}
