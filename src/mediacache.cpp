#include "mediacache.h"
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QDebug>
#include <QNetworkRequest>
#include <QUrl>

MediaCache::MediaCache(QObject *parent)
    : QObject(parent)
    , m_maxSize(4LL * 1024 * 1024 * 1024) // Default 4GB
    , m_networkManager(new QNetworkAccessManager(this))
{
    // Set cache directory to user's cache location
    QString defaultCacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    m_cacheDir = defaultCacheDir + "/VideoTimeline/media";
    
    ensureCacheDir();
    loadCacheIndex();
    
    // Update stats
    m_stats.maxSize = m_maxSize;
    m_stats.totalSize = calculateCurrentSize();
    m_stats.itemCount = m_cache.size();
}

MediaCache::~MediaCache()
{
    saveCacheIndex();
}

void MediaCache::setMaxSize(qint64 sizeInBytes)
{
    QMutexLocker locker(&m_mutex);
    m_maxSize = sizeInBytes;
    m_stats.maxSize = sizeInBytes;
    
    // Evict items if we're now over the limit
    if (calculateCurrentSize() > m_maxSize) {
        evictLRU();
    }
}

void MediaCache::setCacheDir(const QString &path)
{
    QMutexLocker locker(&m_mutex);
    
    // Save current index before changing directory
    saveCacheIndex();
    
    m_cacheDir = path;
    ensureCacheDir();
    loadCacheIndex();
}

QString MediaCache::getCachedPath(const QString &url)
{
    QMutexLocker locker(&m_mutex);
    
    QString key = generateCacheKey(url);
    
    if (m_cache.contains(key)) {
        CacheEntry &entry = m_cache[key];
        
        // Check if file still exists
        if (QFile::exists(entry.localPath)) {
            // Update access time
            entry.lastAccess = QDateTime::currentMSecsSinceEpoch();
            recordHit();
            emit cacheUpdated();
            return entry.localPath;
        } else {
            // File was deleted, remove from cache
            m_cache.remove(key);
            m_stats.itemCount = m_cache.size();
        }
    }
    
    recordMiss();
    return QString();
}

void MediaCache::cacheFile(const QString &url, const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    
    QString key = generateCacheKey(url);
    QString hash = generateContentHash(data);
    
    // Check if we already have this exact content cached
    if (m_cache.contains(key)) {
        CacheEntry &existing = m_cache[key];
        if (existing.contentHash == hash && QFile::exists(existing.localPath)) {
            // Same content, just update access time
            existing.lastAccess = QDateTime::currentMSecsSinceEpoch();
            qDebug() << "Cache: Content unchanged for" << url;
            emit cacheUpdated();
            return;
        } else {
            // Content changed or file missing, remove old entry
            QFile::remove(existing.localPath);
            m_cache.remove(key);
        }
    }
    
    // Evict items if needed to make room
    qint64 dataSize = data.size();
    while (calculateCurrentSize() + dataSize > m_maxSize && !m_cache.isEmpty()) {
        evictLRU();
    }
    
    // Save the file
    QString localPath = m_cacheDir + "/" + key;
    QFile file(localPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
        
        // Add to cache
        CacheEntry entry;
        entry.url = url;
        entry.localPath = localPath;
        entry.size = dataSize;
        entry.lastAccess = QDateTime::currentMSecsSinceEpoch();
        entry.contentHash = hash;
        
        m_cache[key] = entry;
        
        // Update stats
        m_stats.totalSize = calculateCurrentSize();
        m_stats.itemCount = m_cache.size();
        
        qDebug() << "Cache: Stored" << url << "(" << dataSize << "bytes )";
        emit cacheUpdated();
        saveCacheIndex();
    } else {
        qWarning() << "Cache: Failed to write file:" << localPath;
    }
}

void MediaCache::prefetchUrl(const QString &url)
{
    qDebug() << "Cache: Prefetching" << url;
    
    // Check if already cached
    if (isCached(url)) {
        qDebug() << "Cache: Already cached, skipping prefetch";
        emit prefetchComplete(url, true);
        return;
    }
    
    QNetworkRequest request{QUrl(url)};
    request.setRawHeader("User-Agent", "VideoTimeline Client Cache");
    
    QNetworkReply *reply = m_networkManager->get(request);
    reply->setProperty("prefetch_url", url);
    connect(reply, &QNetworkReply::finished, this, &MediaCache::onPrefetchFinished);
}

bool MediaCache::isCached(const QString &url) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_mutex));
    QString key = generateCacheKey(url);
    
    if (m_cache.contains(key)) {
        const CacheEntry &entry = m_cache[key];
        return QFile::exists(entry.localPath);
    }
    
    return false;
}

