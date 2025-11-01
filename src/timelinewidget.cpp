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
#include <QGuiApplication>
#include <QScreen>

const QTime TimelineWidget::FIRST_PERIOD_END(9, 30);
const QTime TimelineWidget::LUNCH_START(12, 0);
const QTime TimelineWidget::LUNCH_END(12, 45);
const QString preferredFont = "Inter";


TimelineWidget::TimelineWidget(NetworkClient *networkClient, QWidget *parent) : QWidget(parent)
    , m_schoolStart(8, 50)
    , m_schoolEnd(15, 55)
{
    // Get screen DPI for scaling
    QScreen *screen = QGuiApplication::primaryScreen();
    qreal dpi = screen ? screen->logicalDotsPerInch() : 96.0;
    
    // Scale height based on DPI (96 DPI = 140px, scales up/down)
    qreal scaleFactor = dpi / 96.0;
    int timelineHeight = qRound(140 * scaleFactor);
    
    setFixedHeight(timelineHeight);
    
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

    // Get screen DPI for scaling
    QScreen *screen = QGuiApplication::primaryScreen();
    qreal dpi = screen ? screen->logicalDotsPerInch() : 96.0;
    qreal scaleFactor = dpi / 96.0;
    
    const int spacing = qRound(MD3Spacing::spacing4() * scaleFactor);
    const int timeScaleAreaHeight = qRound(30 * scaleFactor);

    QRect blocksAreaRect = rect().adjusted(spacing, spacing + timeScaleAreaHeight, -spacing, -spacing);

    int totalMinutes = m_schoolStart.msecsTo(m_schoolEnd) / 60000;

    QFont labelFont;
    int fontSize = qRound(12 * scaleFactor);
    if (QFontDatabase().families().contains(preferredFont)) {
        labelFont = QFont(preferredFont, fontSize, QFont::DemiBold);
    } else if (QFontDatabase().families().contains("Roboto")) {
        labelFont = QFont("Roboto", qRound(MD3Typography::LabelMedium::size() * scaleFactor), MD3Typography::LabelMedium::weight());
    } else {
        labelFont = QFont(font().family(), qRound(MD3Typography::LabelMedium::size() * scaleFactor), MD3Typography::LabelMedium::weight());
    }
    painter.setFont(labelFont);
    painter.setPen(MD3Colors::DarkTheme::onSurfaceVariant());
    QFontMetrics fm(labelFont);

    for (int hour = 9; hour <= 15; ++hour) {
        QTime timeMarker(hour, 0);
        int minutesFromStart = m_schoolStart.msecsTo(timeMarker) / 60000;
        if (timeMarker < m_schoolStart || timeMarker > m_schoolEnd) continue;

        double xPos = blocksAreaRect.left() + (double)minutesFromStart / totalMinutes * blocksAreaRect.width();
        QString timeString = timeMarker.toString("HH:mm");
        int textWidth = fm.horizontalAdvance(timeString);

        painter.drawText(QPointF(xPos - textWidth / 2.0, blocksAreaRect.top() - spacing), timeString);

        painter.setPen(QPen(MD3Colors::DarkTheme::outline(), 1.5));
        painter.drawLine(QPointF(xPos, blocksAreaRect.top() - spacing / 2.0),
                         QPointF(xPos, blocksAreaRect.top()));
    }

    for (int i = 0; i < m_schedule.size(); ++i) {
        const ScheduleBlock &block = m_schedule.at(i);
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
        drawActivityBlock(painter, blocksAreaRect, block.startTime, block.endTime,
                          m_schoolStart, totalMinutes, blocksAreaRect.top(), blocksAreaRect.height(),
                          fillColor, textColor, block.name,
                          i == 0, i == m_schedule.size() - 1);
    }

    if (m_currentTime.isValid()) {
        drawCurrentTimeIndicator(painter, blocksAreaRect, m_schoolStart, totalMinutes,
                                 blocksAreaRect.top(), blocksAreaRect.height());
    } else if (m_scheduleLoaded) {
        drawOffHoursIndicator(painter, blocksAreaRect);
    }
}

