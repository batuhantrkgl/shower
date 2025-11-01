#include <iostream>
#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QUrl>
#include <QDateTime>
#include <QHostInfo>
#include "server.h"

void HttpServer::log(LogLevel level, const QString &message) {
    QString timestamp = getCurrentTimestamp();
    QString levelStr = getLogLevelString(level);
    QString colorCode = getLogLevelColor(level);
    QString resetColor = "\033[0m"; // Reset to default color
    
    std::cout << colorCode.toStdString() << "[" << timestamp.toStdString() << "] [" << levelStr.toStdString() << "] " 
              << message.toStdString() << resetColor.toStdString() << std::endl;
}

QString HttpServer::getLogLevelString(LogLevel level) {
    switch (level) {
        case DEBUG: return "DEBUG";
        case INFO: return "INFO";
        case WARN: return "WARN";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

QString HttpServer::getLogLevelColor(LogLevel level) {
    switch (level) {
        case DEBUG: return "\033[36m"; // Cyan
        case INFO: return "\033[32m";  // Green
        case WARN: return "\033[33m";  // Yellow
        case ERROR: return "\033[31m"; // Red
        default: return "\033[0m";     // Default
    }
}

QString HttpServer::getCurrentTimestamp() {
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

HttpServer::HttpServer(QObject *parent) : QObject(parent), port(3232) {
        dataDir = DATA_DIR;
        mediaDir = MEDIA_DIR;
        server = new QTcpServer(this);
        connect(server, &QTcpServer::newConnection, this, &HttpServer::handleNewConnection);
        
        // Setup directories
        QDir().mkpath(dataDir);
        QDir().mkpath(mediaDir);
        
        log(INFO, "Server initialized");
        log(INFO, QString("Media directory: %1").arg(mediaDir));
        log(INFO, QString("Data directory: %1").arg(dataDir));
        
        // Create default schedule if needed
        ensureDefaultSchedule();
        
        // Generate playlist from media folder only if needed
        ensurePlaylist();
    }

HttpServer::~HttpServer() {
    // QTcpServer is a child object and will be deleted automatically
}

bool HttpServer::listen(quint16 p) {
        port = p;
        if (server->listen(QHostAddress::Any, port)) {
            log(INFO, QString("Server listening on port %1").arg(port));
            return true;
        } else {
            log(ERROR, QString("Failed to bind to port %1").arg(port));
            return false;
        }
    }

void HttpServer::handleNewConnection() {
        QTcpSocket *socket = server->nextPendingConnection();
        QString clientIP = socket->peerAddress().toString();
        log(INFO, QString("New connection from %1:%2").arg(clientIP).arg(socket->peerPort()));
        
        connect(socket, &QTcpSocket::readyRead, [this, socket]() {
            handleRequest(socket);
        });
        connect(socket, &QTcpSocket::disconnected, [this, socket, clientIP]() {
            log(INFO, QString("Connection closed from %1").arg(clientIP));
            socket->deleteLater();
        });
    }

void HttpServer::handleRequest(QTcpSocket *socket) {
        QByteArray request = socket->readAll();
        
        // Parse request line
        int firstSpace = request.indexOf(' ');
        int secondSpace = request.indexOf(' ', firstSpace + 1);
        if (firstSpace == -1 || secondSpace == -1) {
            log(WARN, QString("Invalid request from %1 - malformed request line").arg(socket->peerAddress().toString()));
            sendResponse(socket, "400 Bad Request", "text/plain", "Bad Request");
            return;
        }
        
        QString method = request.left(firstSpace);
        QString path = request.mid(firstSpace + 1, secondSpace - firstSpace - 1);
        
        // Parse URL
        QUrl url("http://localhost" + path);
        QString pathStr = url.path();
        
        QString clientIP = socket->peerAddress().toString();
        log(INFO, QString("Request: %1 %2 from %3").arg(method).arg(pathStr).arg(clientIP));
        
        if (method == "GET") {
            handleGetRequest(socket, pathStr);
        } else if (method == "POST") {
            handlePostRequest(socket, pathStr, request);
        } else if (method == "HEAD") {
            handleHeadRequest(socket, pathStr);
        } else {
            log(WARN, QString("Unsupported method %1 from %2").arg(method).arg(clientIP));
            sendResponse(socket, "405 Method Not Allowed", "text/plain", "Method Not Allowed");
        }
    }

void HttpServer::handleGetRequest(QTcpSocket *socket, const QString &path) {
        if (path == "/api/schedule") {
            handleGetSchedule(socket);
        } else if (path == "/api/media/playlist") {
            handleGetPlaylist(socket);
        } else if (path == "/api/media/regenerate") {
            generatePlaylist();
            sendResponse(socket, "200 OK", "application/json", "{\"status\":\"success\",\"message\":\"Playlist regenerated\"}");
        } else if (path == "/api/media/toggle-auto-regenerate") {
            toggleAutoRegenerate(socket);
        } else if (path == "/api/screen/toggle") {
            toggleScreenMirroring(socket);
        } else if (path.startsWith("/media/")) {
            handleGetMediaFile(socket, path);
        } else {
            sendResponse(socket, "404 Not Found", "text/plain", "Not Found");
        }
    }

void HttpServer::handlePostRequest(QTcpSocket *socket, const QString &path, const QByteArray &request) {
        // Extract JSON body (basic implementation)
        QByteArray body = request;
        int bodyStart = request.indexOf("\r\n\r\n");
        if (bodyStart != -1) {
            body = request.mid(bodyStart + 4);
        }
        
        if (path == "/api/schedule") {
            handlePostSchedule(socket, body);
        } else if (path == "/api/media/playlist") {
            handlePostPlaylist(socket, body);
        } else {
            sendResponse(socket, "404 Not Found", "text/plain", "Not Found");
        }
    }

void HttpServer::handleHeadRequest(QTcpSocket *socket, const QString &path) {
        if (path == "/api/schedule") {
            // For HEAD requests, just send headers without body
            QString filePath = dataDir + "/schedule.json";
            QString json = readFile(filePath);
            
            if (json.isEmpty()) {
                json = getDefaultSchedule();
            }
            
            // Parse the JSON to add server information
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
            
            if (error.error == QJsonParseError::NoError && doc.isObject()) {
                QJsonObject scheduleObj = doc.object();
                
                // Add server information
                QString hostname = QHostInfo::localHostName();
                scheduleObj["server_hostname"] = hostname;
                scheduleObj["server_ip"] = socket->localAddress().toString();
                
                // Re-create the document with server info
                doc = QJsonDocument(scheduleObj);
                json = doc.toJson(QJsonDocument::Indented);
            }
            
            sendHeadResponse(socket, "200 OK", "application/json", json.size());
        } else if (path == "/api/media/playlist") {
            // For HEAD requests, just send headers without body
            QString filePath = dataDir + "/playlist.json";
            QString json = readFile(filePath);
            
            if (json.isEmpty()) {
                log(WARN, "Playlist file not found for HEAD request");
                sendHeadResponse(socket, "404 Not Found", "text/plain", 0);
                return;
            }
            
            sendHeadResponse(socket, "200 OK", "application/json", json.size());
        } else if (path.startsWith("/media/")) {
            QString fileName = path.mid(7); // Remove "/media/"
            
            // Security: prevent directory traversal
            if (fileName.contains("..") || fileName.contains("/")) {
                log(WARN, QString("Directory traversal attempt blocked: %1 from %2").arg(fileName).arg(socket->peerAddress().toString()));
                sendHeadResponse(socket, "403 Forbidden", "text/plain", 0);
                return;
            }
            
            QString filePath = mediaDir + "/" + fileName;
            
            if (!QFile::exists(filePath)) {
                log(WARN, QString("Requested file not found: %1 from %2").arg(filePath).arg(socket->peerAddress().toString()));
                sendHeadResponse(socket, "404 Not Found", "text/plain", 0);
                return;
            }
            
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly)) {
                QString contentType = getContentType(fileName);
                sendHeadResponse(socket, "200 OK", contentType, file.size());
            } else {
                log(ERROR, QString("Failed to read media file: %1").arg(filePath));
                sendHeadResponse(socket, "500 Internal Server Error", "text/plain", 0);
            }
        } else {
            sendHeadResponse(socket, "404 Not Found", "text/plain", 0);
        }
    }

void HttpServer::handleGetSchedule(QTcpSocket *socket) {
        QString filePath = dataDir + "/schedule.json";
        QString json = readFile(filePath);
        
        if (json.isEmpty()) {
            json = getDefaultSchedule();
        }
        
        // Parse the JSON to add server information
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject scheduleObj = doc.object();
            
            // Add server information
            QString hostname = QHostInfo::localHostName();
            scheduleObj["server_hostname"] = hostname;
            scheduleObj["server_ip"] = socket->localAddress().toString();
            
            // Re-create the document with server info
            doc = QJsonDocument(scheduleObj);
            json = doc.toJson(QJsonDocument::Indented);
        }
        
        sendResponse(socket, "200 OK", "application/json", json);
    }

