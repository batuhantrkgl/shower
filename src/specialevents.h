#ifndef SPECIALEVENTS_H
#define SPECIALEVENTS_H

#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <QString>
#include "networkclient.h"

struct SpecialEvent {
    int month;          // 1-12 (0 = any month)
    int day;            // 1-31 (0 = any day)
    int year;           // Full year (0 = every year)
    QTime triggerTime;  // Exact time to trigger
    int durationSecs;   // How long to display (in seconds)
    QString imageUrl;   // URL or local path to image (deprecated, use playlistPath)
    QString playlistPath; // Path to special playlist JSON file
    QString title;      // Event title (e.g., "Atatürk'ü Anma")
    bool muted;         // Whether to mute any playing audio
    
    // Check if this event should trigger on given date/time
    // Returns true if we're within the event's time window (trigger time to trigger time + duration)
    bool shouldTrigger(const QDateTime &dateTime) const {
        // Check date match
        if (year != 0 && dateTime.date().year() != year) return false;
        if (month != 0 && dateTime.date().month() != month) return false;
        if (day != 0 && dateTime.date().day() != day) return false;
        
        // Check if current time is within event window
        QDateTime eventStart = QDateTime(dateTime.date(), triggerTime);
        QDateTime eventEnd = eventStart.addSecs(durationSecs);
        
        return dateTime >= eventStart && dateTime < eventEnd;
    }
};

class SpecialEvents : public QObject
{
    Q_OBJECT

public:
    explicit SpecialEvents(QObject *parent = nullptr);
    
    void checkForEvents(const QDateTime &currentDateTime);
    bool isEventActive() const { return m_activeEvent != nullptr; }
    const SpecialEvent* getActiveEvent() const { return m_activeEvent; }
    MediaItem getEventMediaItem() const;
    MediaPlaylist getEventPlaylist() const;
    void addCustomEvent(const SpecialEvent &event);
    void loadSpecialPlaylistsFromDirectory(const QString &dirPath);
    
signals:
    void eventTriggered(const SpecialEvent &event);
    void eventEnded();

private:
    void initializeEvents();
    void activateEvent(const SpecialEvent &event);
    void deactivateEvent();
    MediaPlaylist loadPlaylistFromFile(const QString &filePath) const;
    
    QList<SpecialEvent> m_events;
    const SpecialEvent *m_activeEvent = nullptr;
    QTimer *m_eventTimer = nullptr;
    QDateTime m_eventStartTime;
    MediaPlaylist m_activePlaylist;
};

#endif // SPECIALEVENTS_H
