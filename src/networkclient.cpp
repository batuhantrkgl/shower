#include "networkclient.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QEventLoop>
#include <QTimer>
#include <QTime>
#include <QElapsedTimer>
#include <QDateTime>

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_serverUrl("http://localhost:3232")  // Default fallback
    , m_fetchTimer(new QTimer(this))
    , m_discovered(false)
    , m_connected(false)
    , m_lastPingMs(-1)
    , m_pingTimer(new QTimer(this))
    , m_reconnectTimer(new QTimer(this))
{
    // Set up periodic fetch timer (every 5 minutes)
    m_fetchTimer->setInterval(5 * 60 * 1000);
    connect(m_fetchTimer, &QTimer::timeout, this, &NetworkClient::periodicFetch);
    
    // Set up ping timer (every 30 seconds)
    m_pingTimer->setInterval(30 * 1000);
    connect(m_pingTimer, &QTimer::timeout, this, &NetworkClient::measurePing);
    
    // Set up reconnection timer (every 10 seconds when disconnected)
    m_reconnectTimer->setInterval(10 * 1000);
    connect(m_reconnectTimer, &QTimer::timeout, this, &NetworkClient::attemptReconnection);
}

void NetworkClient::setServerUrl(const QString &url)
{
    m_serverUrl = url;
    m_connected = false; // Reset connection status
    m_hostname.clear(); // Clear hostname
    emit connectionStatusChanged(false);
    // Start reconnection attempts
    if (!m_reconnectTimer->isActive()) {
        m_reconnectTimer->start();
    }
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
    m_pingTimer->start();
}

