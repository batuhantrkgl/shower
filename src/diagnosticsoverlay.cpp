#include "diagnosticsoverlay.h"
#include "md3colors.h"
#include "qt6compat.h"
#include <QKeyEvent>
#include <QApplication>
#include <QScreen>

DiagnosticsOverlay::DiagnosticsOverlay(QWidget *parent)
    : QWidget(parent)
    , m_updateTimer(new QTimer(this))
{
    setupUI();
    
    // Auto-hide initially
    hide();
    
    // Update display every second
    connect(m_updateTimer, &QTimer::timeout, this, &DiagnosticsOverlay::updateDisplay);
    m_updateTimer->start(1000);
}

DiagnosticsOverlay::~DiagnosticsOverlay()
{
}

void DiagnosticsOverlay::setupUI()
{
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, false);
    
    // Semi-transparent dark background
    setStyleSheet(QString(
        "QWidget {"
        "    background-color: rgba(0, 0, 0, 200);"
        "    color: %1;"
        "    font-family: 'SF Pro Display', 'Segoe UI', sans-serif;"
        "}"
        "QLabel {"
        "    padding: 2px;"
        "}"
    ).arg(MD3Colors::DarkTheme::onSurface().name()));
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Title
    m_titleLabel = new QLabel("📊 Diagnostics (Press F12 to close)");
    QFont titleFont;
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet("color: #4CAF50;");
    mainLayout->addWidget(m_titleLabel);
    
    // Grid layout for info
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(8);
    int row = 0;
    
    // --- Network Section ---
    QLabel *networkHeader = new QLabel("🌐 Network");
    QFont headerFont;
    headerFont.setPointSize(12);
    headerFont.setBold(true);
    networkHeader->setFont(headerFont);
    networkHeader->setStyleSheet("color: #2196F3;");
    gridLayout->addWidget(networkHeader, row++, 0, 1, 2);
    
    gridLayout->addWidget(new QLabel("Server URL:"), row, 0);
    m_serverUrlLabel = new QLabel("--");
    gridLayout->addWidget(m_serverUrlLabel, row++, 1);
    
    gridLayout->addWidget(new QLabel("Hostname:"), row, 0);
    m_hostnameLabel = new QLabel("--");
    gridLayout->addWidget(m_hostnameLabel, row++, 1);
    
    gridLayout->addWidget(new QLabel("Connection:"), row, 0);
    m_connectionLabel = new QLabel("--");
    gridLayout->addWidget(m_connectionLabel, row++, 1);
    
    gridLayout->addWidget(new QLabel("Ping:"), row, 0);
    m_pingLabel = new QLabel("--");
    gridLayout->addWidget(m_pingLabel, row++, 1);
    
    // --- Media Section ---
    row++;
    QLabel *mediaHeader = new QLabel("🎬 Media");
    mediaHeader->setFont(headerFont);
    mediaHeader->setStyleSheet("color: #FF9800;");
    gridLayout->addWidget(mediaHeader, row++, 0, 1, 2);
    
    gridLayout->addWidget(new QLabel("Current Source:"), row, 0);
    m_sourceLabel = new QLabel("--");
    m_sourceLabel->setWordWrap(true);
    gridLayout->addWidget(m_sourceLabel, row++, 1);
    
    gridLayout->addWidget(new QLabel("Codec:"), row, 0);
    m_codecLabel = new QLabel("--");
    gridLayout->addWidget(m_codecLabel, row++, 1);
    
    gridLayout->addWidget(new QLabel("HW Decode:"), row, 0);
    m_hwDecodeLabel = new QLabel("--");
    gridLayout->addWidget(m_hwDecodeLabel, row++, 1);
    
    gridLayout->addWidget(new QLabel("Resolution:"), row, 0);
    m_resolutionLabel = new QLabel("--");
    gridLayout->addWidget(m_resolutionLabel, row++, 1);
    
    gridLayout->addWidget(new QLabel("FPS:"), row, 0);
    m_fpsLabel = new QLabel("--");
    gridLayout->addWidget(m_fpsLabel, row++, 1);
    
    gridLayout->addWidget(new QLabel("Status:"), row, 0);
    m_statusLabel = new QLabel("--");
    gridLayout->addWidget(m_statusLabel, row++, 1);
    
    // --- Cache Section ---
    row++;
    QLabel *cacheHeader = new QLabel("💾 Cache");
    cacheHeader->setFont(headerFont);
    cacheHeader->setStyleSheet("color: #9C27B0;");
    gridLayout->addWidget(cacheHeader, row++, 0, 1, 2);
    
    gridLayout->addWidget(new QLabel("Hit Rate:"), row, 0);
    m_cacheHitRateLabel = new QLabel("--");
    gridLayout->addWidget(m_cacheHitRateLabel, row++, 1);
    
    gridLayout->addWidget(new QLabel("Hits / Misses:"), row, 0);
    m_cacheHitsLabel = new QLabel("--");
    gridLayout->addWidget(m_cacheHitsLabel, row++, 1);
    
    gridLayout->addWidget(new QLabel("Cache Size:"), row, 0);
    m_cacheSizeLabel = new QLabel("--");
    gridLayout->addWidget(m_cacheSizeLabel, row++, 1);
    
    gridLayout->addWidget(new QLabel("Cached Items:"), row, 0);
    m_cacheCountLabel = new QLabel("--");
    gridLayout->addWidget(m_cacheCountLabel, row++, 1);
    
    // --- System Section ---
    row++;
    QLabel *systemHeader = new QLabel("ℹ️ System");
    systemHeader->setFont(headerFont);
    systemHeader->setStyleSheet("color: #00BCD4;");
    gridLayout->addWidget(systemHeader, row++, 0, 1, 2);
    
    gridLayout->addWidget(new QLabel("Version:"), row, 0);
    m_versionLabel = new QLabel("--");
    gridLayout->addWidget(m_versionLabel, row++, 1);
    
    gridLayout->addWidget(new QLabel("Build:"), row, 0);
    m_buildLabel = new QLabel("--");
    gridLayout->addWidget(m_buildLabel, row++, 1);
    
    mainLayout->addLayout(gridLayout);
    mainLayout->addStretch();
    
    // Set reasonable size
    setMinimumSize(500, 600);
    resize(550, 700);
}

