#ifndef QT6COMPAT_H
#define QT6COMPAT_H

#include <QtGlobal>

// Qt6 compatibility macros and includes
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #define QT6_OR_LATER
#endif

// Handle QMediaPlayer changes between Qt5 and Qt6
#ifdef QT6_OR_LATER
    #include <QMediaPlayer>
    #include <QAudioOutput>
    #include <QVideoWidget>
    
    // Qt6 uses different signal signatures
    #define MEDIAPLAYER_ERROR_SIGNAL QOverload<QMediaPlayer::Error, const QString &>::of(&QMediaPlayer::errorOccurred)
    
    // Qt6 requires explicit QAudioOutput setup
    #define SETUP_AUDIO_OUTPUT(player) \
        do { \
            QAudioOutput *audioOutput = new QAudioOutput(player->parent()); \
            player->setAudioOutput(audioOutput); \
        } while(0)
    
    // Qt6 uses setSource instead of setMedia
    #define SET_MEDIA_SOURCE(player, url) player->setSource(url)
    
    // Qt6 audio muting
    #define SET_AUDIO_MUTED(player, muted) \
        do { \
            QAudioOutput *audio = player->audioOutput(); \
            if (audio) audio->setMuted(muted); \
        } while(0)
        
#else
    // Qt5 compatibility
    #include <QMediaPlayer>
    #include <QVideoWidget>
    
    #define MEDIAPLAYER_ERROR_SIGNAL QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error)
    
    #define SETUP_AUDIO_OUTPUT(player) // No-op for Qt5
    
    #define SET_MEDIA_SOURCE(player, url) player->setMedia(QMediaContent(url))
    
    #define SET_AUDIO_MUTED(player, muted) player->setMuted(muted)
#endif

// Network request compatibility
#include <QNetworkRequest>
#include <QUrl>

// Safe network request creation for both Qt5 and Qt6
inline QNetworkRequest createNetworkRequest(const QUrl &url) {
    QNetworkRequest request;
    request.setUrl(url);
    return request;
}

inline QNetworkRequest createNetworkRequest(const QString &url) {
    return createNetworkRequest(QUrl(url));
}

// String-based URL creation
inline QUrl createUrl(const QString &urlString) {
    return QUrl(urlString);
}

// File path handling for cross-platform compatibility
#include <QString>
#include <QDir>

inline QString convertMediaPath(const QString &serverPath) {
    QString path = serverPath;
    if (path.startsWith("/media/")) {
        // Convert server path to local relative path
        path = path.mid(7); // Remove "/media/" prefix
    }
    return QDir::toNativeSeparators(path);
}

// Debug output compatibility
#include <QDebug>

#define COMPAT_DEBUG(msg) qDebug() << "[Qt" << QT_VERSION_STR << "]" << msg

// Timer compatibility
#include <QTimer>

inline void startSingleShotTimer(QTimer *timer, int msec) {
    timer->setSingleShot(true);
    timer->start(msec);
}

// Layout compatibility
#include <QStackedLayout>

// Ensure consistent widget indices
enum StackedLayoutIndices {
    VIDEO_WIDGET_INDEX = 0,
    IMAGE_WIDGET_INDEX = 1
};

// Error handling macros
#define HANDLE_MEDIA_ERROR(error, errorString) \
    do { \
        COMPAT_DEBUG("Media Error:" << error << errorString); \
    } while(0)

#define HANDLE_NETWORK_ERROR(error, errorString) \
    do { \
        COMPAT_DEBUG("Network Error:" << error << errorString); \
    } while(0)

// Version checking utilities
inline bool isQt6OrLater() {
#ifdef QT6_OR_LATER
    return true;
#else
    return false;
#endif
}

inline QString getQtVersionString() {
    return QString(QT_VERSION_STR);
}

// Build information
#ifndef APP_VERSION
#define APP_VERSION "1.1.0"
#endif

#ifndef APP_BUILD_ID
#define APP_BUILD_ID "unknown"
#endif

#ifndef APP_RELEASE_DATE
#define APP_RELEASE_DATE "unknown"
#endif

// Preprocessor utilities for conditional compilation
#ifdef QT6_OR_LATER
    #define IF_QT6(code) code
    #define IF_QT5(code)
#else
    #define IF_QT6(code)
    #define IF_QT5(code) code
#endif

// Common includes that are safe across Qt versions
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QString>
#include <QPixmap>
#include <QList>
#include <QTime>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#endif // QT6COMPAT_H