#include "mainwindow.h"
#include "videowidget.h"
#include "timelinewidget.h"
#include "statusbar.h"
#include "activityoverlay.h"
#include "md3colors.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QApplication>
#include <QScreen>
#include <QResizeEvent>
#include <QEvent>
#include <iostream>

static qreal s_forcedDpi = 0.0;

MainWindow::MainWindow(bool autoDiscover, const QString &networkRange, qreal forcedDpi, const QString &testTimeStr, QWidget *parent)
    : QMainWindow(parent), m_forcedDpi(forcedDpi)
{
    // Set the global forced DPI for all widgets to use
    s_forcedDpi = forcedDpi;

    // Parse test time if provided
    if (!testTimeStr.isEmpty()) {
        m_testTime = QTime::fromString(testTimeStr, "HH:mm");
        if (m_testTime.isValid()) {
            qDebug() << "[TIME TEST] Using forced test time:" << m_testTime.toString("HH:mm");
        } else {
            qWarning() << "[TIME TEST] Invalid test time format:" << testTimeStr << "(expected HH:mm format)";
        }
    }

    setWindowTitle("Video Timeline");
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
    QWidget *centralWidget = new QWidget(this);
    QString mainStyle = QString(
        "QWidget {"
            "background-color: %1;"
            "color: %2;"
        "}"
    ).arg(MD3Colors::DarkTheme::background().name())
     .arg(MD3Colors::DarkTheme::onBackground().name());
    centralWidget->setStyleSheet(mainStyle);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Initialize network client and discover server if requested
    m_networkClient = new NetworkClient(this);
    if (!networkRange.isEmpty()) {
        // Check if it's a specific server URL (contains port) or network range
        if (networkRange.contains(':')) {
            // Specific server URL
            m_networkClient->setSpecificServer(networkRange);
        } else {
            // Network range to scan
            m_networkClient->discoverInRange(networkRange);
        }
    } else if (autoDiscover) {
        // Auto-discover using default algorithm
        m_networkClient->discoverAndSetServer();
    }
    
    // Create UI widgets
    m_statusBar = new StatusBar(this);
    m_videoWidget = new VideoWidget(this);
    m_timelineWidget = new TimelineWidget(m_networkClient, this);
    
    // Create activity overlay as child of MainWindow (not centralWidget!)
    // Widgets must be children of the main window to float above layout-managed widgets
    m_activityOverlay = new ActivityOverlay(this);
    
    // Don't install event filter on centralWidget anymore
    
    m_activityOverlay->raise();
    m_activityOverlay->show();
    
    std::cout << "ActivityOverlay created and initialized" << std::endl;

    // Add widgets to layout (activity overlay not in layout - it's floating)
    mainLayout->addWidget(m_statusBar, 0);
    mainLayout->addWidget(m_videoWidget, 1);
    mainLayout->addWidget(m_timelineWidget, 0);

    // Connect signals and slots
    connect(m_networkClient, &NetworkClient::playlistReceived,
            m_videoWidget, &VideoWidget::onPlaylistReceived);
    connect(m_networkClient, &NetworkClient::scheduleReceived,
            this, &MainWindow::onScheduleReceived);
    connect(m_networkClient, &NetworkClient::networkError,
            m_videoWidget, &VideoWidget::onNetworkError);
    connect(m_networkClient, &NetworkClient::connectionStatusChanged,
            m_statusBar, &StatusBar::setConnectionStatus);
    connect(m_networkClient, &NetworkClient::pingUpdated,
            m_statusBar, &StatusBar::setPing);
    connect(m_timelineWidget, &TimelineWidget::currentActivityChanged,
            m_activityOverlay, &ActivityOverlay::updateCurrentActivity);
    
    // Reposition overlay when its content changes (and thus its size)
    connect(m_activityOverlay, &QWidget::destroyed, this, [this]() { m_activityOverlay = nullptr; });
    
    // Ensure overlay stays visible when media changes
    connect(m_videoWidget, &VideoWidget::mediaChanged,
            this, &MainWindow::onMediaChanged);

    // Setup update timer
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::updateUIState);
    m_updateTimer->start(1000);

    // Start network polling
    m_networkClient->startPeriodicFetch();
    
    // Configure window
    setCentralWidget(centralWidget);
    QScreen *primaryScreen = QApplication::primaryScreen();
    if (primaryScreen) {
        setGeometry(primaryScreen->geometry());
        showFullScreen();
        
        // Position the activity overlay over the video widget
        QTimer::singleShot(100, this, &MainWindow::positionActivityOverlay);
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::onScheduleReceived(const QTime &schoolStart, const QTime &schoolEnd, const QList<ScheduleBlock> &schedule)
{
    Q_UNUSED(schedule);
    m_schoolStartTime = schoolStart;
    m_schoolEndTime = schoolEnd;
    m_scheduleLoaded = true;
    updateUIState();
}

void MainWindow::updateUIState()
{
    if (!m_scheduleLoaded) {
        m_timelineWidget->updateCurrentTime(QTime());
        return;
    }

    QTime currentTime = m_testTime.isValid() ? m_testTime : QTime::currentTime();

    if (currentTime < m_schoolStartTime || currentTime > m_schoolEndTime) {
        m_timelineWidget->updateCurrentTime(QTime());
    } else {
        m_timelineWidget->updateCurrentTime(currentTime);
    }
    
    // Ensure overlay stays positioned correctly
    if (m_activityOverlay && m_activityOverlay->isVisible()) {
        QTimer::singleShot(0, this, &MainWindow::positionActivityOverlay);
    }
}

qreal MainWindow::getDpiForScreen(QWidget *widget)
{
    // If forced DPI is set (for testing), use it
    if (s_forcedDpi > 0.0) {
        // Only show debug output once to avoid spam
        static bool shownDebug = false;
        if (!shownDebug) {
            qDebug() << "[DPI TEST] Using forced DPI:" << s_forcedDpi << "DPI (scale factor:" << (s_forcedDpi / 96.0) << ")";
            shownDebug = true;
        }
        return s_forcedDpi;
    }
    
    // Otherwise get actual screen DPI
    QScreen *screen = nullptr;
    if (widget) {
        screen = widget->screen();
    } else {
        // Fallback to primary screen
        screen = QApplication::primaryScreen();
    }
    
    return screen ? screen->logicalDotsPerInch() : 96.0;
}

void MainWindow::positionActivityOverlay()
{
    if (!m_activityOverlay || !m_videoWidget) {
        std::cout << "positionActivityOverlay: overlay or videoWidget is null" << std::endl;
        return;
    }

    // Overlay is now a top-level window, so we need global screen coordinates
    QPoint videoGlobalPos = m_videoWidget->mapToGlobal(QPoint(0, 0));
    QSize videoSize = m_videoWidget->size();
    
    int overlayWidth = m_activityOverlay->width();
    int overlayHeight = m_activityOverlay->height();
    
    // Center horizontally within video widget area
    int overlayX = videoGlobalPos.x() + (videoSize.width() - overlayWidth) / 2;
    // Position 20px above the timeline widget (at bottom of video area)
    int overlayY = videoGlobalPos.y() + videoSize.height() - overlayHeight - 20;
    
    m_activityOverlay->move(overlayX, overlayY);
    m_activityOverlay->raise(); // Bring to front
    m_activityOverlay->show();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    // Reposition overlay when window is resized
    QTimer::singleShot(0, this, &MainWindow::positionActivityOverlay);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Keep overlay on top whenever centralWidget's children are reordered
    if (event->type() == QEvent::ChildPolished || 
        event->type() == QEvent::ChildAdded ||
        event->type() == QEvent::Paint) {
        if (m_activityOverlay && m_activityOverlay->isVisible()) {
            m_activityOverlay->raise();
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::onMediaChanged(const MediaItem &item)
{
    Q_UNUSED(item);
    std::cout << "MainWindow: Media changed, re-raising activity overlay" << std::endl;
    // Ensure overlay stays visible and on top when media changes
    if (m_activityOverlay) {
        m_activityOverlay->raise();
        m_activityOverlay->show();
        // Reposition after a short delay to ensure layout is updated
        QTimer::singleShot(50, this, [this]() {
            if (m_activityOverlay) {
                positionActivityOverlay();
                m_activityOverlay->raise();
                m_activityOverlay->show();
            }
        });
    }
}
