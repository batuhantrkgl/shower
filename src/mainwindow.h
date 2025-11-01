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
class ActivityOverlay;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(bool autoDiscover = false, const QString &networkRange = QString(), qreal forcedDpi = 0.0, const QString &testTimeStr = QString(), QWidget *parent = nullptr);
    ~MainWindow();
    
    static qreal getDpiForScreen(QWidget *widget = nullptr);

private slots:
    void onScheduleReceived(const QTime &schoolStart, const QTime &schoolEnd, const QList<ScheduleBlock> &schedule);
    void updateUIState();
    void positionActivityOverlay();
    void onMediaChanged(const MediaItem &item);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    VideoWidget *m_videoWidget;
    TimelineWidget *m_timelineWidget;
    StatusBar *m_statusBar;
    ActivityOverlay *m_activityOverlay;
    NetworkClient *m_networkClient;
    QTimer *m_updateTimer;
    QTime m_schoolStartTime;
    QTime m_schoolEndTime;
    bool m_scheduleLoaded = false;
    qreal m_forcedDpi = 0.0;
    QTime m_testTime;
};
#endif // MAINWINDOW_H
