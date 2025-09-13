#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFont>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Set application properties for Material Design 3
    a.setApplicationName("VideoTimeline");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("MD3 Apps");
    
    // Set the application style to a modern one
    a.setStyle(QStyleFactory::create("Fusion"));
    
    // Load system fonts (Roboto if available, otherwise default to system font)
    QFontDatabase fontDb;
    QFont appFont;
    
    // Try to use Roboto font family or fall back to system font
    if (fontDb.families().contains("Roboto")) {
        appFont = QFont("Roboto", 14, QFont::Normal);
    } else {
        appFont = a.font();
        appFont.setPointSize(14);
    }
    
    a.setFont(appFont);
    
    // Note: High DPI support is enabled by default in Qt 6
    
    MainWindow w;
    w.showFullScreen(); // Ensure fullscreen mode
    
    return a.exec();
}
