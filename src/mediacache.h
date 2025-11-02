#ifndef MEDIACACHE_H
#define MEDIACACHE_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QQueue>
#include <QFile>
#include <QDir>
#include <QMutex>
#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>

struct CacheEntry {
    QString url;           // Original URL
    QString localPath;     // Path to cached file
    qint64 size;          // File size in bytes
    qint64 lastAccess;    // Timestamp of last access
    QString contentHash;   // Hash of the content for change detection
};

struct CacheStats {
    int hits = 0;
    int misses = 0;
    qint64 totalSize = 0;
    int itemCount = 0;
    qint64 maxSize = 0;
    
    double hitRate() const {
        int total = hits + misses;
        return total > 0 ? (static_cast<double>(hits) / total) * 100.0 : 0.0;
    }
};

class MediaCache : public QObject
{
    Q_OBJECT

public:
    explicit MediaCache(QObject *parent = nullptr);
    ~MediaCache();
    
    // Configuration
    void setMaxSize(qint64 sizeInBytes); // Set max cache size (2-8 GB)
    void setCacheDir(const QString &path); // Set cache directory
    qint64 getMaxSize() const { return m_maxSize; }
    QString getCacheDir() const { return m_cacheDir; }
    
    // Cache operations
    QString getCachedPath(const QString &url); // Get local path if cached, empty if not
    void cacheFile(const QString &url, const QByteArray &data); // Cache a file
    void prefetchUrl(const QString &url); // Asynchronously prefetch a URL
    bool isCached(const QString &url) const;
    
    // Cache management
    void clear(); // Clear entire cache
    void evictLRU(); // Remove least recently used items to stay under limit
    void updateAccess(const QString &url); // Update LRU timestamp
    
    // Statistics
    CacheStats getStats() const { return m_stats; }
    void recordHit() { m_stats.hits++; }
    void recordMiss() { m_stats.misses++; }
    
signals:
    void cacheUpdated();
    void prefetchComplete(const QString &url, bool success);

private slots:
    void onPrefetchFinished();

private:
    QString generateCacheKey(const QString &url) const;
    QString generateContentHash(const QByteArray &data) const;
    void loadCacheIndex();
    void saveCacheIndex();
    void ensureCacheDir();
    qint64 calculateCurrentSize() const;
    
    QHash<QString, CacheEntry> m_cache; // URL hash -> CacheEntry
    QMutex m_mutex; // Thread safety
    QString m_cacheDir;
    qint64 m_maxSize; // Maximum cache size in bytes (default 4GB)
    CacheStats m_stats;
    QNetworkAccessManager *m_networkManager;
};

#endif // MEDIACACHE_H
