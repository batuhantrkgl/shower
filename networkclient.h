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
    void fetchSchedule();
    void fetchCurrentMedia();
    void startPeriodicFetch();
    void stopPeriodicFetch();

signals:
    void scheduleReceived(const QTime &schoolStart, const QTime &schoolEnd, 
                         const QList<ScheduleBlock> &schedule);
    void playlistReceived(const MediaPlaylist &playlist);
    void networkError(const QString &error);

private slots:
    void onScheduleReplyFinished();
    void onMediaReplyFinished();
    void periodicFetch();

private:
    QNetworkAccessManager *m_networkManager;
    QString m_serverUrl;
    QTimer *m_fetchTimer;
    
    // Default fallback schedule
    QList<ScheduleBlock> createDefaultSchedule();
    void parseScheduleJson(const QJsonObject &json);
    void parsePlaylistJson(const QJsonObject &json);
};

#endif // NETWORKCLIENT_H
