#include <QApplication>

#include "wsclient.h"

using namespace Qt::Literals::StringLiterals;

namespace Aria2Tray {

WSClient::WSClient(const QUrl &url, QObject *parent) : QObject(parent)
{
    connect(&m_ws, &QWebSocket::connected, this, &WSClient::onConnected);
    m_ws.open(url);
}

WSClient::~WSClient() {}

void WSClient::onConnected()
{
    connect(&m_ws, &QWebSocket::textMessageReceived, this, &WSClient::onMsgReceived);
    m_ws.sendTextMessage(u"DOPPELGANGER"_s);
    QApplication::exit();
}

void WSClient::onMsgReceived(QString msg)
{
    qDebug() << msg;
    // TODO: xxx
}

} // namespace Aria2Tray