void DiagnosticsOverlay::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    
    if (visible) {
        // Center on screen
        QScreen *screen = QApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->geometry();
            int x = (screenGeometry.width() - width()) / 2;
            int y = (screenGeometry.height() - height()) / 2;
            move(x, y);
        }
        
        raise();
        activateWindow();
        updateDisplay();
    }
}

void DiagnosticsOverlay::updateInfo(const DiagnosticsInfo &info)
{
    m_info = info;
    updateDisplay();
}

void DiagnosticsOverlay::setServerInfo(const QString &url, const QString &hostname, int pingMs, bool connected)
{
    m_info.serverUrl = url;
    m_info.hostname = hostname;
    m_info.pingMs = pingMs;
    m_info.connected = connected;
}

void DiagnosticsOverlay::setMediaInfo(const QString &codec, bool hwDecode, const QString &resolution, qreal fps)
{
    m_info.currentCodec = codec;
    m_info.hardwareDecodeEnabled = hwDecode;
    m_info.resolution = resolution;
    m_info.fps = fps;
}

void DiagnosticsOverlay::setCurrentSource(const QString &source)
{
    m_info.currentItemSource = source;
}

void DiagnosticsOverlay::setCacheStats(const CacheStats &stats)
{
    m_info.cacheHits = stats.hits;
    m_info.cacheMisses = stats.misses;
    m_info.cacheHitRate = stats.hitRate();
    m_info.cacheSize = stats.totalSize;
    m_info.cacheItemCount = stats.itemCount;
}

void DiagnosticsOverlay::setMediaStatus(QMediaPlayer::MediaStatus status)
{
    m_info.mediaStatus = status;
}

