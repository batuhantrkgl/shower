#include "networkclient.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_serverUrl("http://192.168.32.52:8080")
    , m_fetchTimer(new QTimer(this))
{
    // Set up periodic fetch timer (every 5 minutes)
    m_fetchTimer->setInterval(5 * 60 * 1000);
    connect(m_fetchTimer, &QTimer::timeout, this, &NetworkClient::periodicFetch);
}

void NetworkClient::setServerUrl(const QString &url)
{
    m_serverUrl = url;
}

void NetworkClient::fetchSchedule()
{
    QNetworkRequest request(QUrl(m_serverUrl + "/api/schedule"));
    qDebug() << "Fetching schedule from: " << request.url();
    request.setRawHeader("User-Agent", "VideoTimeline Client");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &NetworkClient::onScheduleReplyFinished);
}

void NetworkClient::fetchCurrentMedia()
{
    QNetworkRequest request(QUrl(m_serverUrl + "/api/media/playlist"));
    qDebug() << "Fetching media playlist from: " << request.url();
    request.setRawHeader("User-Agent", "VideoTimeline Client");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &NetworkClient::onMediaReplyFinished);
}

void NetworkClient::startPeriodicFetch()
{
    // Initial fetch
    fetchSchedule();
    fetchCurrentMedia();
    
    // Start periodic updates
    m_fetchTimer->start();
}

void NetworkClient::stopPeriodicFetch()
{
    m_fetchTimer->stop();
}

void NetworkClient::onScheduleReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            parseScheduleJson(doc.object());
        } else {
            qDebug() << "JSON parse error:" << error.errorString();
            emit networkError("Failed to parse schedule JSON");
            // Use default schedule as fallback
            emit scheduleReceived(QTime(8, 50), QTime(15, 55), createDefaultSchedule());
        }
    } else {
        qDebug() << "Network error:" << reply->errorString();
        emit networkError("Failed to fetch schedule: " + reply->errorString());
        // Use default schedule as fallback
        emit scheduleReceived(QTime(8, 50), QTime(15, 55), createDefaultSchedule());
    }
    
    reply->deleteLater();
}

void NetworkClient::onMediaReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            parsePlaylistJson(doc.object());
        } else {
            qDebug() << "JSON parse error:" << error.errorString();
            emit networkError("Failed to parse playlist JSON");
        }
    } else {
        qDebug() << "Network error:" << reply->errorString();
        emit networkError("Failed to fetch playlist: " + reply->errorString());
    }
    
    reply->deleteLater();
}

void NetworkClient::periodicFetch()
{
    fetchSchedule();
    fetchCurrentMedia();
}

QList<ScheduleBlock> NetworkClient::createDefaultSchedule()
{
    QList<ScheduleBlock> schedule;
    
    // Turkish school schedule ending at 15:55 (same as current hardcoded)
    schedule.append({QTime(8, 50), QTime(9, 30), "Ders 1", "lesson"});
    schedule.append({QTime(9, 30), QTime(9, 40), "Teneffüs", "break"});
    schedule.append({QTime(9, 40), QTime(10, 20), "Ders 2", "lesson"});
    schedule.append({QTime(10, 20), QTime(10, 30), "Teneffüs", "break"});
    schedule.append({QTime(10, 30), QTime(11, 10), "Ders 3", "lesson"});
    schedule.append({QTime(11, 10), QTime(11, 20), "Teneffüs", "break"});
    schedule.append({QTime(11, 20), QTime(12, 0), "Ders 4", "lesson"});
    schedule.append({QTime(12, 0), QTime(12, 45), "Öğle Arası", "lunch"});
    schedule.append({QTime(12, 45), QTime(13, 25), "Ders 5", "lesson"});
    schedule.append({QTime(13, 25), QTime(13, 35), "Teneffüs", "break"});
    schedule.append({QTime(13, 35), QTime(14, 15), "Ders 6", "lesson"});
    schedule.append({QTime(14, 15), QTime(14, 25), "Teneffüs", "break"});
    schedule.append({QTime(14, 25), QTime(15, 5), "Ders 7", "lesson"});
    schedule.append({QTime(15, 5), QTime(15, 15), "Teneffüs", "break"});
    schedule.append({QTime(15, 15), QTime(15, 55), "Ders 8", "lesson"});
    
    return schedule;
}

void NetworkClient::parseScheduleJson(const QJsonObject &json)
{
    QString schoolStartStr = json["school_start"].toString();
    QString schoolEndStr = json["school_end"].toString();
    QJsonArray blocksArray = json["blocks"].toArray();
    
    QTime schoolStart = QTime::fromString(schoolStartStr, "HH:mm");
    QTime schoolEnd = QTime::fromString(schoolEndStr, "HH:mm");
    
    if (!schoolStart.isValid()) schoolStart = QTime(8, 50);
    if (!schoolEnd.isValid()) schoolEnd = QTime(15, 55);
    
    QList<ScheduleBlock> schedule;
    
    for (const QJsonValue &blockValue : blocksArray) {
        QJsonObject blockObj = blockValue.toObject();
        
        ScheduleBlock block;
        block.startTime = QTime::fromString(blockObj["start_time"].toString(), "HH:mm");
        block.endTime = QTime::fromString(blockObj["end_time"].toString(), "HH:mm");
        block.name = blockObj["name"].toString();
        block.type = blockObj["type"].toString();
        
        if (block.startTime.isValid() && block.endTime.isValid() && 
            !block.name.isEmpty() && !block.type.isEmpty()) {
            schedule.append(block);
        }
    }
    
    // If no valid blocks, use default
    if (schedule.isEmpty()) {
        schedule = createDefaultSchedule();
    }
    
    emit scheduleReceived(schoolStart, schoolEnd, schedule);
}

void NetworkClient::parsePlaylistJson(const QJsonObject &json)
{
    MediaPlaylist playlist;
    QJsonArray itemsArray = json["items"].toArray();
    
    for (const QJsonValue &itemValue : itemsArray) {
        QJsonObject itemObj = itemValue.toObject();
        
        MediaItem item;
        item.type = itemObj["type"].toString();
        item.url = itemObj["url"].toString();
        item.duration = itemObj["duration"].toInt();
        item.muted = itemObj["muted"].toBool();
        
        if (!item.type.isEmpty() && !item.url.isEmpty()) {
            // Convert relative URL to absolute
            if (item.url.startsWith("/")) {
                item.url = m_serverUrl + item.url;
            }
            playlist.items.append(item);
        }
    }
    
    if (playlist.hasItems()) {
        emit playlistReceived(playlist);
    } else {
        emit networkError("Received empty or invalid playlist");
    }
}
