#include "videowidget.h"
#include "md3colors.h"
#include <QPaintEvent>
#include <QFont>
#include <QFontDatabase>
#include <QPainterPath>
#include <QResizeEvent>
#include <QHBoxLayout>
#include <QSpacerItem>

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent), m_hasVideo(false)
{
    // Set minimum height following MD3 guidelines
    setMinimumHeight(480);
    
    // Apply MD3 surface container styling with elevation
    QString videoStyle = QString(
        "VideoWidget {"
            "background-color: %1;"
            "border-radius: %2px;"
            "border: 1px solid %3;"
        "}"
    ).arg(MD3Colors::DarkTheme::surfaceContainerHigh().name())
     .arg(MD3Shapes::cornerLarge())
     .arg(MD3Colors::DarkTheme::outline().name());
    
    setStyleSheet(videoStyle);
    
    // Initialize multimedia components
    m_mediaPlayer = new QMediaPlayer(this);
    m_videoWidget = new QVideoWidget(this);
    
    // Connect the media player to the video widget
    m_mediaPlayer->setVideoOutput(m_videoWidget);
    
    // Setup UI
    setupUI();
    
    // Connect signals
    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged,
            this, &VideoWidget::onMediaStatusChanged);
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged,
            this, &VideoWidget::onPlaybackStateChanged);
    
    // Do not load any video by default
    m_hasVideo = false;
}

void VideoWidget::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(MD3Spacing::spacing4());
    m_layout->setContentsMargins(
        MD3Spacing::spacing4(),
        MD3Spacing::spacing4(),
        MD3Spacing::spacing4(),
        MD3Spacing::spacing4()
    );
    
    // Add video widget
    m_layout->addWidget(m_videoWidget, 1);
    
    // Create control layout
    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->setSpacing(MD3Spacing::spacing4());
    
    // Status label
    m_statusLabel = new QLabel("Loading video...", this);
    m_statusLabel->setStyleSheet(QString(
        "QLabel {"
            "color: %1;"
            "font-size: %2px;"
            "background: transparent;"
        "}"
    ).arg(MD3Colors::DarkTheme::onSurface().name())
     .arg(MD3Typography::BodyMedium::size()));
    
    // Play button
    m_playButton = new QPushButton("▶ Play", this);
    m_playButton->setStyleSheet(QString(
        "QPushButton {"
            "background-color: %1;"
            "color: %2;"
            "border-radius: %3px;"
            "padding: %4px %5px;"
            "font-size: %6px;"
            "font-weight: %7;"
            "border: none;"
        "}"
        "QPushButton:hover {"
            "background-color: %8;"
        "}"
        "QPushButton:pressed {"
            "background-color: %9;"
        "}"
    ).arg(MD3Colors::DarkTheme::primary().name())
     .arg(MD3Colors::DarkTheme::onPrimary().name())
     .arg(MD3Shapes::cornerMedium())
     .arg(MD3Spacing::spacing3())
     .arg(MD3Spacing::spacing6())
     .arg(MD3Typography::LabelLarge::size())
     .arg(MD3Typography::LabelLarge::weight())
     .arg(MD3Colors::DarkTheme::primary().lighter(110).name())
     .arg(MD3Colors::DarkTheme::primary().darker(110).name()));
    
    connect(m_playButton, &QPushButton::clicked, this, &VideoWidget::playPause);
    
    // Add controls to layout
    controlLayout->addWidget(m_statusLabel, 1);
    controlLayout->addWidget(m_playButton, 0);
    
    m_layout->addLayout(controlLayout, 0);
    
    setLayout(m_layout);
}

void VideoWidget::setVideoUrl(const QString &url)
{
    m_videoUrl = url;
    m_statusLabel->setText("Loading: " + url);
    
    QUrl videoUrl(url);
    m_mediaPlayer->setSource(videoUrl);
}

void VideoWidget::onMediaReceived(const MediaInfo &media)
{
    if (media.type == "video") {
        setVideoUrl(media.url);
        m_mediaPlayer->play();
        m_hasVideo = true;
    } else {
        // Handle other media types like 'image' in the future
        m_statusLabel->setText("Received non-video media: " + media.type);
    }
}
void VideoWidget::playPause()
{
    if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        m_mediaPlayer->pause();
    } else {
        m_mediaPlayer->play();
    }
}

void VideoWidget::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    switch (status) {
    case QMediaPlayer::LoadedMedia:
        m_statusLabel->setText("Ready to play");
        break;
    case QMediaPlayer::LoadingMedia:
        m_statusLabel->setText("Loading media...");
        break;
    case QMediaPlayer::BufferingMedia:
        m_statusLabel->setText("Buffering...");
        break;
    case QMediaPlayer::EndOfMedia:
        m_statusLabel->setText("Finished");
        break;
    case QMediaPlayer::InvalidMedia:
        m_statusLabel->setText("Error: Cannot load media");
        break;
    default:
        break;
    }
    updatePlayButton();
}

void VideoWidget::onPlaybackStateChanged(QMediaPlayer::PlaybackState state)
{
    updatePlayButton();
    
    switch (state) {
    case QMediaPlayer::PlayingState:
        m_statusLabel->setText("Playing");
        break;
    case QMediaPlayer::PausedState:
        m_statusLabel->setText("Paused");
        break;
    case QMediaPlayer::StoppedState:
        m_statusLabel->setText("Stopped");
        break;
    }
}

void VideoWidget::updatePlayButton()
{
    if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        m_playButton->setText("⏸ Pause");
    } else {
        m_playButton->setText("▶ Play");
    }
}

void VideoWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}

void VideoWidget::paintEvent(QPaintEvent *event)
{
    // Let the base widget handle painting for the layout
    QWidget::paintEvent(event);
}

void VideoWidget::drawPlayIcon(QPainter &painter, const QRect &rect)
{
    // This method is kept for compatibility but not used with real video playback
    Q_UNUSED(painter)
    Q_UNUSED(rect)
}
