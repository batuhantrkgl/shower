#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QTimer>
#include "md3colors.h"
#include "networkclient.h"
#include "specialevents.h"

class VideoWidget;
class TimelineWidget;
class StatusBar;
class ActivityOverlay;
class DiagnosticsOverlay;
class MediaCache;
class MediaPlayer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(bool autoDiscover = false, const QString &networkRange = QString(), qreal forcedDpi = 0.0, 
               const QString &testDateStr = QString(), const QString &testTimeStr = QString(), qint64 cacheSize = 4LL * 1024 * 1024 * 1024,
               const QString &specialEventDate = QString(), const QString &specialEventTime = QString(), 
               const QString &specialEventImage = QString(), const QString &specialEventTitle = QString(), 
               int specialEventDuration = 180, QWidget *parent = nullptr);
    ~MainWindow();
    
    static qreal getDpiForScreen(QWidget *widget = nullptr);

private slots:
    void onScheduleReceived(const QTime &schoolStart, const QTime &schoolEnd, const QList<ScheduleBlock> &schedule);
    void onPlaylistReceived(const MediaPlaylist &playlist);
    void onPlaylistFinished();
    void updateUIState();
    void positionActivityOverlay();
    void onMediaChanged(const MediaItem &item);
    void toggleDiagnostics();
    void updateDiagnostics();
    void onLogLevelChanged(const QString &level);
    void onSpecialEventTriggered(const SpecialEvent &event);
    void onSpecialEventEnded();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    VideoWidget *m_videoWidget;
    TimelineWidget *m_timelineWidget;
    StatusBar *m_statusBar;
    ActivityOverlay *m_activityOverlay;
    DiagnosticsOverlay *m_diagnosticsOverlay;
    NetworkClient *m_networkClient;
    MediaCache *m_mediaCache;
    SpecialEvents *m_specialEvents;
    QTimer *m_updateTimer;
    QTimer *m_diagnosticsTimer;
    QTime m_schoolStartTime;
    QTime m_schoolEndTime;
    bool m_scheduleLoaded = false;
    qreal m_forcedDpi = 0.0;
    QTime m_testTime;
    QDate m_testDate;
};
#endif // MAINWINDOW_H
