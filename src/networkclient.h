#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTime>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QString>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QAbstractSocket>

struct ScheduleBlock {
    QTime startTime;
    QTime endTime;
    QString name;
    QString type;
};

struct MediaItem {
    QString type; // "video", "image", or "screen"
    QString url;
    int duration; // in milliseconds (for images) or -1 for full video duration, ignored for screen
    bool muted; // for videos, ignored for images and screen
    // Optional per-item custom trigger time (HH:MM). If set, this item should be
    // played exactly at that time during special playlists. hasCustomTime indicates
    // whether customTime is valid.
    QTime customTime;
    bool hasCustomTime = false;
};

struct MediaPlaylist {
    QList<MediaItem> items;
    int currentIndex;
    bool isSpecial = false; // if true, play once and notify when finished
    // Optional special date for playlists loaded from JSON (YYYY-MM-DD)
    QDate specialDate;
    QString title;

    MediaPlaylist() : currentIndex(0) {}
    
    MediaItem getCurrentItem() const {
        if (items.isEmpty()) return MediaItem();
        return items[currentIndex];
    }
    
    void moveToNext() {
        if (items.isEmpty()) return;
        currentIndex = (currentIndex + 1) % items.size();
    }
    
    bool hasItems() const {
        return !items.isEmpty();
    }
};

class NetworkClient : public QObject
{
    Q_OBJECT

public:
    explicit NetworkClient(QObject *parent = nullptr);
    void setServerUrl(const QString &url);
    void discoverAndSetServer();
    void discoverInRange(const QString &networkPrefix);
    void setSpecificServer(const QString &serverUrl); // e.g., "10.1.1" to scan 10.1.1.*
    void fetchSchedule();
    void fetchCurrentMedia();
    void fetchServerTime();
    void startPeriodicFetch();
    void stopPeriodicFetch();
    bool isConnected() const { return m_connected; }
    int getLastPing() const { return m_lastPingMs; }
    QString getServerUrl() const { return m_serverUrl; }
    QString getHostname() const { return m_hostname; }
    QDateTime getCurrentDateTime() const; // Get synchronized time (server or system)
    void setTestDateTime(const QDateTime &testDateTime); // Set test date/time for simulation

signals:
    void scheduleReceived(const QTime &schoolStart, const QTime &schoolEnd, 
                         const QList<ScheduleBlock> &schedule);
    void playlistReceived(const MediaPlaylist &playlist);
    void networkError(const QString &error);
    void serverDiscovered(const QString &serverUrl);
    void connectionStatusChanged(bool connected, const QString &serverUrl = QString(), const QString &hostname = QString());
    void pingUpdated(int pingMs);
    void serverTimeReceived(const QDateTime &serverTime, qint64 offsetMs);
    void timeSyncFailed(const QString &reason);

private slots:
    void onScheduleReplyFinished();
    void onMediaReplyFinished();
    void onTimeReplyFinished();
    void periodicFetch();
    void measurePing();
    void onPingReplyFinished();
    void attemptReconnection();
    void syncTimeFromInternet();

private:
    QNetworkAccessManager *m_networkManager;
    QString m_serverUrl;
    QTimer *m_fetchTimer;
    bool m_discovered = false;
    bool m_connected = false;
    int m_lastPingMs = -1;
    QString m_hostname;
    QTimer *m_pingTimer;
    QTimer *m_reconnectTimer;
    int m_reconnectAttempts = 0;
    int m_currentBackoffMs = 1000; // Start with 1 second
    static const int MAX_BACKOFF_MS = 60000; // Max 60 seconds
    static const int BACKOFF_MULTIPLIER = 2;
    
    QString m_cacheDir; // Directory for persistent cache storage
    
    // Time synchronization
    QDateTime m_lastSyncTime; // Last time we synced with server
    qint64 m_timeOffsetMs = 0; // Offset between local and server time (ms)
    bool m_timeSynced = false;
    QTimer *m_timeSyncTimer; // Periodic time sync timer
    
    // Test date/time simulation
    QDateTime m_testDateTime; // Test date/time for simulation (if valid)
    bool m_useTestDateTime = false; // Whether to use test date/time
    
    // Server discovery helpers
    QString getLocalNetworkPrefix();  // Get local network prefix (e.g., "192.168.1" from "192.168.1.42")
    bool tryServerUrl(const QString &url);  // Test if a URL hosts our server
    
    // Reconnection helpers
    void resetBackoff();
    void increaseBackoff();
    
    // Schedule/playlist helpers
    QList<ScheduleBlock> createDefaultSchedule();
    void parseScheduleJson(const QJsonObject &json);
    void parsePlaylistJson(const QJsonObject &json);
    
    // Persistent cache helpers
    void saveCachedSchedule(const QJsonObject &json);
    void saveCachedPlaylist(const QJsonObject &json);
    bool loadCachedSchedule();
    bool loadCachedPlaylist();
    void ensureCacheDir();
};

#endif // NETWORKCLIENT_H