void MediaCache::clear()
{
    QMutexLocker locker(&m_mutex);
    
    // Delete all cached files
    for (const CacheEntry &entry : m_cache) {
        QFile::remove(entry.localPath);
    }
    
    m_cache.clear();
    
    // Update stats
    m_stats.totalSize = 0;
    m_stats.itemCount = 0;
    m_stats.hits = 0;
    m_stats.misses = 0;
    
    saveCacheIndex();
    emit cacheUpdated();
    
    qDebug() << "Cache: Cleared all entries";
}

void MediaCache::evictLRU()
{
    if (m_cache.isEmpty()) {
        return;
    }
    
    // Find the least recently used entry
    QString oldestKey;
    qint64 oldestTime = QDateTime::currentMSecsSinceEpoch();
    
    for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
        if (it.value().lastAccess < oldestTime) {
            oldestTime = it.value().lastAccess;
            oldestKey = it.key();
        }
    }
    
    // Remove the oldest entry
    if (!oldestKey.isEmpty()) {
        CacheEntry entry = m_cache[oldestKey];
        QFile::remove(entry.localPath);
        m_cache.remove(oldestKey);
        
        qDebug() << "Cache: Evicted LRU item:" << entry.url << "(" << entry.size << "bytes)";
        
        // Update stats
        m_stats.totalSize = calculateCurrentSize();
        m_stats.itemCount = m_cache.size();
        emit cacheUpdated();
    }
}

void MediaCache::updateAccess(const QString &url)
{
    QMutexLocker locker(&m_mutex);
    QString key = generateCacheKey(url);
    
    if (m_cache.contains(key)) {
        m_cache[key].lastAccess = QDateTime::currentMSecsSinceEpoch();
    }
}

void MediaCache::onPrefetchFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    QString url = reply->property("prefetch_url").toString();
    bool success = false;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        cacheFile(url, data);
        success = true;
        qDebug() << "Cache: Prefetch complete for" << url;
    } else {
        qWarning() << "Cache: Prefetch failed for" << url << ":" << reply->errorString();
    }
    
    emit prefetchComplete(url, success);
    reply->deleteLater();
}

QString MediaCache::generateCacheKey(const QString &url) const
{
    // Generate a hash-based filename from the URL
    QByteArray hash = QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}

QString MediaCache::generateContentHash(const QByteArray &data) const
{
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    return hash.toHex();
}

void MediaCache::loadCacheIndex()
{
    QString indexPath = m_cacheDir + "/index.json";
    QFile file(indexPath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cache: No index file found, starting fresh";
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qWarning() << "Cache: Invalid index file format";
        return;
    }
    
    QJsonObject root = doc.object();
    QJsonArray entries = root["entries"].toArray();
    
    m_cache.clear();
    
    for (const QJsonValue &value : entries) {
        QJsonObject obj = value.toObject();
        
        CacheEntry entry;
        entry.url = obj["url"].toString();
        entry.localPath = obj["localPath"].toString();
        entry.size = obj["size"].toVariant().toLongLong();
        entry.lastAccess = obj["lastAccess"].toVariant().toLongLong();
        entry.contentHash = obj["contentHash"].toString();
        
        // Only load if file still exists
        if (QFile::exists(entry.localPath)) {
            QString key = generateCacheKey(entry.url);
            m_cache[key] = entry;
        }
    }
    
    qDebug() << "Cache: Loaded" << m_cache.size() << "entries from index";
}

void MediaCache::saveCacheIndex()
{
    QString indexPath = m_cacheDir + "/index.json";
    
    QJsonArray entries;
    for (const CacheEntry &entry : m_cache) {
        QJsonObject obj;
        obj["url"] = entry.url;
        obj["localPath"] = entry.localPath;
        obj["size"] = QJsonValue::fromVariant(entry.size);
        obj["lastAccess"] = QJsonValue::fromVariant(entry.lastAccess);
        obj["contentHash"] = entry.contentHash;
        entries.append(obj);
    }
    
    QJsonObject root;
    root["entries"] = entries;
    root["maxSize"] = QJsonValue::fromVariant(m_maxSize);
    
    QJsonDocument doc(root);
    
    QFile file(indexPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    } else {
        qWarning() << "Cache: Failed to save index file";
    }
}

void MediaCache::ensureCacheDir()
{
    QDir dir;
    if (!dir.exists(m_cacheDir)) {
        if (dir.mkpath(m_cacheDir)) {
            qDebug() << "Cache: Created directory:" << m_cacheDir;
        } else {
            qWarning() << "Cache: Failed to create directory:" << m_cacheDir;
        }
    }
}

qint64 MediaCache::calculateCurrentSize() const
{
    qint64 total = 0;
    for (const CacheEntry &entry : m_cache) {
        total += entry.size;
    }
    return total;
}
