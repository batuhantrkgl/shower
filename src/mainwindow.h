#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QTimer>
#include "md3colors.h"
#include "networkclient.h"

class VideoWidget;
class TimelineWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(bool autoDiscover = false, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onScheduleReceived(const QTime &schoolStart, const QTime &schoolEnd, const QList<ScheduleBlock> &schedule);
    void updateUIState();

private:
    VideoWidget *m_videoWidget;
    TimelineWidget *m_timelineWidget;
    NetworkClient *m_networkClient;
    QTimer *m_updateTimer;
    QTime m_schoolStartTime;
    QTime m_schoolEndTime;
    bool m_scheduleLoaded = false;
};
#endif // MAINWINDOW_H
