#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTime>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>

struct ScheduleBlock {
    QTime startTime;
    QTime endTime;
    QString name;
    QString type;
};

struct MediaInfo {
    QString type; // "video" or "image"
    QString url;
    int duration; // in milliseconds
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
    void mediaReceived(const MediaInfo &media);
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
    void parseMediaJson(const QJsonObject &json);
};

#endif // NETWORKCLIENT_H