void TimelineWidget::drawOffHoursIndicator(QPainter &painter, const QRect &timelineRect)
{
    // Get screen DPI for scaling
    QScreen *screen = QGuiApplication::primaryScreen();
    qreal dpi = screen ? screen->logicalDotsPerInch() : 96.0;
    qreal scaleFactor = dpi / 96.0;
    
    QString text = "Mesai Dışı";
    QFont textFont;
    int fontSize = qRound(12 * scaleFactor);
    if (QFontDatabase().families().contains(preferredFont)) {
        textFont = QFont(preferredFont, fontSize, QFont::Normal);
    } else if (QFontDatabase().families().contains("Roboto")) {
        textFont = QFont("Roboto", qRound(MD3Typography::LabelMedium::size() * scaleFactor), MD3Typography::LabelMedium::weight());
    } else {
        textFont = QFont(font().family(), qRound(MD3Typography::LabelMedium::size() * scaleFactor), MD3Typography::LabelMedium::weight());
    }
    painter.setFont(textFont);
    QFontMetrics fm(textFont);

    QFontMetrics labelFm(painter.font());
    int timeScaleHeight = qRound((labelFm.height() + MD3Spacing::spacing4()) * scaleFactor);
    int indicatorYStart = timelineRect.top() + timeScaleHeight;
    int indicatorAreaHeight = qRound(MD3Spacing::spacing4() * scaleFactor);

    int textWidth = qRound((fm.horizontalAdvance(text) + MD3Spacing::spacing4()) * scaleFactor);
    int textHeight = qRound((fm.height() + MD3Spacing::spacing2()) * scaleFactor);
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
                                       const QString &label, bool isFirst, bool isLast)
{
    // Get screen DPI for scaling
    QScreen *screen = QGuiApplication::primaryScreen();
    qreal dpi = screen ? screen->logicalDotsPerInch() : 96.0;
    qreal scaleFactor = dpi / 96.0;
    
    int startMinutes = (startTime.hour() * 60 + startTime.minute()) - (referenceTime.hour() * 60 + referenceTime.minute());
    int endMinutes = (endTime.hour() * 60 + endTime.minute()) - (referenceTime.hour() * 60 + referenceTime.minute());
    double x1 = timelineRect.left() + (double)startMinutes / totalMinutes * timelineRect.width();
    double x2 = timelineRect.left() + (double)endMinutes / totalMinutes * timelineRect.width();
    if (x1 >= timelineRect.right() || x2 <= timelineRect.left()) return;
    x1 = qMax(x1, (double)timelineRect.left());
    x2 = qMin(x2, (double)timelineRect.right());
    if (x2 - x1 < 2) return;
    QRectF blockRect(x1, yPos, x2 - x1, height);


    painter.setPen(QPen(MD3Colors::DarkTheme::outline(), 1));
    painter.setBrush(fillColor);
    const qreal radius = qRound(8.0 * scaleFactor);
    QPainterPath path;
    path.moveTo(blockRect.left() + (isFirst ? radius : 0), blockRect.top());
    path.lineTo(blockRect.right() - (isLast ? radius : 0), blockRect.top());
    if (isLast)
        path.arcTo(blockRect.right() - 2 * radius, blockRect.top(), 2 * radius, 2 * radius, 90, -90);
    path.lineTo(blockRect.right(), blockRect.bottom() - (isLast ? radius : 0));
    if (isLast)
        path.arcTo(blockRect.right() - 2 * radius, blockRect.bottom() - 2 * radius, 2 * radius, 2 * radius, 0, -90);
    path.lineTo(blockRect.left() + (isFirst ? radius : 0), blockRect.bottom());
    if (isFirst)
        path.arcTo(blockRect.left(), blockRect.bottom() - 2 * radius, 2 * radius, 2 * radius, 270, -90);
    path.lineTo(blockRect.left(), blockRect.top() + (isFirst ? radius : 0));
    if (isFirst)
        path.arcTo(blockRect.left(), blockRect.top(), 2 * radius, 2 * radius, 180, -90);
    painter.drawPath(path);
    if (!isLast) {
        painter.setPen(QPen(MD3Colors::DarkTheme::outline(), 1));
        painter.drawLine(blockRect.topRight(), blockRect.bottomRight());
    }

    if (blockRect.width() > qRound(40 * scaleFactor)) {
        QFont blockFont;
        int fontSize = qRound(10 * scaleFactor);
        if (QFontDatabase().families().contains(preferredFont)) {
            blockFont = QFont(preferredFont, fontSize, QFont::Medium);
        } else if (QFontDatabase().families().contains("Roboto")) {
            blockFont = QFont("Roboto", qRound(MD3Typography::LabelSmall::size() * scaleFactor), MD3Typography::LabelSmall::weight());
        } else {
            blockFont = QFont(font().family(), qRound(MD3Typography::LabelSmall::size() * scaleFactor), MD3Typography::LabelSmall::weight());
        }
        painter.setFont(blockFont);
        painter.setPen(textColor);
        QFontMetrics blockFm(blockFont);
        if (blockRect.width() > blockFm.horizontalAdvance(label) + qRound(MD3Spacing::spacing2() * scaleFactor)) {
            painter.drawText(blockRect, Qt::AlignCenter, label);
        }
    }
}

