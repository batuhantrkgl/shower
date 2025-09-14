#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFont>
#include <QFontDatabase>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTextStream>

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

    if (fontDb.families().contains("Inter")) {
        appFont = QFont("Inter", 14, QFont::Normal);
    } else if (fontDb.families().contains("Roboto")) {
        appFont = QFont("Roboto", 14, QFont::Normal);
    } else {
        appFont = a.font();
        appFont.setPointSize(14);
    }

    a.setFont(appFont);

    MainWindow w;
    w.showFullScreen();

    return a.exec();
}
