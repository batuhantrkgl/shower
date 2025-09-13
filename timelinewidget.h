#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QDateTime>
#include "md3colors.h"
#include "networkclient.h"

class TimelineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TimelineWidget(NetworkClient *networkClient, QWidget *parent = nullptr);

public slots:
    void updateCurrentTime();
    void onScheduleReceived(const QTime &schoolStart, const QTime &schoolEnd, 
                           const QList<ScheduleBlock> &schedule);
    void onNetworkError(const QString &error);

protected:
    void paintEvent(QPaintEvent *event) override;
    void drawActivityBlock(QPainter &painter, const QRect &timelineRect,
                          const QTime &startTime, const QTime &endTime,
                          const QTime &referenceTime, int totalMinutes,
                          int yPos, int height,
                          const QColor &fillColor, const QColor &textColor,
                          const QString &label);
    void drawCurrentTimeIndicator(QPainter &painter, const QRect &timelineRect,
                                 const QTime &referenceTime, int totalMinutes,
                                 int yStart, int height);
    void generateSchoolSchedule();
    QString getCurrentActivityName(const QTime &currentTime);

private:
    // Note: ScheduleBlock is now defined in networkclient.h
    
    QTimer *m_updateTimer;
    QTime m_currentTime;
    QList<ScheduleBlock> m_schedule;
    NetworkClient *m_networkClient = nullptr;
    
    // School schedule constants (now dynamic from server)
    QTime m_schoolStart;
    QTime m_schoolEnd;
    static const QTime FIRST_PERIOD_END;
    static const QTime LUNCH_START;
    static const QTime LUNCH_END;
};

#endif // TIMELINEWIDGET_H