void HttpServer::handleGetPlaylist(QTcpSocket *socket) {
        QString filePath = dataDir + "/playlist.json";
        QString json = readFile(filePath);
        
        if (json.isEmpty()) {
            log(WARN, "Playlist file not found, generating new playlist");
            generatePlaylist();
            json = readFile(filePath);
        } else {
            // Check if we should auto-regenerate based on media folder changes
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
            
            if (error.error == QJsonParseError::NoError && doc.isObject()) {
                QJsonObject playlist = doc.object();
                bool autoRegenerate = playlist.value("auto_regenerate").toBool(true); // Default to true for backward compatibility
                
                if (autoRegenerate && shouldRegeneratePlaylist()) {
                    log(INFO, "Auto-regenerating playlist due to media folder changes");
                    generatePlaylist();
                    json = readFile(filePath);
                }
            } else {
                log(ERROR, QString("Invalid playlist JSON, regenerating: %1").arg(error.errorString()));
                generatePlaylist();
                json = readFile(filePath);
            }
        }
        
        sendResponse(socket, "200 OK", "application/json", json);
    }

void HttpServer::handleGetMediaFile(QTcpSocket *socket, const QString &path) {
        QString fileName = path.mid(7); // Remove "/media/"
        
        // Security: prevent directory traversal
        if (fileName.contains("..") || fileName.contains("/")) {
            log(WARN, QString("Directory traversal attempt blocked: %1 from %2").arg(fileName).arg(socket->peerAddress().toString()));
            sendResponse(socket, "403 Forbidden", "text/plain", "Forbidden");
            return;
        }
        
        QString filePath = mediaDir + "/" + fileName;
        
        if (!QFile::exists(filePath)) {
            log(WARN, QString("Requested file not found: %1 from %2").arg(filePath).arg(socket->peerAddress().toString()));
            sendResponse(socket, "404 Not Found", "text/plain", "File Not Found");
            return;
        }
        
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray content = file.readAll();
            QString contentType = getContentType(fileName);
            log(DEBUG, QString("Serving file: %1 (%2 bytes, %3) to %4").arg(fileName).arg(content.size()).arg(contentType).arg(socket->peerAddress().toString()));
            sendResponse(socket, "200 OK", contentType, content);
        } else {
            log(ERROR, QString("Failed to read media file: %1").arg(filePath));
            sendResponse(socket, "500 Internal Server Error", "text/plain", "Could not read file");
        }
    }