void NetworkClient::stopPeriodicFetch()
{
    m_fetchTimer->stop();
    m_pingTimer->stop();
    m_reconnectTimer->stop();
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
            if (!m_connected) {
                m_connected = true;
                emit connectionStatusChanged(true, m_serverUrl, m_hostname);
                m_reconnectTimer->stop(); // Stop reconnection attempts
            }
        } else {
            qDebug() << "JSON parse error:" << error.errorString();
            emit networkError("Failed to parse schedule JSON");
            // Use default schedule as fallback
            emit scheduleReceived(QTime(8, 50), QTime(15, 55), createDefaultSchedule());
            if (m_connected) {
                m_connected = false;
                emit connectionStatusChanged(false);
                // Start reconnection attempts
                if (!m_reconnectTimer->isActive()) {
                    m_reconnectTimer->start();
                }
            }
        }
    } else {
        qDebug() << "Network error:" << reply->errorString();
        emit networkError("Failed to fetch schedule: " + reply->errorString());
        // Use default schedule as fallback
        emit scheduleReceived(QTime(8, 50), QTime(15, 55), createDefaultSchedule());
        if (m_connected) {
            m_connected = false;
            emit connectionStatusChanged(false);
            // Start reconnection attempts
            if (!m_reconnectTimer->isActive()) {
                m_reconnectTimer->start();
            }
        }
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
            if (!m_connected) {
                m_connected = true;
                emit connectionStatusChanged(true, m_serverUrl, m_hostname);
                m_reconnectTimer->stop(); // Stop reconnection attempts
            }
        } else {
            qDebug() << "JSON parse error:" << error.errorString();
            emit networkError("Failed to parse playlist JSON");
            if (m_connected) {
                m_connected = false;
                emit connectionStatusChanged(false);
                // Start reconnection attempts
                if (!m_reconnectTimer->isActive()) {
                    m_reconnectTimer->start();
                }
            }
        }
    } else {
        qDebug() << "Network error:" << reply->errorString();
        emit networkError("Failed to fetch playlist: " + reply->errorString());
        if (m_connected) {
            m_connected = false;
            emit connectionStatusChanged(false);
            // Start reconnection attempts
            if (!m_reconnectTimer->isActive()) {
                m_reconnectTimer->start();
            }
        }
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
    
    // Extract server information
    m_hostname = json["server_hostname"].toString();
    
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

void NetworkClient::discoverAndSetServer()
{
    if (m_discovered) {
        return;
    }
    
    qDebug() << "Starting server discovery...";
    
    // Strategy: Try common IPs first, then scan local network
    QString networkPrefix = getLocalNetworkPrefix();
    
    // Priority 1: Try common/predictable IPs first (much faster)
    QStringList commonIPs = {
        QString("%1.1:3232").arg(networkPrefix.isEmpty() ? "192.168.1" : networkPrefix),
        QString("%1.100:3232").arg(networkPrefix.isEmpty() ? "192.168.1" : networkPrefix),
        QString("%1.254:3232").arg(networkPrefix.isEmpty() ? "192.168.1" : networkPrefix),
        "192.168.1.1:3232",
        "192.168.1.100:3232",
        "192.168.0.1:3232",
        "10.135.176.176:3232",
        "10.0.0.1:3232",
        "10.0.1.1:3232",
        "10.1.1.1:3232",
        "10.10.10.1:3232",
        "localhost:3232"
    };
    
    for (const QString &url : commonIPs) {
        if (tryServerUrl("http://" + url)) {
            m_serverUrl = "http://" + url;
            m_discovered = true;
            qDebug() << "Found server at:" << m_serverUrl;
            emit serverDiscovered(m_serverUrl);
            return;
        }
    }
    
    // Priority 2: Scan local network (only if we detected the network prefix)
    if (!networkPrefix.isEmpty()) {
        qDebug() << "Scanning local network:" << networkPrefix << ".*";
        for (int i = 1; i <= 254; i++) {
            QString testUrl = QString("http://%1.%2:3232").arg(networkPrefix).arg(i);
            if (tryServerUrl(testUrl)) {
                m_serverUrl = testUrl;
                m_discovered = true;
                qDebug() << "Found server at:" << m_serverUrl;
                emit serverDiscovered(m_serverUrl);
                return;
            }
        }
    }
    
    // Priority 3: Try other common private networks if local scan failed
    qDebug() << "Local network scan failed, trying other common subnets...";
    QStringList commonSubnets = {"192.168.0", "192.168.32", "10.0.0"};
    for (const QString &subnet : commonSubnets) {
        if (subnet == networkPrefix) {
            continue;  // Skip if already scanned
        }
        qDebug() << "Scanning subnet:" << subnet << ".*";
        for (int i = 1; i <= 254; i++) {
            QString testUrl = QString("http://%1.%2:3232").arg(subnet).arg(i);
            if (tryServerUrl(testUrl)) {
                m_serverUrl = testUrl;
                m_discovered = true;
                qDebug() << "Found server at:" << m_serverUrl;
                emit serverDiscovered(m_serverUrl);
                return;
            }
        }
    }
    
    // Priority 4: Scan common 10.*.*.* network ranges for server discovery
    qDebug() << "Scanning common 10.*.*.* network ranges...";
    QStringList common10Subnets = {
        "10.0.0", "10.0.1", "10.1.0", "10.1.1", "10.10.10", 
        "10.0.10", "10.1.10", "10.10.0", "10.10.1", "10.100.100"
    };
    
    for (const QString &subnet : common10Subnets) {
        qDebug() << "Scanning 10.x subnet:" << subnet << ".*";
        for (int i = 1; i <= 254; i++) {
            QString testUrl = QString("http://%1.%2:3232").arg(subnet).arg(i);
            if (tryServerUrl(testUrl)) {
                m_serverUrl = testUrl;
                m_discovered = true;
                qDebug() << "Found server at:" << m_serverUrl;
                emit serverDiscovered(m_serverUrl);
                return;
            }
        }
    }
    
    qDebug() << "Server discovery failed, using default:" << m_serverUrl;
}

void NetworkClient::discoverInRange(const QString &networkPrefix)
{
    qDebug() << "Scanning specific network range:" << networkPrefix << ".*";
    
    for (int i = 1; i <= 254; i++) {
        QString testUrl = QString("http://%1.%2:3232").arg(networkPrefix).arg(i);
        if (tryServerUrl(testUrl)) {
            m_serverUrl = testUrl;
            m_discovered = true;
            qDebug() << "Found server at:" << m_serverUrl;
            emit serverDiscovered(m_serverUrl);
            return;
        }
    }
    
    qDebug() << "No server found in range:" << networkPrefix << ".*";
}

void NetworkClient::setSpecificServer(const QString &serverUrl)
{
    QString url = serverUrl;
    
    // Add http:// prefix if not present
    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        url = "http://" + url;
    }
    
    qDebug() << "Testing specific server:" << url;
    
    if (tryServerUrl(url)) {
        m_serverUrl = url;
        m_discovered = true;
        qDebug() << "Successfully connected to server at:" << m_serverUrl;
        emit serverDiscovered(m_serverUrl);
    } else {
        qDebug() << "Failed to connect to specified server:" << url;
        qDebug() << "Falling back to auto-discovery...";
        discoverAndSetServer();
    }
}

