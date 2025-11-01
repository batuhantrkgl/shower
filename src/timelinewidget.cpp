#include "timelinewidget.h"
#include "mainwindow.h"
#include "md3colors.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>
#include <QFontDatabase>
#include <QDateTime>

const QTime TimelineWidget::FIRST_PERIOD_END(9, 30);
const QTime TimelineWidget::LUNCH_START(12, 0);
const QTime TimelineWidget::LUNCH_END(12, 45);
const QString preferredFont = "Inter";

TimelineWidget::TimelineWidget(NetworkClient *networkClient, QWidget *parent) : QWidget(parent)
    , m_schoolStart(8, 50)
    , m_schoolEnd(15, 55)
    , m_networkClient(networkClient)
    , m_scheduleLoaded(false)
{
    setupUI();

    connect(networkClient, &NetworkClient::scheduleReceived,
            this, &TimelineWidget::onScheduleReceived);
    connect(networkClient, &NetworkClient::networkError, this, &TimelineWidget::onNetworkError);

    // Generate default schedule
    generateSchoolSchedule();
    updateDisplay();
}

void TimelineWidget::setupUI()
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
    int spacing = qRound(16 * scaleFactor);
    int fontSize = qRound(12 * scaleFactor);

    setFixedHeight(barHeight);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(margin, smallMargin, margin, smallMargin);
    layout->setSpacing(spacing);

    // Current activity icon
    m_currentActivityIcon = new QLabel("●");
    m_currentActivityIcon->setFixedSize(iconSize, iconSize);
    m_currentActivityIcon->setAlignment(Qt::AlignCenter);

    // Current activity label
    m_currentActivityLabel = new QLabel("Loading...");
    m_currentActivityLabel->setObjectName("currentActivityLabel");
    m_currentActivityLabel->setAutoFillBackground(true);
    QPalette palette = m_currentActivityLabel->palette();
    palette.setColor(QPalette::Window, MD3Colors::DarkTheme::surfaceContainerHigh());
    palette.setColor(QPalette::WindowText, MD3Colors::DarkTheme::onSurface());
    m_currentActivityLabel->setPalette(palette);
    m_currentActivityLabel->setContentsMargins(4, 2, 4, 2);

    // Time remaining
    m_timeRemainingLabel = new QLabel("--:--");

    // Next activity
    m_nextActivityLabel = new QLabel("Next: --");

    // Current time (right-aligned)
    m_timeLabel = new QLabel();
    m_timeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // Add widgets to layout
    layout->addWidget(m_currentActivityIcon);
    layout->addWidget(m_currentActivityLabel);
    layout->addWidget(m_timeRemainingLabel);
    layout->addWidget(m_nextActivityLabel);
    layout->addStretch();
    layout->addWidget(m_timeLabel);

    // Style with scaled font
    QString style = QString(
        "QLabel {"
        "color: %1;"
        "font-size: %2px;"
        "font-weight: 500;"
        "}"
        "QLabel#currentActivityLabel {"
        "background-color: %3;"
        "color: %1;"
        "padding: 2px 4px;"
        "border-radius: 4px;"
        "font-weight: 500;"
        "}"
    ).arg(MD3Colors::DarkTheme::onSurface().name())
     .arg(fontSize)
     .arg(MD3Colors::DarkTheme::surfaceContainerHigh().name());

    setStyleSheet(style);

    // Set background color like StatusBar
    QString widgetStyle = QString(
        "TimelineWidget {"
        "background-color: %1;"
        "border: none;"
        "}"
    ).arg(MD3Colors::DarkTheme::surfaceContainer().name());
    setStyleSheet(widgetStyle + style);
}

void TimelineWidget::updateCurrentTime(const QTime &currentTime)
{
    m_currentTime = currentTime;
    updateDisplay();
}

void TimelineWidget::updateDisplay()
{
    // Update time label
    if (m_currentTime.isValid()) {
        m_timeLabel->setText(m_currentTime.toString("HH:mm:ss"));
    } else {
        m_timeLabel->setText("--:--:--");
    }

    if (!m_scheduleLoaded) {
        m_currentActivityLabel->setText("No schedule");
        m_timeRemainingLabel->setText("--:--");
        m_nextActivityLabel->setText("Next: --");
        m_currentActivityIcon->setStyleSheet("color: #666666;");
        emit currentActivityChanged("No schedule");
        return;
    }

    updateActivityIndicators();
}

