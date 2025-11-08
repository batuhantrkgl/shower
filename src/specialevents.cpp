#include "specialevents.h"
#include "logger.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>

SpecialEvents::SpecialEvents(QObject *parent)
    : QObject(parent)
    , m_eventTimer(new QTimer(this))
{
    m_eventTimer->setSingleShot(true);
    connect(m_eventTimer, &QTimer::timeout, this, &SpecialEvents::deactivateEvent);
    
    initializeEvents();
    
    // Try to load special playlists from data directory
    loadSpecialPlaylistsFromDirectory("data");
}

void SpecialEvents::initializeEvents()
{
    // Events are now loaded from JSON files in loadSpecialPlaylistsFromDirectory
    // This method is kept for backward compatibility and can be used for hardcoded events
    LOG_INFO_CAT("Special events system initialized", "SpecialEvents");
}

void SpecialEvents::checkForEvents(const QDateTime &currentDateTime)
{
    // If an event is already active, don't check for new ones
    if (m_activeEvent) {
        return;
    }
    
    for (const SpecialEvent &event : m_events) {
        // Use the shouldTrigger method which handles year/month/day/time matching
        if (event.shouldTrigger(currentDateTime)) {
            LOG_INFO_CAT(QString("Triggering special event: %1 (Date: %2-%3-%4, Time: %5)")
                .arg(event.title)
                .arg(event.year)
                .arg(event.month)
                .arg(event.day)
                .arg(event.triggerTime.toString("HH:mm")), "SpecialEvents");
            activateEvent(event);
            return;
        }
    }
}

void SpecialEvents::activateEvent(const SpecialEvent &event)
{
    m_activeEvent = &event;
    m_eventStartTime = QDateTime::currentDateTime();
    
    // Load playlist if available
    if (!event.playlistPath.isEmpty()) {
        m_activePlaylist = loadPlaylistFromFile(event.playlistPath);
        if (m_activePlaylist.hasItems()) {
            // Calculate total duration from playlist items
            int totalDuration = 0;
            for (const MediaItem &item : m_activePlaylist.items) {
                totalDuration += item.duration;
            }
            // Start timer to end the event based on playlist duration
            m_eventTimer->start(totalDuration);
            LOG_INFO_CAT(QString("Event activated: %1 with playlist (duration: %2ms)")
                .arg(event.title)
                .arg(totalDuration), "SpecialEvents");
        } else {
            // Fallback to duration if playlist loading failed
            m_eventTimer->start(event.durationSecs * 1000);
            LOG_WARNING_CAT(QString("Failed to load playlist, using default duration: %1 seconds")
                .arg(event.durationSecs), "SpecialEvents");
        }
    } else {
        // No playlist, use single image with duration
        m_eventTimer->start(event.durationSecs * 1000);
        LOG_INFO_CAT(QString("Event activated: %1 for %2 seconds")
            .arg(event.title)
            .arg(event.durationSecs), "SpecialEvents");
    }
    
    emit eventTriggered(event);
}

void SpecialEvents::deactivateEvent()
{
    if (!m_activeEvent) {
        return;
    }
    
    LOG_INFO_CAT(QString("Event ended: %1").arg(m_activeEvent->title), "SpecialEvents");
    
    m_activeEvent = nullptr;
    m_eventStartTime = QDateTime();
    m_activePlaylist = MediaPlaylist(); // Clear playlist
    
    emit eventEnded();
}

MediaItem SpecialEvents::getEventMediaItem() const
{
    if (!m_activeEvent) {
        return MediaItem();
    }
    
    MediaItem item;
    item.type = "image";
    item.url = m_activeEvent->imageUrl;
    item.duration = m_activeEvent->durationSecs * 1000;  // Convert to milliseconds
    item.muted = m_activeEvent->muted;
    
    return item;
}

void SpecialEvents::addCustomEvent(const SpecialEvent &event)
{
    m_events.append(event);
    LOG_INFO_CAT(QString("Added custom event: %1 on %2/%3/%4 at %5")
        .arg(event.title)
        .arg(event.day)
        .arg(event.month)
        .arg(event.year == 0 ? "every year" : QString::number(event.year))
        .arg(event.triggerTime.toString("HH:mm")), "SpecialEvents");
}

MediaPlaylist SpecialEvents::getEventPlaylist() const
{
    if (!m_activeEvent) {
        return MediaPlaylist();
    }
    
    // Return the loaded playlist if available
    if (m_activePlaylist.hasItems()) {
        return m_activePlaylist;
    }
    
    // Fallback to single image for backward compatibility
    MediaPlaylist playlist;
    MediaItem item;
    item.type = "image";
    item.url = m_activeEvent->imageUrl;
    item.duration = m_activeEvent->durationSecs * 1000;
    item.muted = m_activeEvent->muted;
    playlist.items.append(item);
    return playlist;
}

