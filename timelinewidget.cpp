#include "timelinewidget.h"
#include "md3colors.h"
#include <QPainter>
#include <QPaintEvent>
#include <QTime>
#include <QFontMetrics>
#include <QFont>
#include "networkclient.h"
#include <QFontDatabase>
#include <QPainterPath>
#include <QDateTime>
#include <QDebug>

// Define static constants
const QTime TimelineWidget::FIRST_PERIOD_END(9, 30);
const QTime TimelineWidget::LUNCH_START(12, 0);
const QTime TimelineWidget::LUNCH_END(12, 45);


TimelineWidget::TimelineWidget(NetworkClient *networkClient, QWidget *parent) : QWidget(parent)
    , m_schoolStart(8, 50) // Default fallback values
    , m_schoolEnd(15, 55)
{
    // Set height following MD3 guidelines for better touch targets
    setFixedHeight(140); // Increased for current time indicator
    
    // Apply MD3 surface container styling without roundness
    QString timelineStyle = QString(
        "TimelineWidget {"
            "background-color: %1;"
            "border: 1px solid %2;"
        "}"
    ).arg(MD3Colors::DarkTheme::surfaceContainer().name())
     .arg(MD3Colors::DarkTheme::outline().name());
    
    setStyleSheet(timelineStyle);
    
    // Initialize current time
    m_currentTime = QTime::currentTime();
    
    // Setup network client
     connect(networkClient, &NetworkClient::scheduleReceived,
            this, &TimelineWidget::onScheduleReceived);
      connect(networkClient, &NetworkClient::networkError, this, &TimelineWidget::onNetworkError);
    
    // Generate fallback schedule initially
    generateSchoolSchedule();
    
    // Setup update timer
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &TimelineWidget::updateCurrentTime);
    m_updateTimer->start(30000); // Update every 30 seconds
}

void TimelineWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    
    // Account for border and padding
    QRect timelineRect = rect().adjusted(
        MD3Spacing::spacing4(), 
        MD3Spacing::spacing4(), 
        -MD3Spacing::spacing4(), 
        -MD3Spacing::spacing4()
    );

    // Calculate total minutes from school start to end
    int totalMinutes = m_schoolStart.msecsTo(m_schoolEnd) / 60000;

    // Set up MD3 typography for time labels
    QFont labelFont;
    if (QFontDatabase().families().contains("Roboto")) {
        labelFont = QFont("Roboto", MD3Typography::LabelMedium::size(), MD3Typography::LabelMedium::weight());
    } else {
        labelFont = QFont(font().family(), MD3Typography::LabelMedium::size(), MD3Typography::LabelMedium::weight());
    }

    // --- Draw Time Labels ---
    painter.setPen(MD3Colors::DarkTheme::onSurfaceVariant());
    painter.setFont(labelFont);
    
    QFontMetrics fm(labelFont);
    int timeLabelsY = timelineRect.top() + fm.height() + MD3Spacing::spacing2();
    
    // Draw time scale background without roundness
    QRect timeScaleRect(timelineRect.left(), timelineRect.top(), timelineRect.width(), fm.height() + MD3Spacing::spacing4());
    painter.fillRect(timeScaleRect, MD3Colors::DarkTheme::surfaceVariant());

    // Draw hourly time markers in 24-hour format
    for (int hour = 9; hour <= 15; ++hour) {
        QTime timeMarker(hour, 0);
        int minutesFromStart = m_schoolStart.msecsTo(timeMarker) / 60000;
        
        // Skip if before school start or after school end
        if (timeMarker < m_schoolStart || timeMarker > m_schoolEnd) continue;
        
        double xPos = timelineRect.left() + (double)minutesFromStart / totalMinutes * timelineRect.width();

        QString timeString = timeMarker.toString("HH:mm"); // 24-hour format
        int textWidth = fm.horizontalAdvance(timeString);
        
        // Draw time indicator line
        painter.setPen(QPen(MD3Colors::DarkTheme::outline(), 1));
        painter.drawLine(QPointF(xPos, timeLabelsY + MD3Spacing::spacing2()), 
                        QPointF(xPos, timelineRect.bottom()));
        
        // Draw time label
        painter.setPen(MD3Colors::DarkTheme::onSurfaceVariant());
        painter.drawText(QPointF(xPos - textWidth / 2, timeLabelsY), timeString);
    }

    // --- Draw School Schedule Blocks ---
    int blocksAreaTop = timeLabelsY + MD3Spacing::spacing6();
    int blocksAreaHeight = timelineRect.bottom() - blocksAreaTop - MD3Spacing::spacing4();
    
    // Draw all scheduled blocks
    for (const ScheduleBlock &block : m_schedule) {
        QColor fillColor, textColor;
        
        if (block.type == "lesson") {
            fillColor = MD3Colors::DarkTheme::primaryContainer();
            textColor = MD3Colors::DarkTheme::onPrimaryContainer();
        } else if (block.type == "break") {
            fillColor = MD3Colors::DarkTheme::secondaryContainer();
            textColor = MD3Colors::DarkTheme::onSecondaryContainer();
        } else if (block.type == "lunch") {
            fillColor = MD3Colors::DarkTheme::tertiaryContainer();
            textColor = MD3Colors::DarkTheme::onTertiaryContainer();
        }
        
        drawActivityBlock(painter, timelineRect, block.startTime, block.endTime, 
                         m_schoolStart, totalMinutes, blocksAreaTop, blocksAreaHeight,
                         fillColor, textColor, block.name);
    }
    
    // --- Draw Current Time Indicator ---
    drawCurrentTimeIndicator(painter, timelineRect, m_schoolStart, totalMinutes, 
                            timeLabelsY + MD3Spacing::spacing2(), 
                            timelineRect.bottom() - timeLabelsY - MD3Spacing::spacing2());
}