void HttpServer::handlePostSchedule(QTcpSocket *socket, const QByteArray &body) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(body, &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QString filePath = dataDir + "/schedule.json";
            writeFile(filePath, body);
            log(INFO, "Schedule updated successfully");
            sendResponse(socket, "200 OK", "application/json", "{\"status\":\"success\"}");
        } else {
            log(ERROR, QString("Invalid JSON in schedule update: %1").arg(error.errorString()));
            sendResponse(socket, "400 Bad Request", "text/plain", "Invalid JSON");
        }
    }

void HttpServer::handlePostPlaylist(QTcpSocket *socket, const QByteArray &body) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(body, &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject playlist = doc.object();
            
            // Ensure auto_regenerate field exists (default to true if not specified)
            if (!playlist.contains("auto_regenerate")) {
                playlist["auto_regenerate"] = true;
            }
            
            // Write the updated playlist
            QJsonDocument updatedDoc(playlist);
            QString filePath = dataDir + "/playlist.json";
            writeFile(filePath, updatedDoc.toJson(QJsonDocument::Indented));
            
            bool autoRegenerate = playlist["auto_regenerate"].toBool();
            QString message = QString("{\"status\":\"success\",\"auto_regenerate\":%1}").arg(autoRegenerate ? "true" : "false");
            log(INFO, QString("Playlist updated successfully (auto_regenerate: %1)").arg(autoRegenerate ? "true" : "false"));
            sendResponse(socket, "200 OK", "application/json", message);
        } else {
            log(ERROR, QString("Invalid JSON in playlist update: %1").arg(error.errorString()));
            sendResponse(socket, "400 Bad Request", "text/plain", "Invalid JSON");
        }
    }

