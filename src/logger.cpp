#include "logger.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QtGlobal>

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

Logger::Logger()
    : m_logLevel(LogLevel::Info)
    , m_fileLoggingEnabled(false)
    , m_logFile(nullptr)
    , m_logStream(nullptr)
    , m_maxLogFiles(5)
    , m_maxLogSize(10 * 1024 * 1024) // 10 MB per log file
{
    // Default log directory
    QString defaultLogDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_logDir = defaultLogDir + "/logs";
    
    // Ensure log directory exists
    QDir dir;
    if (!dir.exists(m_logDir)) {
        dir.mkpath(m_logDir);
    }
}

Logger::~Logger()
{
    if (m_logStream) {
        delete m_logStream;
        m_logStream = nullptr;
    }
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }
}

void Logger::setLogLevel(LogLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_logLevel = level;
    qDebug() << "Logger: Log level set to" << levelToString(level);
}

void Logger::enableFileLogging(bool enable)
{
    QMutexLocker locker(&m_mutex);
    
    if (enable == m_fileLoggingEnabled) {
        return;
    }
    
    m_fileLoggingEnabled = enable;
    
    if (enable) {
        // Open log file
        QString logFileName = getLogFileName();
        m_logFile = new QFile(logFileName);
        
        if (m_logFile->open(QIODevice::Append | QIODevice::Text)) {
            m_logStream = new QTextStream(m_logFile);
            qDebug() << "Logger: File logging enabled, writing to" << logFileName;
        } else {
            qWarning() << "Logger: Failed to open log file:" << logFileName;
            delete m_logFile;
            m_logFile = nullptr;
            m_fileLoggingEnabled = false;
        }
    } else {
        // Close log file
        if (m_logStream) {
            delete m_logStream;
            m_logStream = nullptr;
        }
        if (m_logFile) {
            m_logFile->close();
            delete m_logFile;
            m_logFile = nullptr;
        }
        qDebug() << "Logger: File logging disabled";
    }
}

void Logger::setLogDir(const QString &dir)
{
    QMutexLocker locker(&m_mutex);
    
    // Close current log file if open
    bool wasEnabled = m_fileLoggingEnabled;
    if (wasEnabled) {
        enableFileLogging(false);
    }
    
    m_logDir = dir;
    
    // Ensure directory exists
    QDir qdir;
    if (!qdir.exists(m_logDir)) {
        qdir.mkpath(m_logDir);
    }
    
    // Reopen if it was enabled
    if (wasEnabled) {
        enableFileLogging(true);
    }
}

void Logger::log(LogLevel level, const QString &message, const QString &category)
{
    QMutexLocker locker(&m_mutex);
    
    // Filter by log level
    if (level > m_logLevel) {
        return;
    }
    
    // Format timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    
    // Format level
    QString levelStr = levelToString(level);
    
    // Build formatted message
    QString formattedMessage;
    if (category.isEmpty()) {
        formattedMessage = QString("[%1] [%2] %3")
            .arg(timestamp)
            .arg(levelStr)
            .arg(message);
    } else {
        formattedMessage = QString("[%1] [%2] [%3] %4")
            .arg(timestamp)
            .arg(levelStr)
            .arg(category)
            .arg(message);
    }
    
    // Write to console
    switch (level) {
        case LogLevel::Error:
            qCritical().noquote() << formattedMessage;
            break;
        case LogLevel::Warning:
            qWarning().noquote() << formattedMessage;
            break;
        case LogLevel::Info:
            qInfo().noquote() << formattedMessage;
            break;
        case LogLevel::Debug:
            qDebug().noquote() << formattedMessage;
            break;
    }
    
    // Write to file if enabled
    if (m_fileLoggingEnabled) {
        writeToFile(formattedMessage);
    }
    
    // Emit signal for UI consumption
    emit logMessageEmitted(level, formattedMessage);
}

void Logger::error(const QString &message, const QString &category)
{
    log(LogLevel::Error, message, category);
}

void Logger::warning(const QString &message, const QString &category)
{
    log(LogLevel::Warning, message, category);
}

void Logger::info(const QString &message, const QString &category)
{
    log(LogLevel::Info, message, category);
}

void Logger::debug(const QString &message, const QString &category)
{
    log(LogLevel::Debug, message, category);
}

LogLevel Logger::levelFromString(const QString &str)
{
    QString lower = str.toLower();
    if (lower == "error") return LogLevel::Error;
    if (lower == "warning" || lower == "warn") return LogLevel::Warning;
    if (lower == "info") return LogLevel::Info;
    if (lower == "debug") return LogLevel::Debug;
    return LogLevel::Info; // Default
}

QString Logger::levelToString(LogLevel level)
{
    switch (level) {
        case LogLevel::Error: return "ERROR";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Info: return "INFO";
        case LogLevel::Debug: return "DEBUG";
        default: return "UNKNOWN";
    }
}

void Logger::writeToFile(const QString &formattedMessage)
{
    if (!m_logStream || !m_logFile) {
        return;
    }
    
    // Check if rotation is needed
    rotateLogIfNeeded();
    
    // Write message
    *m_logStream << formattedMessage << "\n";
    m_logStream->flush();
}

void Logger::rotateLogIfNeeded()
{
    if (!m_logFile || !m_logFile->isOpen()) {
        return;
    }
    
    // Check file size
    if (m_logFile->size() >= m_maxLogSize) {
        qDebug() << "Logger: Rotating log file (size:" << m_logFile->size() << "bytes)";
        
        // Close current log
        delete m_logStream;
        m_logStream = nullptr;
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
        
        // Rotate log files
        QString baseName = m_logDir + "/app";
        
        // Delete oldest log if we have too many
        QString oldestLog = baseName + QString(".%1.log").arg(m_maxLogFiles - 1);
        QFile::remove(oldestLog);
        
        // Rename existing logs
        for (int i = m_maxLogFiles - 2; i >= 0; i--) {
            QString oldName = (i == 0) ? baseName + ".log" : baseName + QString(".%1.log").arg(i);
            QString newName = baseName + QString(".%1.log").arg(i + 1);
            
            if (QFile::exists(oldName)) {
                QFile::remove(newName); // Remove destination if it exists
                QFile::rename(oldName, newName);
            }
        }
        
        // Open new log file
        m_logFile = new QFile(baseName + ".log");
        if (m_logFile->open(QIODevice::Append | QIODevice::Text)) {
            m_logStream = new QTextStream(m_logFile);
        }
        
        cleanupOldLogs();
    }
}

void Logger::cleanupOldLogs()
{
    QDir dir(m_logDir);
    QFileInfoList files = dir.entryInfoList(QStringList() << "app.*.log", QDir::Files, QDir::Time);
    
    // Keep only the newest m_maxLogFiles
    while (files.size() > m_maxLogFiles) {
        QFile::remove(files.last().absoluteFilePath());
        files.removeLast();
    }
}

QString Logger::getLogFileName() const
{
    return m_logDir + "/app.log";
}
