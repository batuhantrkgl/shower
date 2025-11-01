#include "mainwindow.h"
#include "videowidget.h"
#include "timelinewidget.h"
#include "statusbar.h"
#include "md3colors.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QApplication>
#include <QScreen>

MainWindow::MainWindow(bool autoDiscover, const QString &networkRange, QWidget *parent)
    : QMainWindow(parent)
{
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

    // Add widgets to layout
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

    QTime currentTime = QTime::currentTime();

    if (currentTime < m_schoolStartTime || currentTime > m_schoolEndTime) {
        m_timelineWidget->updateCurrentTime(QTime());
    } else {
        m_timelineWidget->updateCurrentTime(currentTime);
    }
}