void TimelineWidget::drawActivityBlock(QPainter &painter, const QRect &timelineRect,
                                     const QTime &startTime, const QTime &endTime,
                                     const QTime &referenceTime, int totalMinutes,
                                     int yPos, int height,
                                     const QColor &fillColor, const QColor &textColor,
                                     const QString &label)
{
    // Calculate pixel positions
    int startMinutes = (startTime.hour() * 60 + startTime.minute()) - (referenceTime.hour() * 60 + referenceTime.minute());
    int endMinutes = (endTime.hour() * 60 + endTime.minute()) - (referenceTime.hour() * 60 + referenceTime.minute());
    
    double x1 = timelineRect.left() + (double)startMinutes / totalMinutes * timelineRect.width();
    double x2 = timelineRect.left() + (double)endMinutes / totalMinutes * timelineRect.width();
    
    if (x1 >= timelineRect.right() || x2 <= timelineRect.left()) return;
    
    // Clamp to visible area
    x1 = qMax(x1, (double)timelineRect.left());
    x2 = qMin(x2, (double)timelineRect.right());
    
    if (x2 - x1 < 2) return; // Too narrow to draw
    
    // Create rectangular block without roundness
    QRectF blockRect(x1, yPos, x2 - x1, height);
    
    // Draw the block
    painter.fillRect(blockRect, fillColor);
    
    // Draw border
    painter.setPen(QPen(MD3Colors::DarkTheme::outline(), 1));
    painter.drawRect(blockRect);
    
    // Draw label if there's enough space
    if (blockRect.width() > 40) {
        QFont blockFont;
        if (QFontDatabase().families().contains("Roboto")) {
            blockFont = QFont("Roboto", MD3Typography::LabelSmall::size(), MD3Typography::LabelSmall::weight());
        } else {
            blockFont = QFont(font().family(), MD3Typography::LabelSmall::size(), MD3Typography::LabelSmall::weight());
        }
        
        painter.setFont(blockFont);
        painter.setPen(textColor);
        
        QFontMetrics blockFm(blockFont);
        if (blockRect.width() > blockFm.horizontalAdvance(label) + MD3Spacing::spacing2()) {
            painter.drawText(blockRect, Qt::AlignCenter, label);
        }
    }
}

void TimelineWidget::drawCurrentTimeIndicator(QPainter &painter, const QRect &timelineRect,
                                             const QTime &referenceTime, int totalMinutes,
                                             int yStart, int height)
{
    // Only draw if current time is within school hours
    if (m_currentTime < m_schoolStart || m_currentTime > m_schoolEnd) {
        return;
    }
    
    int minutesFromStart = referenceTime.msecsTo(m_currentTime) / 60000;
    double xPos = timelineRect.left() + (double)minutesFromStart / totalMinutes * timelineRect.width();
    
    // Draw current time line
    painter.setPen(QPen(MD3Colors::DarkTheme::error(), 3));
    painter.drawLine(QPointF(xPos, yStart), QPointF(xPos, yStart + height));
    
    // Draw current time indicator circle
    painter.setBrush(MD3Colors::DarkTheme::error());
    painter.drawEllipse(QPointF(xPos, yStart), 6, 6);
    
    // Draw current time text
    QFont timeFont;
    if (QFontDatabase().families().contains("Roboto")) {
        timeFont = QFont("Roboto", MD3Typography::LabelSmall::size(), MD3Typography::LabelSmall::weight());
    } else {
        timeFont = QFont(font().family(), MD3Typography::LabelSmall::size(), MD3Typography::LabelSmall::weight());
    }
    
    painter.setFont(timeFont);
    painter.setPen(MD3Colors::DarkTheme::error());
    
    QString currentTimeText = m_currentTime.toString("HH:mm"); // 24-hour format
    QString activityText = getCurrentActivityName(m_currentTime);
    
    // Draw current time with background
    QFontMetrics timeFm(timeFont);
    int textWidth = timeFm.horizontalAdvance(currentTimeText + " - " + activityText);
    
    QRect textRect(xPos - textWidth/2, yStart - MD3Spacing::spacing8(), textWidth + MD3Spacing::spacing2(), timeFm.height() + MD3Spacing::spacing1());
    
    // Draw background without roundness
    painter.fillRect(textRect, MD3Colors::DarkTheme::errorContainer());
    
    // Draw text
    painter.setPen(MD3Colors::DarkTheme::onErrorContainer());
    painter.drawText(textRect, Qt::AlignCenter, currentTimeText + " - " + activityText);
}

