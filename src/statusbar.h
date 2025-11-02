#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QMenu>
#include "md3colors.h"
#include "mediacache.h"

class StatusBar : public QWidget
{
    Q_OBJECT

public:
    explicit StatusBar(QWidget *parent = nullptr);
    ~StatusBar();

public slots:
    void setConnectionStatus(bool connected, const QString &serverUrl = QString(), const QString &hostname = QString());
    void setPing(int pingMs);
    void updateTime();
    void setCodecInfo(const QString &codec, bool hwDecode);
    void setCacheStats(const CacheStats &stats);
    void setOfflineMode(bool offline);

signals:
    void toggleDiagnostics();
    void logLevelChangeRequested(const QString &level);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    void setupUI();
    void updateConnectionIcon();
    void updatePingDisplay();
    void updateCodecDisplay();
    void updateCacheDisplay();
    void showContextMenu(const QPoint &pos);

    QLabel *m_connectionIcon;
    QLabel *m_connectionText;
    QLabel *m_pingLabel;
    QLabel *m_codecLabel;
    QLabel *m_hwDecodeIcon;
    QLabel *m_cacheLabel;
    QLabel *m_offlineIcon;
    QLabel *m_versionLabel;
    QLabel *m_timeLabel;
    QLabel *m_serverLabel;

    bool m_connected;
    int m_pingMs;
    QString m_serverUrl;
    QString m_hostname;
    QString m_codec;
    bool m_hwDecode;
    CacheStats m_cacheStats;
    bool m_offlineMode;
    QTimer *m_timeTimer;
    QMenu *m_contextMenu;
};

#endif // STATUSBAR_H