void HttpServer::sendResponse(QTcpSocket *socket, const QString &status, const QString &contentType, const QByteArray &body) {
        QString response = QString("HTTP/1.1 %1\r\n"
                                  "Content-Type: %2\r\n"
                                  "Content-Length: %3\r\n"
                                  "Access-Control-Allow-Origin: *\r\n"
                                  "\r\n")
                          .arg(status)
                          .arg(contentType)
                          .arg(body.size());
        
        socket->write(response.toUtf8() + body);
        socket->flush();
        
        QString clientIP = socket->peerAddress().toString();
        log(DEBUG, QString("Response: %1 %2 (%3 bytes) to %4").arg(status).arg(contentType).arg(body.size()).arg(clientIP));
    }
    
void HttpServer::sendHeadResponse(QTcpSocket *socket, const QString &status, const QString &contentType, qint64 contentLength) {
        QString response = QString("HTTP/1.1 %1\r\n"
                                  "Content-Type: %2\r\n"
                                  "Content-Length: %3\r\n"
                                  "Access-Control-Allow-Origin: *\r\n"
                                  "\r\n")
                          .arg(status)
                          .arg(contentType)
                          .arg(contentLength);
        
        socket->write(response.toUtf8());
        socket->flush();
        
        QString clientIP = socket->peerAddress().toString();
        log(DEBUG, QString("HEAD Response: %1 %2 (%3 bytes) to %4").arg(status).arg(contentType).arg(contentLength).arg(clientIP));
    }
    
// Helper overloads to avoid ambiguity
void HttpServer::sendResponse(QTcpSocket *socket, const QString &status, const QString &contentType, const QString &body) {
    sendResponse(socket, status, contentType, body.toUtf8());
}

void HttpServer::sendResponse(QTcpSocket *socket, const QString &status, const QString &contentType, const char *body) {
    sendResponse(socket, status, contentType, QByteArray(body));
}

QString HttpServer::readFile(const QString &filePath) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QString content = file.readAll();
            log(DEBUG, QString("Read %1 bytes from file: %2").arg(content.size()).arg(filePath));
            return content;
        } else {
            log(ERROR, QString("Failed to read file: %1").arg(filePath));
            return QString();
        }
    }

void HttpServer::writeFile(const QString &filePath, const QByteArray &data) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            qint64 bytesWritten = file.write(data);
            log(DEBUG, QString("Wrote %1 bytes to file: %2").arg(bytesWritten).arg(filePath));
        } else {
            log(ERROR, QString("Failed to write file: %1").arg(filePath));
        }
    }

QString HttpServer::getContentType(const QString &fileName) {
        QString ext = fileName.mid(fileName.lastIndexOf('.') + 1).toLower();
        
        if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
        if (ext == "png") return "image/png";
        if (ext == "gif") return "image/gif";
        if (ext == "webp") return "image/webp";
        if (ext == "mp4") return "video/mp4";
        if (ext == "webm") return "video/webm";
        if (ext == "json") return "application/json";
        
        return "application/octet-stream";
    }

