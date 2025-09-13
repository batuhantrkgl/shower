#include "mainwindow.h"
#include "videowidget.h"
#include "timelinewidget.h"
#include "md3colors.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QApplication>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
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
    // --- Fix fullscreen layout ---
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_networkClient = new NetworkClient(this);
    m_videoWidget = new VideoWidget(this);
    m_timelineWidget = new TimelineWidget(m_networkClient, this);

    mainLayout->addWidget(m_videoWidget, 1);
    mainLayout->addWidget(m_timelineWidget, 0);

    // --- Connect Signals and Slots ---
    connect(m_networkClient, &NetworkClient::mediaReceived,
            m_videoWidget, &VideoWidget::onMediaReceived);
    connect(m_networkClient, &NetworkClient::scheduleReceived,
            this, &MainWindow::onScheduleReceived);

    // ** NEW, IMPORTANT CONNECTION **
    // Tell the video widget when a network error happens
    connect(m_networkClient, &NetworkClient::networkError,
            m_videoWidget, &VideoWidget::onNetworkError);


    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::updateUIState);
    m_updateTimer->start(1000);

    m_networkClient->startPeriodicFetch();
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
