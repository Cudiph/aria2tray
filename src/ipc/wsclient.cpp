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
    m_ws.sendTextMessage(u"DOPPELGANGER"_s);
    QApplication::exit();
}

} // namespace Aria2Tray
