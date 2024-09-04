#ifndef ARIA2TRAY_WSSERVER_H_
#define ARIA2TRAY_WSSERVER_H_

#include <QJsonArray>
#include <QJsonObject>
#include <QThread>
#include <QWebSocket>
#include <QWebSocketServer>

namespace Aria2Tray {

class Window;

struct response {
    bool success;
    QJsonObject response_obj;
    QString response_str;
};

struct batch_counter {
    long long use_remaining;
    QJsonArray data;
};

class WSServer : public QObject {
    Q_OBJECT

public:
    WSServer(int port, QObject *parent = nullptr);
    virtual ~WSServer();

    bool isListening();
    void setWindow(Window *main_win);
    void setSecret(const QString &secret);

public Q_SLOTS:
    void onOptionsChange();
    void onResultReady(struct response result, QWebSocket *client, QJsonObject req_obj);

    // watch out when changing function name
    QString filePickerFolderProxy();
    QString filePickerFileProxy(QString filter);

private Q_SLOTS:
    void onNewConnection();
    void onDisconnected();
    void onTextMessage(QString msg);

private:
    void processRequest(QJsonObject &req_obj, QWebSocket *client);
    bool isClientConnected(QWebSocket *client);

    QWebSocketServer *m_server;
    QList<QWebSocket *> m_clients;
    QString m_secret = QString("");
    Window *m_win    = nullptr;
    QHash<quint32, struct batch_counter *> m_batch_hash;
};

class RequestProcessor : public QThread {
    Q_OBJECT

public:
    RequestProcessor(const QString &secret, QJsonObject req_obj, QObject *parent = nullptr);
    void run() override;
    void setClient(QWebSocket *client);

signals:
    void resultReady(struct response result, QWebSocket *client, QJsonObject req_obj);

private:
    struct response verifySecret(QJsonObject &request, QJsonArray &new_param);

    // jsonrpc methods
    struct response methodOpen(const QJsonArray &params);
    struct response methodDelete(const QJsonArray &params);
    struct response methodStatus(const QJsonArray &params);
    struct response methodVersion(const QJsonArray &params);
    struct response methodFilePicker(const QJsonArray &params);

    // authorizationless methods
    struct response methodPing();

    QString m_secret;
    QJsonObject m_req_obj;
    QWebSocket *m_client = nullptr;
    WSServer *m_wsserver = nullptr;
};

} // namespace Aria2Tray

#endif // !ARIA2TRAY_WSSERVER_H_
