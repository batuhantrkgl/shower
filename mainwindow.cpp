#include "mainwindow.h"
#include "videowidget.h"
#include "timelinewidget.h"
#include "md3colors.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QNetworkAccessManager>
#include <QApplication>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Set the window title
    setWindowTitle("Video Timeline");
    
    // Set window properties for fullscreen Material Design 3 experience
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
    
    // Create the main widget and layout with MD3 design system
    QWidget *centralWidget = new QWidget(this);
    
    // Apply MD3 background color and styling
    QString mainStyle = QString(
        "QWidget {"
            "background-color: %1;"
            "color: %2;"
        "}"
    ).arg(MD3Colors::DarkTheme::background().name())
     .arg(MD3Colors::DarkTheme::onBackground().name());
    
    centralWidget->setStyleSheet(mainStyle);
    
    // Create main layout with MD3 spacing
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(MD3Spacing::spacing4());
    mainLayout->setContentsMargins(
        MD3Spacing::spacing6(),  // left
        MD3Spacing::spacing6(),  // top  
        MD3Spacing::spacing6(),  // right
        MD3Spacing::spacing6()   // bottom
    );

    // Create the network client
    networkClient = new NetworkClient(this);

    // Create the video and timeline widgets
    videoWidget = new VideoWidget(this);
    timelineWidget = new TimelineWidget(networkClient, this);

    // Set stretch factors for proper proportions
    mainLayout->addWidget(videoWidget, 1); // Video takes most space
    mainLayout->addWidget(timelineWidget, 0); // Timeline takes fixed space

    // Connect network client signals to widget slots
    connect(networkClient, &NetworkClient::mediaReceived,
            videoWidget, &VideoWidget::onMediaReceived);

    // Start fetching data from the server
    networkClient->startPeriodicFetch();

    // Set the central widget
    setCentralWidget(centralWidget);
    
    // Ensure fullscreen on the primary screen
    QScreen *primaryScreen = QApplication::primaryScreen();
    if (primaryScreen) {
        setGeometry(primaryScreen->geometry());
        showFullScreen();
    }
}

MainWindow::~MainWindow()
{
}