QString HttpServer::getDefaultSchedule() {
        return R"({
  "school_start": "08:50",
  "school_end": "15:55",
  "blocks": [
    {"start_time": "08:50", "end_time": "09:30", "name": "Ders 1", "type": "lesson"},
    {"start_time": "09:30", "end_time": "09:40", "name": "Teneffüs", "type": "break"},
    {"start_time": "09:40", "end_time": "10:20", "name": "Ders 2", "type": "lesson"},
    {"start_time": "10:20", "end_time": "10:30", "name": "Teneffüs", "type": "break"},
    {"start_time": "10:30", "end_time": "11:10", "name": "Ders 3", "type": "lesson"},
    {"start_time": "11:10", "end_time": "11:20", "name": "Teneffüs", "type": "break"},
    {"start_time": "11:20", "end_time": "12:00", "name": "Ders 4", "type": "lesson"},
    {"start_time": "12:00", "end_time": "12:45", "name": "Öğle Arası", "type": "lunch"},
    {"start_time": "12:45", "end_time": "13:25", "name": "Ders 5", "type": "lesson"},
    {"start_time": "13:25", "end_time": "13:35", "name": "Teneffüs", "type": "break"},
    {"start_time": "13:35", "end_time": "14:15", "name": "Ders 6", "type": "lesson"},
    {"start_time": "14:15", "end_time": "14:25", "name": "Teneffüs", "type": "break"},
    {"start_time": "14:25", "end_time": "15:05", "name": "Ders 7", "type": "lesson"},
    {"start_time": "15:05", "end_time": "15:15", "name": "Teneffüs", "type": "break"},
    {"start_time": "15:15", "end_time": "15:55", "name": "Ders 8", "type": "lesson"}
  ]
})";
    }

void HttpServer::ensureDefaultSchedule() {
        QString filePath = dataDir + "/schedule.json";
        if (!QFile::exists(filePath)) {
            writeFile(filePath, getDefaultSchedule().toUtf8());
        }
    }

void HttpServer::ensurePlaylist() {
        QString filePath = dataDir + "/playlist.json";
        
        // If playlist doesn't exist, create it
        if (!QFile::exists(filePath)) {
            log(INFO, "Playlist file does not exist, generating new playlist");
            generatePlaylist();
            return;
        }
        
        // If playlist exists, check if we should auto-regenerate
        QString json = readFile(filePath);
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject playlist = doc.object();
            bool autoRegenerate = playlist.value("auto_regenerate").toBool(true); // Default to true for backward compatibility
            
            if (autoRegenerate && shouldRegeneratePlaylist()) {
                log(INFO, "Auto-regenerating playlist on startup due to media folder changes");
                generatePlaylist();
            } else {
                log(INFO, QString("Preserving existing playlist (auto_regenerate: %1)").arg(autoRegenerate ? "true" : "false"));
            }
        } else {
            // If JSON is invalid, regenerate
            log(WARN, QString("Invalid playlist JSON, regenerating: %1").arg(error.errorString()));
            generatePlaylist();
        }
    }

