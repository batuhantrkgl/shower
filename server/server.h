#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>

class HttpServer : public QObject {
    Q_OBJECT

public:
    HttpServer(QObject *parent = nullptr);
    ~HttpServer();
    bool listen(quint16 port = 8080);

private slots:
    void handleNewConnection();
    void handleRequest(QTcpSocket *socket);

private:
    void handleGetRequest(QTcpSocket *socket, const QString &path);
    void handlePostRequest(QTcpSocket *socket, const QString &path, const QByteArray &request);
    void handleGetSchedule(QTcpSocket *socket);
    void handleGetPlaylist(QTcpSocket *socket);
    void handleGetMediaFile(QTcpSocket *socket, const QString &path);
    void handlePostSchedule(QTcpSocket *socket, const QByteArray &body);
    void handlePostPlaylist(QTcpSocket *socket, const QByteArray &body);
    void sendResponse(QTcpSocket *socket, const QString &status, const QString &contentType, const QByteArray &body);
    void sendResponse(QTcpSocket *socket, const QString &status, const QString &contentType, const QString &body);
    void sendResponse(QTcpSocket *socket, const QString &status, const QString &contentType, const char *body);
    QString readFile(const QString &filePath);
    void writeFile(const QString &filePath, const QByteArray &data);
    QString getContentType(const QString &fileName);
    QString getDefaultSchedule();
    void ensureDefaultSchedule();
    void generatePlaylist();

    QTcpServer *server;
    quint16 port;
    QString dataDir;
    QString mediaDir;
};

#endif // HTTPSERVER_H

