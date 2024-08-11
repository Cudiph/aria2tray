#ifndef ARIA2TRAY_WSCLIENT_H_
#define ARIA2TRAY_WSCLIENT_H_

#include <QUrl>
#include <QWebSocket>

namespace Aria2Tray {

class WSClient : public QObject {
    Q_OBJECT

public:
    WSClient(const QUrl &url, QObject *parent = nullptr);
    virtual ~WSClient();

private Q_SLOTS:
    void onConnected();
    void onMsgReceived(QString msg);

private:
    QWebSocket m_ws;
};

} // namespace Aria2Tray

#endif // !ARIA2TRAY_WSCLIENT_H_