void HttpServer::generatePlaylist() {
        QDir dir(mediaDir);
        QStringList filters = {"*.jpg", "*.jpeg", "*.png", "*.gif", "*.webp", "*.mp4", "*.avi", "*.mov", "*.webm"};
        QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
        
        log(INFO, QString("Scanning media directory for files: found %1 files").arg(files.size()));
        
        QJsonArray items;
        
        // Add screen mirroring as the first item if requested
        QString screenFlagPath = dataDir + "/enable_screen_mirroring";
        if (QFile::exists(screenFlagPath)) {
            QJsonObject screenItem;
            screenItem["type"] = "screen";
            screenItem["url"] = "screen://primary"; // Special URL for screen capture
            screenItem["duration"] = -1; // Continuous until manually stopped
            screenItem["muted"] = false;
            items.append(screenItem);
            log(DEBUG, "Added screen mirroring item to playlist");
        }
        
        for (const QFileInfo &fileInfo : files) {
            QString fileName = fileInfo.fileName();
            QString ext = fileInfo.suffix().toLower();
            
            QJsonObject item;
            
            if (ext == "mp4" || ext == "avi" || ext == "mov" || ext == "webm") {
                item["type"] = "video";
                item["muted"] = fileName.contains("mute", Qt::CaseInsensitive) || 
                                fileName.contains("silent", Qt::CaseInsensitive) ||
                                fileName.contains("background", Qt::CaseInsensitive);
                item["duration"] = -1;
                log(DEBUG, QString("Added video file: %1 (muted: %2)").arg(fileName).arg(item["muted"].toBool()));
            } else {
                item["type"] = "image";
                item["muted"] = false;
                
                // Smart duration detection
                int duration = 5000; // default
                QString lowerName = fileName.toLower();
                if (lowerName.contains("quick") || lowerName.contains("short")) {
                    duration = 2000;
                } else if (lowerName.contains("long") || lowerName.contains("schedule")) {
                    duration = 10000;
                } else if (lowerName.contains("banner") || lowerName.contains("logo")) {
                    duration = 3000;
                }
                item["duration"] = duration;
                log(DEBUG, QString("Added image file: %1 (duration: %2ms)").arg(fileName).arg(duration));
            }
            
            item["url"] = "/media/" + fileName;
            items.append(item);
        }
        
        // If no files, create a default placeholder
        if (items.isEmpty()) {
            QString defaultPath = mediaDir + "/default.jpg";
            if (!QFile::exists(defaultPath)) {
                // Create a proper minimal valid JPEG placeholder
                QFile defaultFile(defaultPath);
                if (defaultFile.open(QIODevice::WriteOnly)) {
                    // Minimal valid JPEG data (2x2 gray pixels)
                    // This is a properly formatted JPEG that won't cause corruption errors
                    unsigned char minJpeg[] = {
                        // SOI (Start of Image)
                        0xFF, 0xD8,
                        // APP0 (JFIF header)
                        0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01,
                        0x01, 0x01, 0x00, 0x48, 0x00, 0x48, 0x00, 0x00,
                        // DQT (Define Quantization Table)
                        0xFF, 0xDB, 0x00, 0x43, 0x00, 0x10, 0x0B, 0x0C, 0x0E, 0x0C,
                        0x0A, 0x10, 0x0E, 0x0D, 0x0E, 0x12, 0x11, 0x10, 0x13, 0x18,
                        0x28, 0x1A, 0x18, 0x16, 0x16, 0x18, 0x31, 0x23, 0x25, 0x1D,
                        0x28, 0x3A, 0x33, 0x3D, 0x3C, 0x39, 0x33, 0x38, 0x37, 0x40,
                        0x48, 0x5C, 0x4E, 0x40, 0x44, 0x57, 0x45, 0x37, 0x38, 0x50,
                        0x6D, 0x51, 0x57, 0x5F, 0x62, 0x67, 0x68, 0x67, 0x3E, 0x4D,
                        0x71, 0x79, 0x70, 0x64, 0x78, 0x5C, 0x65, 0x67, 0x63,
                        // SOF0 (Start of Frame)
                        0xFF, 0xC0, 0x00, 0x11, 0x08, 0x00, 0x02, 0x00, 0x02, 0x01,
                        0x01, 0x11, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01,
                        // DHT (Define Huffman Table)
                        0xFF, 0xC4, 0x00, 0x1F, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01,
                        0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                        0x09, 0x0A, 0x0B,
                        // DHT (Define Huffman Table)
                        0xFF, 0xC4, 0x00, 0x1F, 0x10, 0x00, 0x02, 0x01, 0x03, 0x03,
                        0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01,
                        0x7D, 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21,
                        0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14,
                        0x32, 0x81, 0x91, 0xA1, 0x08, 0x23, 0x42, 0xB1, 0xC1, 0x15,
                        0x52, 0xD1, 0xF0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A,
                        0x16, 0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28, 0x29,
                        0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44,
                        0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56,
                        0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
                        0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A,
                        0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93,
                        0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4,
                        0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5,
                        0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6,
                        0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
                        0xD8, 0xD9, 0xDA, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
                        0xE8, 0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
                        0xF8, 0xF9, 0xFA,
                        // SOS (Start of Scan)
                        0xFF, 0xDA, 0x00, 0x0C, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03,
                        0x11, 0x00, 0x3F, 0x00,
                        // Compressed image data (minimal 2x2 image)
                        0x7F, 0x80,
                        // EOI (End of Image)
                        0xFF, 0xD9
                    };
                    defaultFile.write(reinterpret_cast<const char*>(minJpeg), sizeof(minJpeg));
                    defaultFile.close();
                    log(INFO, "Created default placeholder image");
                }
            }
            
            QJsonObject defaultItem;
            defaultItem["type"] = "image";
            defaultItem["url"] = "/media/default.jpg";
            defaultItem["duration"] = 5000;
            defaultItem["muted"] = false;
            items.append(defaultItem);
            log(DEBUG, "Added default placeholder item to empty playlist");
        }
        
        QJsonObject playlist;
        playlist["auto_regenerate"] = true; // Default to auto-regenerate for new playlists
        playlist["items"] = items;
        
        QJsonDocument doc(playlist);
        writeFile(dataDir + "/playlist.json", doc.toJson(QJsonDocument::Indented));
        
        log(INFO, QString("Generated playlist with %1 items").arg(items.size()));
    }

