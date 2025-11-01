#ifndef ACTIVITYOVERLAY_H
#define ACTIVITYOVERLAY_H

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>
#include <QTime>
#include "md3colors.h"

class ActivityOverlay : public QFrame
{
    Q_OBJECT

public:
    enum class ActivityStatus {
        OffHours,
        Break,
        InClass
    };

    explicit ActivityOverlay(QWidget *parent = nullptr);

public slots:
    void updateCurrentActivity(const QString &activityName);
    void updateStatus(ActivityStatus status);

private:
    void setupUI();
    void updateStatusColor(ActivityStatus status);
    
    QLabel *m_statusIcon;
    QLabel *m_activityLabel;
    ActivityStatus m_currentStatus;
    int m_borderRadius; // Store border radius value
};

#endif // ACTIVITYOVERLAY_H