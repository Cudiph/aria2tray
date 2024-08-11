#include "wsserver.h"

#include <QApplication>
#include <QWebSocket>

using namespace Qt::Literals::StringLiterals;

namespace Aria2Tray {

WSServer::WSServer(int port, QObject *parent)
    : QObject(parent),
      m_server(new QWebSocketServer(u"Aria2Tray IPC"_s, QWebSocketServer::NonSecureMode, this))
{
    if (!m_server->listen(QHostAddress::LocalHost, port))
        return;

    connect(m_server, &QWebSocketServer::newConnection, this, &WSServer::onNewConnection);
}

WSServer::~WSServer()
{
    m_server->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

bool WSServer::isListening() { return m_server->isListening(); }

void WSServer::onNewConnection()
{
    QWebSocket *newSocket = m_server->nextPendingConnection();
    qDebug() << "New connection:" << newSocket;

    connect(newSocket, &QWebSocket::textMessageReceived, this, &WSServer::onTextMessage);
    connect(newSocket, &QWebSocket::disconnected, this, &WSServer::onDisconnected);

    m_clients << newSocket;
}

void WSServer::onDisconnected()
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    qDebug() << "Socket disconnected:" << client;

    if (client) {
        m_clients.removeAll(client);
        client->deleteLater();
    }
}

void WSServer::onTextMessage(QString msg)
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());

    if (msg == "DOPPELGANGER") {
        win->show();
    }

    if (client) {
        client->sendTextMessage(msg);
    }
}

} // namespace Aria2Tray
