#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QHBoxLayout>
#include <QLabel>
#include "md3colors.h"
#include "networkclient.h"

class TimelineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TimelineWidget(NetworkClient *networkClient, QWidget *parent = nullptr);

public slots:
    void updateCurrentTime(const QTime &currentTime);
    void onScheduleReceived(const QTime &schoolStart, const QTime &schoolEnd, const QList<ScheduleBlock> &schedule);
    void onNetworkError(const QString &error);

signals:
    void currentActivityChanged(const QString &activityName);

private:
    void setupUI();
    void updateDisplay();
    void updateActivityIndicators();
    QString getCurrentActivityName(const QTime &currentTime);
    QString getNextActivityName(const QTime &currentTime);
    QTime getNextActivityStartTime(const QTime &currentTime);
    QString formatTimeRemaining(const QTime &currentTime, const QTime &endTime);
    void generateSchoolSchedule();

private:
    QTime m_currentTime;
    QList<ScheduleBlock> m_schedule;
    NetworkClient *m_networkClient = nullptr;
    bool m_scheduleLoaded = false;
    QTime m_schoolStart;
    QTime m_schoolEnd;

    // UI elements
    QLabel *m_currentActivityIcon;
    QLabel *m_currentActivityLabel;
    QLabel *m_timeRemainingLabel;
    QLabel *m_nextActivityLabel;
    QLabel *m_timeLabel;

    static const QTime FIRST_PERIOD_END;
    static const QTime LUNCH_START;
    static const QTime LUNCH_END;
};

#endif // TIMELINEWIDGET_H
