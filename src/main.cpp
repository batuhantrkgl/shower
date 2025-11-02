#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFont>
#include <QFontDatabase>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTextStream>
#include <QDebug> // Include QDebug for debug output

#ifndef QT_STRINGIFY
#define QT_STRINGIFY2(x) #x
#define QT_STRINGIFY(x) QT_STRINGIFY2(x)
#endif

// Fetch the version info passed from the .pro file
const char* appVersion = QT_STRINGIFY(APP_VERSION);
const char* appReleaseDate = QT_STRINGIFY(APP_RELEASE_DATE);
const char* appBuildId = QT_STRINGIFY(APP_BUILD_ID);


// --- Start of Color Edit ---
// Namespace for ANSI TTY color codes for the console output
namespace TTY {
    const char* const Reset   = "\033[0m";
    const char* const Cyan    = "\033[1;36m"; // Bold Cyan
    const char* const Yellow  = "\033[0;33m";
    const char* const Green   = "\033[0;32m";
    const char* const White   = "\033[1;37m"; // Bold White
}
// --- End of Color Edit ---


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Set application properties
    a.setApplicationName("VideoTimeline");
    a.setApplicationVersion(appVersion);
    a.setOrganizationName("Batuhantrkgl");

    // Use QCommandLineParser to handle arguments like --version
    QCommandLineParser parser;
    parser.setApplicationDescription("A custom video timeline display application.");
    parser.addHelpOption();
    
    QCommandLineOption versionOption(QStringList() << "v" << "version", "Displays application version and build information.");
    parser.addOption(versionOption);
    
    QCommandLineOption autoOption(QStringList() << "auto", "Automatically discover and connect to the server.");
    parser.addOption(autoOption);
    
    QCommandLineOption networkOption(QStringList() << "network", "Connect to specific server URL (e.g., 10.135.176.176:3232) or scan network range (e.g., 10.1.1 for 10.1.1.*:3232).", "server_or_range");
    parser.addOption(networkOption);
    
    QCommandLineOption dpiOption(QStringList() << "dpi", "Override screen DPI for testing UI scaling (e.g., 96, 144, 192).", "dpi_value");
    parser.addOption(dpiOption);
    
    QCommandLineOption testTimeOption(QStringList() << "test-time", "Force a specific time for testing time-based UI states (e.g., '06:00' for off-hours, '09:00' for school hours).", "time_value");
    parser.addOption(testTimeOption);

    parser.process(a);
    
    // If --version or -v was passed, print info and exit gracefully
    if (parser.isSet(versionOption)) {
        QTextStream out(stdout);

        // --- Start of Color Edit ---
        // Print the version string with added colors
        out << TTY::White << "Version " << TTY::Cyan << appVersion
            << TTY::White << " (" << TTY::Yellow << appReleaseDate << TTY::White << ") - Build ID: "
            << TTY::Cyan << appBuildId << TTY::Reset << "\n";

        out << "Written in C++ using the Qt Framework, and inspired by my love for Duru.\n";

        out << TTY::Green << "Made by @Batuhantrkgl" << TTY::Reset << "\n";
        // --- End of Color Edit ---
        
        out.flush();
        return 0;
    }

    a.setStyle(QStyleFactory::create("Fusion"));

    QFontDatabase fontDb;
    QFont appFont;
    QString selectedFontName;

    // Try SF Pro Display first (Apple font)
    if (fontDb.families().contains("SF Pro Display")) {
        appFont = QFont("SF Pro Display", 16, QFont::Normal);
        selectedFontName = "SF Pro Display";
    } else if (fontDb.families().contains("Inter")) {
        appFont = QFont("Inter", 14, QFont::Normal);
        selectedFontName = "Inter";
    } else if (fontDb.families().contains("Roboto")) {
        appFont = QFont("Roboto", 14, QFont::Normal);
        selectedFontName = "Roboto";
    } else {
        appFont = a.font();
        appFont.setPointSize(14);
        selectedFontName = appFont.family() + " (system default)";
    }

    a.setFont(appFont);
    
    // Log font selection with TTY colors for visibility
    QTextStream out(stdout);
    out << TTY::Cyan << "[FONT] " << TTY::Reset 
        << "Using font: " << TTY::Yellow << selectedFontName << TTY::Reset 
        << " at " << TTY::Green << appFont.pointSize() << "pt" << TTY::Reset << "\n";
    out.flush();

    QString networkRange = parser.value(networkOption);
    qreal forcedDpi = parser.value(dpiOption).toDouble();
    QString testTimeStr = parser.value(testTimeOption);
    
    MainWindow w(parser.isSet(autoOption), networkRange, forcedDpi, testTimeStr);
    w.showFullScreen();

    return a.exec();
}
