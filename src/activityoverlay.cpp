#include "activityoverlay.h"
#include "mainwindow.h"
#include "md3colors.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>
#include <QFontDatabase>
#include <QTimer>
#include <QPainterPath>
#include <QRegion>
#include <iostream>

ActivityOverlay::ActivityOverlay(QWidget *parent) : QFrame(parent)
{
    // Set widget attributes to ensure proper rendering
    setAttribute(Qt::WA_StyledBackground, true);
    
    // Use ToolTip window type with transparent background for rounded corners
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_TranslucentBackground, true); // Enable transparency for rounded corners
    setAttribute(Qt::WA_ShowWithoutActivating);
    
    setupUI();
}

void ActivityOverlay::setupUI()
{
    // Get screen DPI for scaling
    qreal dpi = MainWindow::getDpiForScreen(this);
    qreal scaleFactor = dpi / 96.0;

    // Scale dimensions to match timeline widget
    int iconSize = qRound(16 * scaleFactor); // Larger for better visibility
    int padding = qRound(12 * scaleFactor); // More padding
    int fontSize = qRound(12 * scaleFactor);
    m_borderRadius = qRound(8 * scaleFactor); // Match timeline widget (8px)
    
    // Main frame is transparent (just a container)
    setStyleSheet("QFrame { background-color: transparent; }");
    
    // Create inner container widget for the actual styled background
    QWidget *container = new QWidget(this);
    container->setAutoFillBackground(true);
    container->setStyleSheet(QString(
        "QWidget {"
        "    background-color: %1;"
        "    border-radius: %2px;"
        "}"
    ).arg(MD3Colors::DarkTheme::surfaceContainerHigh().name())
     .arg(m_borderRadius));

    QHBoxLayout *containerLayout = new QHBoxLayout(container);
    containerLayout->setContentsMargins(padding, padding, padding, padding);
    containerLayout->setSpacing(qRound(8 * scaleFactor));

    // Status indicator dot (circular)
    m_statusIcon = new QLabel("●", container);
    m_statusIcon->setFixedSize(iconSize, iconSize);
    m_statusIcon->setAlignment(Qt::AlignCenter);
    m_statusIcon->setStyleSheet("background: transparent;"); // Transparent background for dot
    // Make the font size larger so the dot fills the space better
    QFont dotFont = m_statusIcon->font();
    dotFont.setPointSize(qRound(12 * scaleFactor));
    m_statusIcon->setFont(dotFont);
    updateStatusColor(ActivityStatus::InClass); // Default to in-class color

    // Activity label - with even lighter background
    m_activityLabel = new QLabel("Loading...", container);
    m_activityLabel->setObjectName("activityLabel");
    m_activityLabel->setAlignment(Qt::AlignCenter);
    
    // Label uses surfaceContainerHighest (lighter than container)
    m_activityLabel->setStyleSheet(QString(
        "QLabel#activityLabel {"
        "    background-color: %1;"
        "    color: %2;"
        "    padding: 6px 16px;"
        "    border-radius: %3px;"
        "    font-weight: 500;"
        "}"
    ).arg(MD3Colors::DarkTheme::surfaceContainerHighest().name())
     .arg(MD3Colors::DarkTheme::onSurface().name())
     .arg(m_borderRadius));
    
    // Set label font properties
    QFont labelFont = m_activityLabel->font();
    labelFont.setPointSize(fontSize);
    labelFont.setWeight(QFont::Medium);
    m_activityLabel->setFont(labelFont);

    containerLayout->addWidget(m_statusIcon);
    containerLayout->addWidget(m_activityLabel);
    
    // Main layout wraps the container with no margins
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(container);
    
    // Let it size dynamically based on content
    adjustSize();
}

void ActivityOverlay::updateCurrentActivity(const QString &activityName)
{
    QString currentText = m_activityLabel->text();
    
    std::cout << "ActivityOverlay::updateCurrentActivity called with: " 
              << activityName.toStdString() << " current: " 
              << currentText.toStdString() << std::endl;
    
    // Only update if the activity actually changed
    if (currentText != activityName) {
        m_activityLabel->setText(activityName);
        
        // Update dot color based on activity type (matching timeline logic)
        if (activityName.contains("Ders")) {
            updateStatusColor(ActivityStatus::InClass); // Blue for lessons
        } else if (activityName.contains("Teneffüs") || activityName.contains("Öğle")) {
            updateStatusColor(ActivityStatus::Break); // Green/Orange for breaks
        } else {
            updateStatusColor(ActivityStatus::OffHours); // Gray for off hours
        }
        
        // Resize the widget to fit the new text
        adjustSize();
        updateGeometry();

        std::cout << "ActivityOverlay updated, new size: " << width() << "x" << height() 
                  << " visible: " << isVisible() << std::endl;

        // Always show the overlay with activity information
        show();
    }
}

void ActivityOverlay::updateStatus(ActivityStatus status)
{
    m_currentStatus = status;
    updateStatusColor(status);
}

void ActivityOverlay::updateStatusColor(ActivityStatus status)
{
    QString color;
    switch (status) {
        case ActivityStatus::OffHours:
            color = "#666666"; // Gray for off hours
            break;
        case ActivityStatus::Break:
            color = "#FF9800"; // Orange for breaks (matching timeline)
            break;
        case ActivityStatus::InClass:
            color = "#2196F3"; // Blue for in class (matching timeline)
            break;
    }
    
    m_statusIcon->setStyleSheet(QString("QLabel { background: transparent; color: %1; }").arg(color));
}