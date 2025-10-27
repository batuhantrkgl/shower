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
#include "server.h"

HttpServer::HttpServer(QObject *parent) : QObject(parent), port(8080) {
        dataDir = "data";
        mediaDir = "media";
        server = new QTcpServer(this);
        connect(server, &QTcpServer::newConnection, this, &HttpServer::handleNewConnection);
        
        // Setup directories
        QDir().mkpath(dataDir);
        QDir().mkpath(mediaDir);
        
        // Create default schedule if needed
        ensureDefaultSchedule();
        
        // Generate playlist from media folder
        generatePlaylist();
    }

bool HttpServer::listen(quint16 p) {
        port = p;
        if (server->listen(QHostAddress::Any, port)) {
            std::cout << "Server listening on port " << port << std::endl;
            std::cout << "Media directory: " << mediaDir.toStdString() << std::endl;
            std::cout << "Data directory: " << dataDir.toStdString() << std::endl;
            return true;
        }
        return false;
    }

void HttpServer::handleNewConnection() {
        QTcpSocket *socket = server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, [this, socket]() {
            handleRequest(socket);
        });
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }

void HttpServer::handleRequest(QTcpSocket *socket) {
        QByteArray request = socket->readAll();
        
        // Parse request line
        int firstSpace = request.indexOf(' ');
        int secondSpace = request.indexOf(' ', firstSpace + 1);
        if (firstSpace == -1 || secondSpace == -1) {
            sendResponse(socket, "400 Bad Request", "text/plain", "Bad Request");
            return;
        }
        
        QString method = request.left(firstSpace);
        QString path = request.mid(firstSpace + 1, secondSpace - firstSpace - 1);
        
        // Parse URL
        QUrl url("http://localhost" + path);
        QString pathStr = url.path();
        
        std::cout << "Request: " << method.toStdString() << " " << pathStr.toStdString() << std::endl;
        
        if (method == "GET") {
            handleGetRequest(socket, pathStr);
        } else if (method == "POST") {
            handlePostRequest(socket, pathStr, request);
        } else {
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

void HttpServer::handleGetSchedule(QTcpSocket *socket) {
        QString filePath = dataDir + "/schedule.json";
        QString json = readFile(filePath);
        
        if (json.isEmpty()) {
            json = getDefaultSchedule();
        }
        
        sendResponse(socket, "200 OK", "application/json", json);
    }

void HttpServer::handleGetPlaylist(QTcpSocket *socket) {
        QString filePath = dataDir + "/playlist.json";
        QString json = readFile(filePath);
        
        if (json.isEmpty()) {
            generatePlaylist();
            json = readFile(filePath);
        }
        
        sendResponse(socket, "200 OK", "application/json", json);
    }

void HttpServer::handleGetMediaFile(QTcpSocket *socket, const QString &path) {
        QString fileName = path.mid(7); // Remove "/media/"
        
        // Security: prevent directory traversal
        if (fileName.contains("..") || fileName.contains("/")) {
            sendResponse(socket, "403 Forbidden", "text/plain", "Forbidden");
            return;
        }
        
        QString filePath = mediaDir + "/" + fileName;
        
        if (!QFile::exists(filePath)) {
            sendResponse(socket, "404 Not Found", "text/plain", "File Not Found");
            return;
        }
        
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray content = file.readAll();
            QString contentType = getContentType(fileName);
            sendResponse(socket, "200 OK", contentType, content);
        } else {
            sendResponse(socket, "500 Internal Server Error", "text/plain", "Could not read file");
        }
    }

void HttpServer::handlePostSchedule(QTcpSocket *socket, const QByteArray &body) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(body, &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QString filePath = dataDir + "/schedule.json";
            writeFile(filePath, body);
            sendResponse(socket, "200 OK", "application/json", "{\"status\":\"success\"}");
        } else {
            sendResponse(socket, "400 Bad Request", "text/plain", "Invalid JSON");
        }
    }

void HttpServer::handlePostPlaylist(QTcpSocket *socket, const QByteArray &body) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(body, &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QString filePath = dataDir + "/playlist.json";
            writeFile(filePath, body);
            sendResponse(socket, "200 OK", "application/json", "{\"status\":\"success\"}");
        } else {
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
            return file.readAll();
        }
        return QString();
    }

void HttpServer::writeFile(const QString &filePath, const QByteArray &data) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(data);
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

void HttpServer::generatePlaylist() {
        QDir dir(mediaDir);
        QStringList filters = {"*.jpg", "*.jpeg", "*.png", "*.gif", "*.webp", "*.mp4", "*.avi", "*.mov", "*.webm"};
        QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
        
        QJsonArray items;
        
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
            }
            
            item["url"] = "/media/" + fileName;
            items.append(item);
        }
        
        // If no files, add default
        if (items.isEmpty()) {
            QJsonObject defaultItem;
            defaultItem["type"] = "image";
            defaultItem["url"] = "/media/default.jpg";
            defaultItem["duration"] = 5000;
            defaultItem["muted"] = false;
            items.append(defaultItem);
        }
        
        QJsonObject playlist;
        playlist["items"] = items;
        
        QJsonDocument doc(playlist);
        writeFile(dataDir + "/playlist.json", doc.toJson(QJsonDocument::Indented));
        
        std::cout << "Generated playlist with " << items.size() << " items" << std::endl;
    }


#include "server.moc"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    quint16 port = 8080;
    if (argc > 1) {
        port = QString(argv[1]).toUShort();
    }
    
    HttpServer httpServer;
    if (!httpServer.listen(port)) {
        std::cerr << "Failed to start server on port " << port << std::endl;
        return 1;
    }
    
    std::cout << "VideoTimeline Server running on http://localhost:" << port << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    
    return app.exec();
}

