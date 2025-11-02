#include "statusbar.h"
#include "mainwindow.h"
#include "qt6compat.h"
#include "logger.h"
#include <QDateTime>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>
#include <QGuiApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>

StatusBar::StatusBar(QWidget *parent)
    : QWidget(parent)
    , m_connected(false)
    , m_pingMs(-1)
    , m_hostname(QString())
    , m_hwDecode(false)
    , m_offlineMode(false)
    , m_timeTimer(new QTimer(this))
    , m_contextMenu(new QMenu(this))
{
    setupUI();

    // Update time every second
    connect(m_timeTimer, &QTimer::timeout, this, &StatusBar::updateTime);
    m_timeTimer->start(1000);

    updateTime();
}

StatusBar::~StatusBar()
{
}

void StatusBar::setupUI()
{
    // Get screen DPI for scaling (supports forced DPI for testing)
    qreal dpi = MainWindow::getDpiForScreen(this);
    
    // Scale factors based on DPI (96 DPI = 100%, 192 DPI = 200%)
    qreal scaleFactor = dpi / 96.0;
    
    // Scale dimensions
    int barHeight = qRound(32 * scaleFactor);
    int iconSize = qRound(12 * scaleFactor);
    int margin = qRound(12 * scaleFactor);
    int smallMargin = qRound(4 * scaleFactor);
    int spacing = qRound(12 * scaleFactor);
    int fontSize = qRound(11 * scaleFactor);
    
    setFixedHeight(barHeight);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(margin, smallMargin, margin, smallMargin);
    layout->setSpacing(spacing);

    // Connection status
    m_connectionIcon = new QLabel("●");
    m_connectionIcon->setFixedSize(iconSize, iconSize);
    m_connectionText = new QLabel("Disconnected");

    // Server info
    m_serverLabel = new QLabel("No server");

    // Ping
    m_pingLabel = new QLabel("Ping: --");
    
    // Codec info
    m_codecLabel = new QLabel("Codec: --");
    
    // Hardware decode indicator
    m_hwDecodeIcon = new QLabel("");
    m_hwDecodeIcon->setToolTip("Hardware decode status");
    m_hwDecodeIcon->setFixedWidth(qRound(16 * scaleFactor)); // Fixed width for icon
    
    // Cache info
    m_cacheLabel = new QLabel("Cache: --");
    m_cacheLabel->setToolTip("Cache hit rate");
    
    // Offline mode indicator
    m_offlineIcon = new QLabel("");
    m_offlineIcon->setFixedWidth(qRound(16 * scaleFactor)); // Fixed width for icon
    m_offlineIcon->setVisible(false); // Hidden by default

    // Version - show full version info
    m_versionLabel = new QLabel(QString("v%1").arg(APP_VERSION));
    m_versionLabel->setToolTip(QString("Build: %1 (%2)").arg(APP_BUILD_ID).arg(APP_RELEASE_DATE));

    // Time (right-aligned)
    m_timeLabel = new QLabel();
    m_timeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // Add widgets to layout
    layout->addWidget(m_connectionIcon);
    layout->addWidget(m_connectionText);
    layout->addWidget(m_serverLabel);
    layout->addWidget(m_pingLabel);
    layout->addWidget(m_codecLabel);
    layout->addWidget(m_hwDecodeIcon);
    layout->addWidget(m_cacheLabel);
    layout->addWidget(m_offlineIcon);
    layout->addWidget(m_versionLabel);
    layout->addStretch();
    layout->addWidget(m_timeLabel);

    // Style with scaled font
    QString style = QString(
        "QLabel {"
        "color: %1;"
        "font-size: %2px;"
        "font-weight: 500;"
        "}"
    ).arg(MD3Colors::DarkTheme::onSurface().name())
     .arg(fontSize);

    setStyleSheet(style);
    updateConnectionIcon();
    
    // Setup context menu
    QAction *diagAction = m_contextMenu->addAction("Toggle Diagnostics (F12)");
    connect(diagAction, &QAction::triggered, this, &StatusBar::toggleDiagnostics);
    
    m_contextMenu->addSeparator();
    
    QMenu *logMenu = m_contextMenu->addMenu("Log Level");
    QAction *errorAction = logMenu->addAction("Error");
    QAction *warnAction = logMenu->addAction("Warning");
    QAction *infoAction = logMenu->addAction("Info");
    QAction *debugAction = logMenu->addAction("Debug");
    
    connect(errorAction, &QAction::triggered, this, [this]() { emit logLevelChangeRequested("error"); });
    connect(warnAction, &QAction::triggered, this, [this]() { emit logLevelChangeRequested("warning"); });
    connect(infoAction, &QAction::triggered, this, [this]() { emit logLevelChangeRequested("info"); });
    connect(debugAction, &QAction::triggered, this, [this]() { emit logLevelChangeRequested("debug"); });
}

