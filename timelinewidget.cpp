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

const QTime TimelineWidget::FIRST_PERIOD_END(9, 30);
const QTime TimelineWidget::LUNCH_START(12, 0);
const QTime TimelineWidget::LUNCH_END(12, 45);

TimelineWidget::TimelineWidget(NetworkClient *networkClient, QWidget *parent) : QWidget(parent)
    , m_schoolStart(8, 50)
    , m_schoolEnd(15, 55)
{
    setFixedHeight(140);
    QString timelineStyle = QString(
        "TimelineWidget {"
            "background-color: %1;"
            "border: 1px solid %2;"
        "}"
    ).arg(MD3Colors::DarkTheme::surfaceContainer().name())
     .arg(MD3Colors::DarkTheme::outline().name());
    setStyleSheet(timelineStyle);
    m_currentTime = QTime();
    connect(networkClient, &NetworkClient::scheduleReceived,
            this, &TimelineWidget::onScheduleReceived);
    connect(networkClient, &NetworkClient::networkError, this, &TimelineWidget::onNetworkError);
    generateSchoolSchedule();
}

void TimelineWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QRect timelineRect = rect().adjusted(
        MD3Spacing::spacing4(), MD3Spacing::spacing4(),
        -MD3Spacing::spacing4(), -MD3Spacing::spacing4()
    );

    int totalMinutes = m_schoolStart.msecsTo(m_schoolEnd) / 60000;

    QFont labelFont;
    if (QFontDatabase().families().contains("Roboto")) {
        labelFont = QFont("Roboto", MD3Typography::LabelMedium::size(), MD3Typography::LabelMedium::weight());
    } else {
        labelFont = QFont(font().family(), MD3Typography::LabelMedium::size(), MD3Typography::LabelMedium::weight());
    }

    painter.setPen(MD3Colors::DarkTheme::onSurfaceVariant());
    painter.setFont(labelFont);

    QFontMetrics fm(labelFont);
    int timeLabelsY = timelineRect.top() + fm.height() + MD3Spacing::spacing2();
    int timeScaleHeight = fm.height() + MD3Spacing::spacing4();
    QRect timeScaleRect(timelineRect.left(), timelineRect.top(), timelineRect.width(), timeScaleHeight);
    painter.fillRect(timeScaleRect, MD3Colors::DarkTheme::surfaceVariant());

    for (int hour = 9; hour <= 15; ++hour) {
        QTime timeMarker(hour, 0);
        int minutesFromStart = m_schoolStart.msecsTo(timeMarker) / 60000;
        if (timeMarker < m_schoolStart || timeMarker > m_schoolEnd) continue;
        double xPos = timelineRect.left() + (double)minutesFromStart / totalMinutes * timelineRect.width();
        QString timeString = timeMarker.toString("HH:mm");
        int textWidth = fm.horizontalAdvance(timeString);
        painter.setPen(QPen(MD3Colors::DarkTheme::outline(), 1));
        painter.drawLine(QPointF(xPos, timeLabelsY + MD3Spacing::spacing2()),
                         QPointF(xPos, timelineRect.bottom()));
        painter.setPen(MD3Colors::DarkTheme::onSurfaceVariant());
        painter.drawText(QPointF(xPos - textWidth / 2, timeLabelsY), timeString);
    }

    int blocksAreaTop = timeScaleRect.bottom() + MD3Spacing::spacing4();
    int blocksAreaHeight = timelineRect.bottom() - blocksAreaTop;

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

    if (m_currentTime.isValid()) {
        drawCurrentTimeIndicator(painter, timelineRect, m_schoolStart, totalMinutes,
                                 timeScaleRect.bottom(),
                                 timelineRect.bottom() - timeScaleRect.bottom());
    } else if (m_scheduleLoaded) {
        drawOffHoursIndicator(painter, timelineRect);
    }
}