void TimelineWidget::drawCurrentTimeIndicator(QPainter &painter, const QRect &timelineRect,
                                              const QTime &referenceTime, int totalMinutes,
                                              int yStart, int height)
{
    // Get screen DPI for scaling
    QScreen *screen = QGuiApplication::primaryScreen();
    qreal dpi = screen ? screen->logicalDotsPerInch() : 96.0;
    qreal scaleFactor = dpi / 96.0;
    
    if (m_currentTime < m_schoolStart || m_currentTime > m_schoolEnd) {
        return;
    }
    int minutesFromStart = referenceTime.msecsTo(m_currentTime) / 60000;
    double xPos = timelineRect.left() + (double)minutesFromStart / totalMinutes * timelineRect.width();


    QFont timeFont;
    int fontSize = qRound(11 * scaleFactor);
    if (QFontDatabase().families().contains(preferredFont)) {
        // --- THIS IS THE FIX ---
        // Changed QFont::SemiBold to QFont::DemiBold, which is the correct enum in Qt.
        timeFont = QFont(preferredFont, fontSize, QFont::DemiBold);
        // --- END OF FIX ---
    } else if (QFontDatabase().families().contains("Roboto")) {
        timeFont = QFont("Roboto", qRound(MD3Typography::LabelSmall::size() * scaleFactor), MD3Typography::LabelSmall::weight());
    } else {
        timeFont = QFont(font().family(), qRound(MD3Typography::LabelSmall::size() * scaleFactor), MD3Typography::LabelSmall::weight());
    }

    painter.setFont(timeFont);
    QString currentTimeText = m_currentTime.toString("HH:mm");
    QString activityText = getCurrentActivityName(m_currentTime);
    QString fullText = currentTimeText + " - " + activityText;
    QFontMetrics timeFm(timeFont);
    int textWidth = qRound((timeFm.horizontalAdvance(fullText) + MD3Spacing::spacing4()) * scaleFactor);
    int textHeight = qRound((timeFm.height() + MD3Spacing::spacing2()) * scaleFactor);

    const qreal pointerHeight = qRound(6.0 * scaleFactor);
    const qreal pointerWidth = qRound(10.0 * scaleFactor);
    const qreal radius = qRound(8.0 * scaleFactor);
    const QRectF textRect(
        xPos - textWidth / 2.0,
        yStart - textHeight - pointerHeight,
        textWidth,
        textHeight
    );

    QPainterPath path;
    path.moveTo(xPos, textRect.bottom() + pointerHeight);
    path.lineTo(xPos - pointerWidth / 2.0, textRect.bottom());
    path.lineTo(textRect.left() + radius, textRect.bottom());
    path.arcTo(textRect.left(), textRect.bottom() - 2 * radius, 2 * radius, 2 * radius, 270, -90);
    path.lineTo(textRect.left(), textRect.top() + radius);
    path.arcTo(textRect.left(), textRect.top(), 2 * radius, 2 * radius, 180, -90);
    path.lineTo(textRect.right() - radius, textRect.top());
    path.arcTo(textRect.right() - 2 * radius, textRect.top(), 2 * radius, 2 * radius, 90, -90);
    path.lineTo(textRect.right(), textRect.bottom() - radius);
    path.arcTo(textRect.right() - 2 * radius, textRect.bottom() - 2 * radius, 2 * radius, 2 * radius, 0, -90);
    path.lineTo(xPos + pointerWidth / 2.0, textRect.bottom());
    path.closeSubpath();

    painter.setPen(QPen(MD3Colors::DarkTheme::error(), 2));
    painter.drawLine(QPointF(xPos, yStart), QPointF(xPos, yStart + height));
    painter.setBrush(MD3Colors::DarkTheme::errorContainer());
    painter.setPen(Qt::NoPen);
    painter.drawPath(path);
    painter.setPen(MD3Colors::DarkTheme::onErrorContainer());
    painter.drawText(textRect, Qt::AlignCenter, fullText);
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