void StatusBar::setConnectionStatus(bool connected, const QString &serverUrl, const QString &hostname)
{
    m_connected = connected;
    m_serverUrl = serverUrl;
    m_hostname = hostname;
    updateConnectionIcon();

    if (connected) {
        m_connectionText->setText("Connected");
        if (!hostname.isEmpty()) {
            m_serverLabel->setText(hostname);
        } else if (!serverUrl.isEmpty()) {
            // Fallback to server URL if hostname not available
            m_serverLabel->setText(serverUrl);
        }
    } else {
        m_connectionText->setText("Disconnected");
        m_serverLabel->setText("No server");
        m_pingMs = -1;
        updatePingDisplay();
    }
}

void StatusBar::setPing(int pingMs)
{
    m_pingMs = pingMs;
    updatePingDisplay();
}

void StatusBar::updateTime()
{
    m_timeLabel->setText(QDateTime::currentDateTime().toString("HH:mm:ss"));
}

void StatusBar::updateConnectionIcon()
{
    if (m_connected) {
        m_connectionIcon->setStyleSheet("color: #4CAF50;"); // Green
    } else {
        m_connectionIcon->setStyleSheet("color: #F44336;"); // Red
    }
}

void StatusBar::updatePingDisplay()
{
    if (m_pingMs >= 0) {
        QString color;
        if (m_pingMs < 50) {
            color = "#4CAF50"; // Green for good ping
        } else if (m_pingMs < 150) {
            color = "#FF9800"; // Orange for medium ping
        } else {
            color = "#F44336"; // Red for high ping
        }
        m_pingLabel->setText(QString("Ping: %1ms").arg(m_pingMs));
        m_pingLabel->setStyleSheet(QString("color: %1;").arg(color));
    } else {
        m_pingLabel->setText("Ping: --");
        m_pingLabel->setStyleSheet("color: #666666;");
    }
}

void StatusBar::setCodecInfo(const QString &codec, bool hwDecode)
{
    m_codec = codec;
    m_hwDecode = hwDecode;
    updateCodecDisplay();
}

void StatusBar::updateCodecDisplay()
{
    if (!m_codec.isEmpty() && m_codec != "unknown") {
        m_codecLabel->setText(QString("Codec: %1").arg(m_codec));
    } else {
        m_codecLabel->setText("Codec: --");
    }
    
    if (m_hwDecode) {
        m_hwDecodeIcon->setText("⚡");
        m_hwDecodeIcon->setStyleSheet("color: #4CAF50;");
        m_hwDecodeIcon->setToolTip("Hardware decode: ON");
    } else {
        m_hwDecodeIcon->setText("🐌");
        m_hwDecodeIcon->setStyleSheet("color: #FF9800;");
        m_hwDecodeIcon->setToolTip("Hardware decode: OFF (Software)");
    }
}

void StatusBar::setCacheStats(const CacheStats &stats)
{
    m_cacheStats = stats;
    updateCacheDisplay();
}

void StatusBar::updateCacheDisplay()
{
    if (m_cacheStats.hits + m_cacheStats.misses > 0) {
        double hitRate = m_cacheStats.hitRate();
        QString color;
        if (hitRate >= 70.0) {
            color = "#4CAF50"; // Green
        } else if (hitRate >= 40.0) {
            color = "#FF9800"; // Orange
        } else {
            color = "#F44336"; // Red
        }
        
        m_cacheLabel->setText(QString("Cache: %1%").arg(hitRate, 0, 'f', 0));
        m_cacheLabel->setStyleSheet(QString("color: %1;").arg(color));
        m_cacheLabel->setToolTip(QString("Cache: %1 hits, %2 misses\nSize: %3 / %4 items")
            .arg(m_cacheStats.hits)
            .arg(m_cacheStats.misses)
            .arg(m_cacheStats.totalSize / (1024 * 1024))
            .arg(m_cacheStats.itemCount));
    } else {
        m_cacheLabel->setText("Cache: --");
        m_cacheLabel->setStyleSheet("");
        m_cacheLabel->setToolTip("Cache statistics");
    }
}

void StatusBar::setOfflineMode(bool offline)
{
    m_offlineMode = offline;
    
    if (offline) {
        m_offlineIcon->setText("📡");
        m_offlineIcon->setStyleSheet("color: #FF9800;");
        m_offlineIcon->setToolTip("Offline mode: Playing cached content");
        m_offlineIcon->setVisible(true);
    } else {
        m_offlineIcon->setText("");
        m_offlineIcon->setToolTip("");
        m_offlineIcon->setVisible(false);
    }
}

void StatusBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        showContextMenu(event->pos());
    }
    QWidget::mousePressEvent(event);
}

void StatusBar::showContextMenu(const QPoint &pos)
{
    m_contextMenu->exec(mapToGlobal(pos));
}