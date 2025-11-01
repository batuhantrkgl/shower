#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include "md3colors.h"

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

private:
    void setupUI();
    void updateConnectionIcon();
    void updatePingDisplay();

    QLabel *m_connectionIcon;
    QLabel *m_connectionText;
    QLabel *m_pingLabel;
    QLabel *m_timeLabel;
    QLabel *m_serverLabel;

    bool m_connected;
    int m_pingMs;
    QString m_serverUrl;
    QString m_hostname;
    QTimer *m_timeTimer;
};

#endif // STATUSBAR_H