void DiagnosticsOverlay::updateDisplay()
{
    // Network
    m_serverUrlLabel->setText(m_info.serverUrl.isEmpty() ? "Not set" : m_info.serverUrl);
    m_hostnameLabel->setText(m_info.hostname.isEmpty() ? "Unknown" : m_info.hostname);
    
    if (m_info.connected) {
        m_connectionLabel->setText("✅ Connected");
        m_connectionLabel->setStyleSheet("color: #4CAF50;");
    } else {
        m_connectionLabel->setText("❌ Disconnected");
        m_connectionLabel->setStyleSheet("color: #F44336;");
    }
    
    if (m_info.pingMs >= 0) {
        QString color;
        if (m_info.pingMs < 50) color = "#4CAF50";
        else if (m_info.pingMs < 150) color = "#FF9800";
        else color = "#F44336";
        m_pingLabel->setText(QString("%1 ms").arg(m_info.pingMs));
        m_pingLabel->setStyleSheet(QString("color: %1;").arg(color));
    } else {
        m_pingLabel->setText("--");
        m_pingLabel->setStyleSheet("");
    }
    
    // Media
    m_sourceLabel->setText(m_info.currentItemSource.isEmpty() ? "None" : m_info.currentItemSource);
    m_codecLabel->setText(m_info.currentCodec.isEmpty() ? "Unknown" : m_info.currentCodec);
    
    if (m_info.hardwareDecodeEnabled) {
        m_hwDecodeLabel->setText("✅ Enabled");
        m_hwDecodeLabel->setStyleSheet("color: #4CAF50;");
    } else {
        m_hwDecodeLabel->setText("❌ Disabled (Software)");
        m_hwDecodeLabel->setStyleSheet("color: #FF9800;");
    }
    
    m_resolutionLabel->setText(m_info.resolution.isEmpty() ? "--" : m_info.resolution);
    m_fpsLabel->setText(m_info.fps > 0 ? QString::number(m_info.fps, 'f', 2) : "--");
    m_statusLabel->setText(getMediaStatusString(m_info.mediaStatus));
    
    // Cache
    m_cacheHitRateLabel->setText(QString("%1%").arg(m_info.cacheHitRate, 0, 'f', 1));
    m_cacheHitsLabel->setText(QString("%1 / %2").arg(m_info.cacheHits).arg(m_info.cacheMisses));
    m_cacheSizeLabel->setText(formatSize(m_info.cacheSize));
    m_cacheCountLabel->setText(QString::number(m_info.cacheItemCount));
    
    // System
    m_versionLabel->setText(m_info.appVersion.isEmpty() ? APP_VERSION : m_info.appVersion);
    m_buildLabel->setText(m_info.buildId.isEmpty() ? 
        QString("%1 (%2)").arg(APP_BUILD_ID).arg(APP_RELEASE_DATE) : 
        QString("%1 (%2)").arg(m_info.buildId).arg(m_info.buildDate));
}

void DiagnosticsOverlay::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F12 || event->key() == Qt::Key_Escape) {
        hide();
        event->accept();
    } else {
        QWidget::keyPressEvent(event);
    }
}

QString DiagnosticsOverlay::formatSize(qint64 bytes) const
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (bytes >= GB) {
        return QString("%1 GB").arg(bytes / static_cast<double>(GB), 0, 'f', 2);
    } else if (bytes >= MB) {
        return QString("%1 MB").arg(bytes / static_cast<double>(MB), 0, 'f', 2);
    } else if (bytes >= KB) {
        return QString("%1 KB").arg(bytes / static_cast<double>(KB), 0, 'f', 2);
    } else {
        return QString("%1 bytes").arg(bytes);
    }
}

QString DiagnosticsOverlay::getMediaStatusString(QMediaPlayer::MediaStatus status) const
{
    switch (status) {
        case QMediaPlayer::NoMedia: return "No Media";
        case QMediaPlayer::LoadingMedia: return "Loading...";
        case QMediaPlayer::LoadedMedia: return "Loaded";
        case QMediaPlayer::StalledMedia: return "⚠️ Stalled";
        case QMediaPlayer::BufferingMedia: return "⏳ Buffering...";
        case QMediaPlayer::BufferedMedia: return "✅ Buffered";
        case QMediaPlayer::EndOfMedia: return "End of Media";
        case QMediaPlayer::InvalidMedia: return "❌ Invalid Media";
        default: return "Unknown";
    }
}
