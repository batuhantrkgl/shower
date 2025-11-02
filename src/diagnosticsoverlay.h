#ifndef DIAGNOSTICSOVERLAY_H
#define DIAGNOSTICSOVERLAY_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QMediaPlayer>
#include "mediacache.h"

struct DiagnosticsInfo {
    // Network
    QString serverUrl;
    QString hostname;
    int pingMs = -1;
    bool connected = false;
    
    // Media
    QString currentCodec;
    bool hardwareDecodeEnabled = false;
    QString resolution;
    qreal fps = 0.0;
    QString currentItemSource;
    QMediaPlayer::MediaStatus mediaStatus = QMediaPlayer::NoMedia;
    
    // Cache
    int cacheHits = 0;
    int cacheMisses = 0;
    double cacheHitRate = 0.0;
    qint64 cacheSize = 0;
    int cacheItemCount = 0;
    
    // System
    QString appVersion;
    QString buildId;
    QString buildDate;
};

class DiagnosticsOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit DiagnosticsOverlay(QWidget *parent = nullptr);
    ~DiagnosticsOverlay();
    
    void setVisible(bool visible) override;
    void updateInfo(const DiagnosticsInfo &info);
    
    // Update individual fields
    void setServerInfo(const QString &url, const QString &hostname, int pingMs, bool connected);
    void setMediaInfo(const QString &codec, bool hwDecode, const QString &resolution, qreal fps);
    void setCurrentSource(const QString &source);
    void setCacheStats(const CacheStats &stats);
    void setMediaStatus(QMediaPlayer::MediaStatus status);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupUI();
    void updateDisplay();
    QString formatSize(qint64 bytes) const;
    QString getMediaStatusString(QMediaPlayer::MediaStatus status) const;
    
    DiagnosticsInfo m_info;
    
    // UI Components
    QLabel *m_titleLabel;
    
    // Network section
    QLabel *m_serverUrlLabel;
    QLabel *m_hostnameLabel;
    QLabel *m_pingLabel;
    QLabel *m_connectionLabel;
    
    // Media section
    QLabel *m_codecLabel;
    QLabel *m_hwDecodeLabel;
    QLabel *m_resolutionLabel;
    QLabel *m_fpsLabel;
    QLabel *m_sourceLabel;
    QLabel *m_statusLabel;
    
    // Cache section
    QLabel *m_cacheHitRateLabel;
    QLabel *m_cacheHitsLabel;
    QLabel *m_cacheMissesLabel;
    QLabel *m_cacheSizeLabel;
    QLabel *m_cacheCountLabel;
    
    // System section
    QLabel *m_versionLabel;
    QLabel *m_buildLabel;
    
    QTimer *m_updateTimer;
};

#endif // DIAGNOSTICSOVERLAY_H