void SpecialEvents::loadSpecialPlaylistsFromDirectory(const QString &dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        LOG_WARNING_CAT(QString("Special playlists directory not found: %1").arg(dirPath), "SpecialEvents");
        return;
    }
    
    // Look for *_playlist.json files
    QStringList filters;
    filters << "*_playlist.json";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    LOG_INFO_CAT(QString("Scanning for special playlists in: %1 (found %2 files)")
        .arg(dirPath)
        .arg(files.size()), "SpecialEvents");
    
    for (const QFileInfo &fileInfo : files) {
        QString filePath = fileInfo.absoluteFilePath();
        QFile file(filePath);
        
        if (!file.open(QIODevice::ReadOnly)) {
            LOG_WARNING_CAT(QString("Failed to open playlist file: %1").arg(filePath), "SpecialEvents");
            continue;
        }
        
        QByteArray data = file.readAll();
        file.close();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error != QJsonParseError::NoError || !doc.isObject()) {
            LOG_WARNING_CAT(QString("Failed to parse playlist JSON: %1 - %2")
                .arg(filePath).arg(error.errorString()), "SpecialEvents");
            continue;
        }
        
        QJsonObject obj = doc.object();
        
        // Only process if marked as special
        if (!obj["special"].toBool()) {
            continue;
        }
        
        SpecialEvent event;
        event.title = obj["title"].toString();
        event.playlistPath = filePath;
        event.muted = true; // Special events are muted by default
        
        // Parse date (YYYY-MM-DD format)
        QString dateStr = obj["date"].toString();
        QStringList dateParts = dateStr.split('-');
        if (dateParts.size() == 3) {
            event.year = dateParts[0].toInt();
            event.month = dateParts[1].toInt();
            event.day = dateParts[2].toInt();
        }
        
        // Parse trigger time from first item with custom_time
        QJsonArray items = obj["items"].toArray();
        for (const QJsonValue &itemValue : items) {
            QJsonObject itemObj = itemValue.toObject();
            QString customTime = itemObj["custom_time"].toString();
            if (!customTime.isEmpty() && customTime != "NA") {
                event.triggerTime = QTime::fromString(customTime, "HH:mm");
                break;
            }
        }
        
        // Calculate total duration from items
        event.durationSecs = 0;
        for (const QJsonValue &itemValue : items) {
            QJsonObject itemObj = itemValue.toObject();
            event.durationSecs += itemObj["duration"].toInt() / 1000; // Convert ms to seconds
        }
        
        if (event.triggerTime.isValid() && event.month > 0 && event.day > 0) {
            m_events.append(event);
            LOG_INFO_CAT(QString("Loaded special playlist: %1 on %2-%3-%4 at %5")
                .arg(event.title)
                .arg(event.year)
                .arg(event.month)
                .arg(event.day)
                .arg(event.triggerTime.toString("HH:mm")), "SpecialEvents");
        } else {
            LOG_WARNING_CAT(QString("Invalid event configuration in: %1").arg(filePath), "SpecialEvents");
        }
    }
    
    LOG_INFO_CAT(QString("Loaded %1 special events from directory").arg(m_events.size()), "SpecialEvents");
}

MediaPlaylist SpecialEvents::loadPlaylistFromFile(const QString &filePath) const
{
    MediaPlaylist playlist;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR_CAT(QString("Failed to open playlist file: %1").arg(filePath), "SpecialEvents");
        return playlist;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        LOG_ERROR_CAT(QString("Failed to parse playlist JSON: %1").arg(error.errorString()), "SpecialEvents");
        return playlist;
    }
    
    QJsonObject obj = doc.object();
    QJsonArray itemsArray = obj["items"].toArray();
    
    for (const QJsonValue &itemValue : itemsArray) {
        QJsonObject itemObj = itemValue.toObject();
        
        MediaItem item;
        item.type = itemObj["type"].toString();
        item.url = itemObj["url"].toString();
        item.duration = itemObj["duration"].toInt();
        item.muted = itemObj["muted"].toBool();
        // Parse optional custom_time field
        if (itemObj.contains("custom_time") && itemObj["custom_time"].isString()) {
            QString ct = itemObj["custom_time"].toString();
            if (!ct.isEmpty() && ct != "NA") {
                QTime t = QTime::fromString(ct, "HH:mm");
                if (t.isValid()) {
                    item.customTime = t;
                    item.hasCustomTime = true;
                }
            }
        }
        
        if (!item.type.isEmpty() && !item.url.isEmpty()) {
            playlist.items.append(item);
        }
    }
    
    // Mark as special playlist
    playlist.isSpecial = obj["special"].toBool(false);
    // Parse optional date and title
    if (obj.contains("date") && obj["date"].isString()) {
        QDate d = QDate::fromString(obj["date"].toString(), "yyyy-MM-dd");
        if (d.isValid()) playlist.specialDate = d;
    }
    if (obj.contains("title") && obj["title"].isString()) {
        playlist.title = obj["title"].toString();
    }
    
    LOG_DEBUG_CAT(QString("Loaded playlist with %1 items from %2")
        .arg(playlist.items.size())
        .arg(filePath), "SpecialEvents");
    
    return playlist;
}