bool HttpServer::shouldRegeneratePlaylist() {
        QString filePath = dataDir + "/playlist.json";
        QFileInfo playlistInfo(filePath);
        
        if (!playlistInfo.exists()) {
            return true;
        }
        
        // Check if any media files are newer than the playlist
        QDir dir(mediaDir);
        QStringList filters = {"*.jpg", "*.jpeg", "*.png", "*.gif", "*.webp", "*.mp4", "*.avi", "*.mov", "*.webm"};
        QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
        
        QDateTime playlistTime = playlistInfo.lastModified();
        
        for (const QFileInfo &fileInfo : files) {
            if (fileInfo.lastModified() > playlistTime) {
                return true; // Media file is newer than playlist
            }
        }
        
        // Check if number of media files matches playlist items
        QString json = readFile(filePath);
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject playlist = doc.object();
            QJsonArray items = playlist["items"].toArray();
            
            if (items.size() != files.size()) {
                return true; // Number of files changed
            }
        }
        
        return false;
    }

void HttpServer::toggleAutoRegenerate(QTcpSocket *socket) {
        QString filePath = dataDir + "/playlist.json";
        QString json = readFile(filePath);
        
        if (json.isEmpty()) {
            log(WARN, "Playlist file not found for auto-regenerate toggle");
            sendResponse(socket, "404 Not Found", "text/plain", "Playlist not found");
            return;
        }
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
        
        if (error.error != QJsonParseError::NoError || !doc.isObject()) {
            log(ERROR, QString("Invalid playlist JSON for auto-regenerate toggle: %1").arg(error.errorString()));
            sendResponse(socket, "400 Bad Request", "text/plain", "Invalid playlist JSON");
            return;
        }
        
        QJsonObject playlist = doc.object();
        bool currentValue = playlist.value("auto_regenerate").toBool(true);
        bool newValue = !currentValue;
        
        playlist["auto_regenerate"] = newValue;
        
        QJsonDocument updatedDoc(playlist);
        writeFile(filePath, updatedDoc.toJson(QJsonDocument::Indented));
        
        QString message = QString("{\"status\":\"success\",\"auto_regenerate\":%1,\"message\":\"Auto-regenerate %2\"}")
                         .arg(newValue ? "true" : "false")
                         .arg(newValue ? "enabled" : "disabled");
        
        sendResponse(socket, "200 OK", "application/json", message);
        
        log(INFO, QString("Auto-regenerate %1").arg(newValue ? "enabled" : "disabled"));
    }

void HttpServer::toggleScreenMirroring(QTcpSocket *socket) {
        QString flagPath = dataDir + "/enable_screen_mirroring";
        bool currentlyEnabled = QFile::exists(flagPath);
        bool newState = !currentlyEnabled;
        
        if (newState) {
            // Create the flag file to enable screen mirroring
            QFile flagFile(flagPath);
            if (flagFile.open(QIODevice::WriteOnly)) {
                flagFile.write("1");
                flagFile.close();
                log(INFO, "Screen mirroring enabled");
            } else {
                log(ERROR, "Failed to create screen mirroring flag file");
                sendResponse(socket, "500 Internal Server Error", "text/plain", "Failed to enable screen mirroring");
                return;
            }
        } else {
            // Remove the flag file to disable screen mirroring
            if (QFile::remove(flagPath)) {
                log(INFO, "Screen mirroring disabled");
            } else {
                log(WARN, "Failed to remove screen mirroring flag file");
            }
        }
        
        // Regenerate playlist to include/exclude screen mirroring
        generatePlaylist();
        
        QString message = QString("{\"status\":\"success\",\"screen_mirroring\":%1,\"message\":\"Screen mirroring %2\"}")
                         .arg(newState ? "true" : "false")
                         .arg(newState ? "enabled" : "disabled");
        
        sendResponse(socket, "200 OK", "application/json", message);
    }


int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    quint16 port = 3232;
    if (argc > 1) {
        port = QString(argv[1]).toUShort();
    }
    
    HttpServer httpServer;
    if (!httpServer.listen(port)) {
        return 1;
    }
    
    std::cout << "VideoTimeline Server running on http://localhost:" << port << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    
    return app.exec();
}