void TimelineWidget::drawOffHoursIndicator(QPainter &painter, const QRect &timelineRect)
{
    QString text = "Mesai Dışı";
    QFont textFont;
    if (QFontDatabase().families().contains("Roboto")) {
        textFont = QFont("Roboto", MD3Typography::LabelMedium::size(), MD3Typography::LabelMedium::weight());
    } else {
        textFont = QFont(font().family(), MD3Typography::LabelMedium::size(), MD3Typography::LabelMedium::weight());
    }
    painter.setFont(textFont);
    QFontMetrics fm(textFont);

    QFontMetrics labelFm(painter.font());
    int timeScaleHeight = labelFm.height() + MD3Spacing::spacing4();
    int indicatorYStart = timelineRect.top() + timeScaleHeight;
    int indicatorAreaHeight = MD3Spacing::spacing4();

    int textWidth = fm.horizontalAdvance(text) + MD3Spacing::spacing4();
    int textHeight = fm.height() + MD3Spacing::spacing2();
    int xPos = timelineRect.left() + (timelineRect.width() - textWidth) / 2;
    int yPos = indicatorYStart + (indicatorAreaHeight - textHeight) / 2;
    QRect textRect(xPos, yPos, textWidth, textHeight);

    painter.fillRect(textRect, MD3Colors::DarkTheme::surfaceContainerHighest());
    painter.setPen(MD3Colors::DarkTheme::onSurface());
    painter.drawText(textRect, Qt::AlignCenter, text);
}

void TimelineWidget::onScheduleReceived(const QTime &schoolStart, const QTime &schoolEnd,
                                        const QList<ScheduleBlock> &schedule)
{
    qDebug() << "Schedule received from server";
    m_schoolStart = schoolStart;
    m_schoolEnd = schoolEnd;
    m_schedule = schedule;
    m_scheduleLoaded = true;
    update();
}

void TimelineWidget::updateCurrentTime(const QTime &currentTime)
{
    m_currentTime = currentTime;
    update();
}

void TimelineWidget::drawActivityBlock(QPainter &painter, const QRect &timelineRect,
                                       const QTime &startTime, const QTime &endTime,
                                       const QTime &referenceTime, int totalMinutes,
                                       int yPos, int height,
                                       const QColor &fillColor, const QColor &textColor,
                                       const QString &label)
{
    int startMinutes = (startTime.hour() * 60 + startTime.minute()) - (referenceTime.hour() * 60 + referenceTime.minute());
    int endMinutes = (endTime.hour() * 60 + endTime.minute()) - (referenceTime.hour() * 60 + referenceTime.minute());
    double x1 = timelineRect.left() + (double)startMinutes / totalMinutes * timelineRect.width();
    double x2 = timelineRect.left() + (double)endMinutes / totalMinutes * timelineRect.width();
    if (x1 >= timelineRect.right() || x2 <= timelineRect.left()) return;
    x1 = qMax(x1, (double)timelineRect.left());
    x2 = qMin(x2, (double)timelineRect.right());
    if (x2 - x1 < 2) return;
    QRectF blockRect(x1, yPos, x2 - x1, height);
    painter.fillRect(blockRect, fillColor);
    painter.setPen(QPen(MD3Colors::DarkTheme::outline(), 1));
    painter.drawRect(blockRect);
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
    if (m_currentTime < m_schoolStart || m_currentTime > m_schoolEnd) {
        return;
    }
    int minutesFromStart = referenceTime.msecsTo(m_currentTime) / 60000;
    double xPos = timelineRect.left() + (double)minutesFromStart / totalMinutes * timelineRect.width();
    painter.setPen(QPen(MD3Colors::DarkTheme::error(), 3));
    painter.drawLine(QPointF(xPos, yStart), QPointF(xPos, yStart + height));
    painter.setBrush(MD3Colors::DarkTheme::error());
    painter.drawEllipse(QPointF(xPos, yStart), 6, 6);
    QFont timeFont;
    if (QFontDatabase().families().contains("Roboto")) {
        timeFont = QFont("Roboto", MD3Typography::LabelSmall::size(), MD3Typography::LabelSmall::weight());
    } else {
        timeFont = QFont(font().family(), MD3Typography::LabelSmall::size(), MD3Typography::LabelSmall::weight());
    }
    painter.setFont(timeFont);
    painter.setPen(MD3Colors::DarkTheme::error());
    QString currentTimeText = m_currentTime.toString("HH:mm");
    QString activityText = getCurrentActivityName(m_currentTime);
    QFontMetrics timeFm(timeFont);
    int textWidth = timeFm.horizontalAdvance(currentTimeText + " - " + activityText);
    QRect textRect(xPos - textWidth/2, yStart - MD3Spacing::spacing8(), textWidth + MD3Spacing::spacing2(), timeFm.height() + MD3Spacing::spacing1());
    painter.fillRect(textRect, MD3Colors::DarkTheme::errorContainer());
    painter.setPen(MD3Colors::DarkTheme::onErrorContainer());
    painter.drawText(textRect, Qt::AlignCenter, currentTimeText + " - " + activityText);
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

void TimelineWidget::onNetworkError(const QString &error)
{
    qDebug() << "Network error:" << error;
}