void TimelineWidget::generateSchoolSchedule()
{
    m_schedule.clear();
    
    // Turkish school schedule ending at 15:55 (fallback when no network)
    // Ders 1: 8:50 - 9:30
    m_schedule.append({m_schoolStart, FIRST_PERIOD_END, "Ders 1", "lesson"});
    
    // Teneffüs: 9:30 - 9:40
    QTime break1End = FIRST_PERIOD_END.addSecs(10 * 60);
    m_schedule.append({FIRST_PERIOD_END, break1End, "Teneffüs", "break"});
    
    // Ders 2: 9:40 - 10:20
    QTime period2End = break1End.addSecs(40 * 60);
    m_schedule.append({break1End, period2End, "Ders 2", "lesson"});
    
    // Teneffüs: 10:20 - 10:30
    QTime break2End = period2End.addSecs(10 * 60);
    m_schedule.append({period2End, break2End, "Teneffüs", "break"});
    
    // Ders 3: 10:30 - 11:10
    QTime period3End = break2End.addSecs(40 * 60);
    m_schedule.append({break2End, period3End, "Ders 3", "lesson"});
    
    // Teneffüs: 11:10 - 11:20
    QTime break3End = period3End.addSecs(10 * 60);
    m_schedule.append({period3End, break3End, "Teneffüs", "break"});
    
    // Ders 4: 11:20 - 12:00
    m_schedule.append({break3End, LUNCH_START, "Ders 4", "lesson"});
    
    // Öğle Arası: 12:00 - 12:45
    m_schedule.append({LUNCH_START, LUNCH_END, "Öğle Arası", "lunch"});
    
    // Ders 5: 12:45 - 13:25
    QTime period5End = LUNCH_END.addSecs(40 * 60);
    m_schedule.append({LUNCH_END, period5End, "Ders 5", "lesson"});
    
    // Teneffüs: 13:25 - 13:35
    QTime break4End = period5End.addSecs(10 * 60);
    m_schedule.append({period5End, break4End, "Teneffüs", "break"});
    
    // Ders 6: 13:35 - 14:15
    QTime period6End = break4End.addSecs(40 * 60);
    m_schedule.append({break4End, period6End, "Ders 6", "lesson"});
    
    // Teneffüs: 14:15 - 14:25
    QTime break5End = period6End.addSecs(10 * 60);
    m_schedule.append({period6End, break5End, "Teneffüs", "break"});
    
    // Ders 7: 14:25 - 15:05
    QTime period7End = break5End.addSecs(40 * 60);
    m_schedule.append({break5End, period7End, "Ders 7", "lesson"});
    
    // Teneffüs: 15:05 - 15:15
    QTime break6End = period7End.addSecs(10 * 60);
    m_schedule.append({period7End, break6End, "Teneffüs", "break"});
    
    // Ders 8: 15:15 - 15:55
    m_schedule.append({break6End, m_schoolEnd, "Ders 8", "lesson"});
}

QString TimelineWidget::getCurrentActivityName(const QTime &currentTime)
{
    for (const ScheduleBlock &block : m_schedule) {
        if (currentTime >= block.startTime && currentTime < block.endTime) {
            return block.name;
        }
    }
    
    if (currentTime < m_schoolStart) {
        return "Okul Öncesi";
    } else if (currentTime >= m_schoolEnd) {
        return "Okul Sonrası";
    }
    
    return "Boş Zaman";
}

void TimelineWidget::updateCurrentTime()
{
    m_currentTime = QTime::currentTime();
    update(); // Trigger repaint
}

void TimelineWidget::onScheduleReceived(const QTime &schoolStart, const QTime &schoolEnd, 
                                       const QList<ScheduleBlock> &schedule)
{
    qDebug() << "Schedule received from server";
    m_schoolStart = schoolStart;
    m_schoolEnd = schoolEnd;
    m_schedule = schedule;
    update(); // Trigger repaint with new schedule
}
void TimelineWidget::onNetworkError(const QString &error)
{
    qDebug() << "Network error:" << error;
    // Continue using current/fallback schedule
}
