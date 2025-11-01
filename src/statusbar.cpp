#include "statusbar.h"
#include <QDateTime>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>

StatusBar::StatusBar(QWidget *parent)
    : QWidget(parent)
    , m_connected(false)
    , m_pingMs(-1)
    , m_hostname(QString())
    , m_timeTimer(new QTimer(this))
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
    setFixedHeight(32); // GNOME-like height

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 4, 12, 4);
    layout->setSpacing(16);

    // Connection status
    m_connectionIcon = new QLabel("●");
    m_connectionIcon->setFixedSize(12, 12);
    m_connectionText = new QLabel("Disconnected");

    // Server info
    m_serverLabel = new QLabel("No server");

    // Ping
    m_pingLabel = new QLabel("Ping: --");

    // Time (right-aligned)
    m_timeLabel = new QLabel();
    m_timeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // Add stretch to push time to the right
    layout->addWidget(m_connectionIcon);
    layout->addWidget(m_connectionText);
    layout->addWidget(m_serverLabel);
    layout->addWidget(m_pingLabel);
    layout->addStretch();
    layout->addWidget(m_timeLabel);

    // Style
    QString style = QString(
        "QLabel {"
        "color: %1;"
        "font-size: 12px;"
        "font-weight: 500;"
        "}"
    ).arg(MD3Colors::DarkTheme::onSurface().name());

    setStyleSheet(style);
    updateConnectionIcon();
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