QString NetworkClient::getLocalNetworkPrefix()
{
    // Find the first active IPv4 interface to determine local network
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    
    for (const QNetworkInterface &interface : interfaces) {
        // Skip loopback and inactive interfaces
        if (interface.flags().testFlag(QNetworkInterface::IsLoopBack) || 
            !interface.flags().testFlag(QNetworkInterface::IsRunning)) {
            continue;
        }
        
        // Check all addresses on this interface
        for (const QNetworkAddressEntry &entry : interface.addressEntries()) {
            QHostAddress addr = entry.ip();
            
            // Only process IPv4 addresses
            if (addr.protocol() != QAbstractSocket::IPv4Protocol) {
                continue;
            }
            
            QString ipString = addr.toString();
            // Extract network prefix (e.g., "192.168.1" from "192.168.1.42")
            int lastDot = ipString.lastIndexOf('.');
            if (lastDot != -1) {
                return ipString.left(lastDot);
            }
        }
    }
    
    return QString();  // No IPv4 interface found
}

bool NetworkClient::tryServerUrl(const QString &url)
{
    QNetworkRequest request(QUrl(url + "/api/schedule"));
    request.setRawHeader("User-Agent", "VideoTimeline Client");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = m_networkManager->get(request);
    
    // Wait for response with a timeout (short timeout for fast scanning)
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    timeout.setInterval(300);  // 300ms timeout per IP for faster scanning
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    
    timeout.start();
    loop.exec();
    
    bool found = false;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            // Check if it looks like our server (has schedule structure)
            if (obj.contains("school_start") || obj.contains("blocks")) {
                found = true;
            }
        }
    }
    
    reply->deleteLater();
    return found;
}

void NetworkClient::measurePing()
{
    if (!m_connected) return;
    
    QNetworkRequest request(QUrl(m_serverUrl + "/api/schedule"));
    request.setRawHeader("User-Agent", "VideoTimeline Client");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    // Store the actual timestamp when request starts
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    
    // Use HEAD request for faster ping measurement (no body download)
    QNetworkReply *reply = m_networkManager->head(request);
    reply->setProperty("ping_start", startTime);
    connect(reply, &QNetworkReply::finished, this, &NetworkClient::onPingReplyFinished);
}

void NetworkClient::onPingReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    qint64 startTime = reply->property("ping_start").toLongLong();
    qint64 endTime = QDateTime::currentMSecsSinceEpoch();
    qint64 pingTime = endTime - startTime;
    
    if (reply->error() == QNetworkReply::NoError) {
        m_lastPingMs = static_cast<int>(pingTime);
        emit pingUpdated(m_lastPingMs);
    } else {
        // Connection lost
        if (m_connected) {
            m_connected = false;
            emit connectionStatusChanged(false);
            // Start reconnection attempts
            if (!m_reconnectTimer->isActive()) {
                m_reconnectTimer->start();
            }
        }
    }
    
    reply->deleteLater();
}

void NetworkClient::attemptReconnection()
{
    if (m_connected) {
        // Already connected, stop reconnection attempts
        m_reconnectTimer->stop();
        return;
    }
    
    qDebug() << "Attempting to reconnect to server:" << m_serverUrl;
    
    // Try to fetch schedule to test connection
    QNetworkRequest request(QUrl(m_serverUrl + "/api/schedule"));
    request.setRawHeader("User-Agent", "VideoTimeline Client");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(data, &error);
            
            if (error.error == QJsonParseError::NoError && doc.isObject()) {
                // Successfully reconnected
                parseScheduleJson(doc.object());
                if (!m_connected) {
                    m_connected = true;
                    emit connectionStatusChanged(true, m_serverUrl, m_hostname);
                    m_reconnectTimer->stop();
                    qDebug() << "Successfully reconnected to server";
                    
                    // Restart normal operations
                    fetchCurrentMedia();
                    m_fetchTimer->start();
                    m_pingTimer->start();
                }
            }
        }
        reply->deleteLater();
    });
}
