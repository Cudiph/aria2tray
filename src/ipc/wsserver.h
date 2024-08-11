#ifndef ARIA2TRAY_WSSERVER_H_
#define ARIA2TRAY_WSSERVER_H_

#include <QWebSocketServer>

#include "win.h"

namespace Aria2Tray {

class WSServer : public QObject {
    Q_OBJECT

public:
    WSServer(int port, QObject *parent = nullptr);
    virtual ~WSServer();

    bool isListening();
    Window *win;

private Q_SLOTS:
    void onNewConnection();
    void onDisconnected();
    void onTextMessage(QString msg);

private:
    QWebSocketServer *m_server;
    QList<QWebSocket *> m_clients;
};

} // namespace Aria2Tray

#endif // !ARIA2TRAY_WSSERVER_H_