void TimelineWidget::updateActivityIndicators()
{
    if (!m_currentTime.isValid()) {
        m_currentActivityLabel->setText("Off hours");
        m_timeRemainingLabel->setText("--:--");
        m_nextActivityLabel->setText("Next: --");
        m_currentActivityIcon->setStyleSheet("color: #666666;");
        emit currentActivityChanged("Off hours");
        return;
    }

    QString currentActivity = getCurrentActivityName(m_currentTime);
    // ... existing code ...
    QString nextActivity = getNextActivityName(m_currentTime);
    QTime nextActivityStart = getNextActivityStartTime(m_currentTime);

    // Update current activity
    m_currentActivityLabel->setText(currentActivity);
    emit currentActivityChanged(currentActivity);

    // Update icon color based on activity type
    QString iconColor = "#666666"; // Default gray
    if (currentActivity.contains("Ders")) {
        iconColor = "#2196F3"; // Blue for lessons
    } else if (currentActivity.contains("Teneffüs")) {
        iconColor = "#4CAF50"; // Green for breaks
    } else if (currentActivity.contains("Öğle")) {
        iconColor = "#FF9800"; // Orange for lunch
    }
    m_currentActivityIcon->setStyleSheet(QString("color: %1;").arg(iconColor));

    // Update time remaining
    if (!currentActivity.isEmpty() && currentActivity != "Off hours") {
        // Find current activity end time
        QTime endTime;
        for (const ScheduleBlock &block : m_schedule) {
            if (m_currentTime >= block.startTime && m_currentTime < block.endTime) {
                endTime = block.endTime;
                break;
            }
        }
        if (endTime.isValid()) {
            m_timeRemainingLabel->setText(formatTimeRemaining(m_currentTime, endTime));
        } else {
            m_timeRemainingLabel->setText("--:--");
        }
    } else {
        m_timeRemainingLabel->setText("--:--");
    }

    // Update next activity
    if (!nextActivity.isEmpty()) {
        QString nextText = QString("Next: %1").arg(nextActivity);
        if (nextActivityStart.isValid()) {
            nextText += QString(" (%1)").arg(nextActivityStart.toString("HH:mm"));
        }
        m_nextActivityLabel->setText(nextText);
    } else {
        m_nextActivityLabel->setText("Next: --");
    }
}

QString TimelineWidget::getCurrentActivityName(const QTime &currentTime)
{
    if (currentTime < m_schoolStart || currentTime > m_schoolEnd) {
        return "Off hours";
    }

    for (const ScheduleBlock &block : m_schedule) {
        if (currentTime >= block.startTime && currentTime < block.endTime) {
            return block.name;
        }
    }
    return "Free time";
}

QString TimelineWidget::getNextActivityName(const QTime &currentTime)
{
    for (const ScheduleBlock &block : m_schedule) {
        if (currentTime < block.startTime) {
            return block.name;
        }
    }
    return QString();
}

QTime TimelineWidget::getNextActivityStartTime(const QTime &currentTime)
{
    for (const ScheduleBlock &block : m_schedule) {
        if (currentTime < block.startTime) {
            return block.startTime;
        }
    }
    return QTime();
}

QString TimelineWidget::formatTimeRemaining(const QTime &currentTime, const QTime &endTime)
{
    if (!currentTime.isValid() || !endTime.isValid()) {
        return "--:--";
    }

    int secondsRemaining = currentTime.secsTo(endTime);
    if (secondsRemaining <= 0) {
        return "00:00";
    }

    int minutes = secondsRemaining / 60;
    int seconds = secondsRemaining % 60;

    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
}

void TimelineWidget::onScheduleReceived(const QTime &schoolStart, const QTime &schoolEnd,
                                        const QList<ScheduleBlock> &schedule)
{
    qDebug() << "Schedule received from server";
    m_schoolStart = schoolStart;
    m_schoolEnd = schoolEnd;
    m_schedule = schedule;
    m_scheduleLoaded = true;
    updateDisplay();
}

void TimelineWidget::generateSchoolSchedule()
{
    m_schedule.clear();
    m_schedule.append({m_schoolStart, FIRST_PERIOD_END, "Ders 1", "lesson"});
    QTime break1End = FIRST_PERIOD_END.addSecs(10 * 60);
    m_schedule.append({FIRST_PERIOD_END, break1End, "Teneffüs", "break"});
    QTime period2End = break1End.addSecs(40 * 60);
    m_schedule.append({break1End, period2End, "Ders 2", "lesson"});
    QTime break2End = period2End.addSecs(10 * 60);
    m_schedule.append({period2End, break2End, "Teneffüs", "break"});
    QTime period3End = break2End.addSecs(40 * 60);
    m_schedule.append({break2End, period3End, "Ders 3", "lesson"});
    QTime break3End = period3End.addSecs(10 * 60);
    m_schedule.append({period3End, break3End, "Teneffüs", "break"});
    m_schedule.append({break3End, LUNCH_START, "Ders 4", "lesson"});
    m_schedule.append({LUNCH_START, LUNCH_END, "Öğle Arası", "lunch"});
    QTime period5End = LUNCH_END.addSecs(40 * 60);
    m_schedule.append({LUNCH_END, period5End, "Ders 5", "lesson"});
    QTime break4End = period5End.addSecs(10 * 60);
    m_schedule.append({period5End, break4End, "Teneffüs", "break"});
    QTime period6End = break4End.addSecs(40 * 60);
    m_schedule.append({break4End, period6End, "Ders 6", "lesson"});
    QTime break5End = period6End.addSecs(10 * 60);
    m_schedule.append({period6End, break5End, "Teneffüs", "break"});
    QTime period7End = break5End.addSecs(40 * 60);
    m_schedule.append({break5End, period7End, "Ders 7", "lesson"});
    QTime break6End = period7End.addSecs(10 * 60);
    m_schedule.append({period7End, break6End, "Teneffüs", "break"});
    m_schedule.append({break6End, m_schoolEnd, "Ders 8", "lesson"});
}

void TimelineWidget::onNetworkError(const QString &error)
{
    qDebug() << "Network error:" << error;
    m_currentActivityLabel->setText("Connection error");
    m_currentActivityIcon->setStyleSheet("color: #F44336;"); // Red for error
    emit currentActivityChanged("Connection error");
}
