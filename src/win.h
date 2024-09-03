#ifndef ARIA2TRAY_WINDOW_H_
#define ARIA2TRAY_WINDOW_H_

#include <QCloseEvent>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QSystemTrayIcon>
#include <QTabWidget>

#include "logs.h"
#include "options.h"

namespace Aria2Tray {

class WSServer;

class Window;

class MenuBar : public QMenuBar {
    Q_OBJECT

public:
    MenuBar(QWidget *parent = nullptr);

private Q_SLOTS:
    void on_aboutAction_triggered();

private:
    void helpMenu();
    void fileMenu();

    Window *attached_window = nullptr;
};

class Window : public QMainWindow {
    Q_OBJECT

public:
    Window();
    virtual ~Window();

    void connectWSServer(WSServer *ws_server);

public Q_SLOTS:
    void handleOpen();
    void handleAbout();
    void handleQuit();
    void handleRestart();

private Q_SLOTS:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void onStateChange(QProcess::ProcessState state);

protected Q_SLOTS:
    void closeEvent(QCloseEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    QSystemTrayIcon *createTray();
    QMenu *createTrayMenu();
    QTabWidget *createTabBar();
    MenuBar *createMenuBar();
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

    QMenuBar *m_menu_bar;

    WSServer *m_ws_server = nullptr;
};

} // namespace Aria2Tray

#endif // ARIA2TRAY_WINDOW_H_
