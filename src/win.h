#ifndef ARIA2TRAY_WINDOW_H_
#define ARIA2TRAY_WINDOW_H_

#include <QCloseEvent>
#include <QMainWindow>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QTabWidget>

#include "logs.h"
#include "options.h"

namespace Aria2Tray {

class WSServer;

class Window : public QMainWindow {
    Q_OBJECT

public:
    Window();
    virtual ~Window();

    void connectWSServer(WSServer *ws_server);

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void onStateChange(QProcess::ProcessState state);
    void handleQuit();
    void handleRestart();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QSystemTrayIcon *createTray();
    QMenu *createTrayMenu();
    QTabWidget *createTabBar();
    void restoreState();
    void exit(int code = 0);

    QTabWidget *tabs;
    Options *options;
    Logs *logs;

    QAction *stopAction;
    QAction *restartAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;

    WSServer *m_ws_server = nullptr;
};

} // namespace Aria2Tray

#endif // ARIA2TRAY_WINDOW_H_
