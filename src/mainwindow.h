#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QTimer>
#include "md3colors.h"
#include "networkclient.h"

class VideoWidget;
class TimelineWidget;
class StatusBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(bool autoDiscover = false, const QString &networkRange = QString(), qreal forcedDpi = 0.0, QWidget *parent = nullptr);
    ~MainWindow();
    
    static qreal getDpiForScreen(QWidget *widget = nullptr);

private slots:
    void onScheduleReceived(const QTime &schoolStart, const QTime &schoolEnd, const QList<ScheduleBlock> &schedule);
    void updateUIState();

private:
    VideoWidget *m_videoWidget;
    TimelineWidget *m_timelineWidget;
    StatusBar *m_statusBar;
    NetworkClient *m_networkClient;
    QTimer *m_updateTimer;
    QTime m_schoolStartTime;
    QTime m_schoolEndTime;
    bool m_scheduleLoaded = false;
    qreal m_forcedDpi = 0.0;
};
#endif // MAINWINDOW_H
