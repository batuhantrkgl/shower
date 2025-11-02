#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include <QDebug>

enum class LogLevel {
    Error,
    Warning,
    Info,
    Debug
};

class Logger : public QObject
{
    Q_OBJECT

public:
    static Logger& instance();
    
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const { return m_logLevel; }
    void enableFileLogging(bool enable);
    void setLogDir(const QString &dir);
    void setMaxLogFiles(int count) { m_maxLogFiles = count; }
    void setMaxLogSize(qint64 bytes) { m_maxLogSize = bytes; }
    
    void log(LogLevel level, const QString &message, const QString &category = QString());
    
    // Convenience functions
    void error(const QString &message, const QString &category = QString());
    void warning(const QString &message, const QString &category = QString());
    void info(const QString &message, const QString &category = QString());
    void debug(const QString &message, const QString &category = QString());
    
    // Log level from string
    static LogLevel levelFromString(const QString &str);
    static QString levelToString(LogLevel level);

signals:
    void logMessageEmitted(LogLevel level, const QString &message);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void writeToFile(const QString &formattedMessage);
    void rotateLogIfNeeded();
    void cleanupOldLogs();
    QString getLogFileName() const;
    
    LogLevel m_logLevel;
    bool m_fileLoggingEnabled;
    QString m_logDir;
    QFile *m_logFile;
    QTextStream *m_logStream;
    QMutex m_mutex;
    int m_maxLogFiles;
    qint64 m_maxLogSize;
};

// Global logging macros for convenience
#define LOG_ERROR(msg) Logger::instance().error(msg)
#define LOG_WARNING(msg) Logger::instance().warning(msg)
#define LOG_INFO(msg) Logger::instance().info(msg)
#define LOG_DEBUG(msg) Logger::instance().debug(msg)

#define LOG_ERROR_CAT(msg, cat) Logger::instance().error(msg, cat)
#define LOG_WARNING_CAT(msg, cat) Logger::instance().warning(msg, cat)
#define LOG_INFO_CAT(msg, cat) Logger::instance().info(msg, cat)
#define LOG_DEBUG_CAT(msg, cat) Logger::instance().debug(msg, cat)

#endif // LOGGER_H
