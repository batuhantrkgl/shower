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
    QString type; // "video" or "image"
    QString url;
    int duration; // in milliseconds (for images) or -1 for full video duration
    bool muted; // for videos, ignored for images
};

struct MediaPlaylist {
    QList<MediaItem> items;
    int currentIndex;
    
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
    void startPeriodicFetch();
    void stopPeriodicFetch();
    bool isConnected() const { return m_connected; }
    int getLastPing() const { return m_lastPingMs; }
    QString getServerUrl() const { return m_serverUrl; }
    QString getHostname() const { return m_hostname; }

signals:
    void scheduleReceived(const QTime &schoolStart, const QTime &schoolEnd, 
                         const QList<ScheduleBlock> &schedule);
    void playlistReceived(const MediaPlaylist &playlist);
    void networkError(const QString &error);
    void serverDiscovered(const QString &serverUrl);
    void connectionStatusChanged(bool connected, const QString &serverUrl = QString(), const QString &hostname = QString());
    void pingUpdated(int pingMs);

private slots:
    void onScheduleReplyFinished();
    void onMediaReplyFinished();
    void periodicFetch();
    void measurePing();
    void onPingReplyFinished();
    void attemptReconnection();

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
    
    // Server discovery helpers
    QString getLocalNetworkPrefix();  // Get local network prefix (e.g., "192.168.1" from "192.168.1.42")
    bool tryServerUrl(const QString &url);  // Test if a URL hosts our server
    
    // Schedule/playlist helpers
    QList<ScheduleBlock> createDefaultSchedule();
    void parseScheduleJson(const QJsonObject &json);
    void parsePlaylistJson(const QJsonObject &json);
};

#endif // NETWORKCLIENT_H
