#ifndef ARIA2TRAY_WSSERVER_H_
#define ARIA2TRAY_WSSERVER_H_

#include <QJsonArray>
#include <QJsonObject>
#include <QWebSocketServer>

namespace Aria2Tray {

class Window;

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

private Q_SLOTS:
    void onNewConnection();
    void onDisconnected();
    void onTextMessage(QString msg);

private:
    void processRequest(QJsonObject &req_obj, QJsonObject &res);
    bool verifySecret(QJsonObject &request, QJsonObject &res, QJsonArray &new_param);

    // jsonrpc methods
    void methodOpen(const QJsonArray &params, QJsonObject &res);
    void methodDelete(const QJsonArray &params, QJsonObject &res);
    void methodStatus(const QJsonArray &params, QJsonObject &res);

    // authorizationless methods
    void methodPing(QJsonObject &res);

    QWebSocketServer *m_server;
    QList<QWebSocket *> m_clients;
    QString m_secret = QString("");
    Window *m_win    = nullptr;
};

} // namespace Aria2Tray

#endif // !ARIA2TRAY_WSSERVER_H_
