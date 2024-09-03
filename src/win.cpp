#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QStringLiteral>
#include <QTabBar>
#include <QVBoxLayout>
#include <cstdlib>

#include "dialogs/about.h"
#include "ipc/wsserver.h"
#include "process.h"
#include "win.h"

using namespace Qt::Literals::StringLiterals;

namespace Aria2Tray {
const int INIT_WIDTH  = 680;
const int INIT_HEIGHT = 540;

Window::Window()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        auto msgBox = QMessageBox();
        msgBox.setText(tr("System tray is not available in this desktop."));
        msgBox.exec();
        exit(EXIT_FAILURE);
    }

    createTray();
    setWindowIcon(QIcon(u":/images/assets/icon.ico"_s));

    auto centralWidget = new QWidget(this);
    auto vLayout       = new QVBoxLayout(centralWidget);
    vLayout->addWidget(createTabBar());
    setCentralWidget(centralWidget);

    // move to center of the screen (work same without)
    auto screen_dim = QApplication::primaryScreen()->size();
    auto xPos       = screen_dim.width() / 2 - INIT_WIDTH / 2;
    auto yPos       = screen_dim.height() / 2 - INIT_HEIGHT / 2;
    move(xPos, yPos);

    m_menu_bar = createMenuBar();
    setMenuBar(m_menu_bar);

    restoreState();
}

Window::~Window() {}

void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        setVisible(!isVisible());
        break;
    default:
        break;
    }
    return;
}

void Window::connectWSServer(WSServer *ws_server)
{
    if (m_ws_server)
        return;
    m_ws_server = ws_server;
    connect(options, &Options::optionsChanged, ws_server, &WSServer::onOptionsChange);
    ws_server->onOptionsChange(); // load initial secret
}

void Window::onStateChange(QProcess::ProcessState state)
{
    switch (state) {
    case QProcess::NotRunning:
        trayIcon->setToolTip(tr("Service stopped."));
        break;
    case QProcess::Starting:
        trayIcon->setToolTip(tr("Service is starting..."));
        break;
    case QProcess::Running:
        trayIcon->setToolTip(tr("Service is running."));
        break;
    }
}

void Window ::handleOpen()
{
    show();
}

void Window ::handleAbout()
{
    auto about_dialog = AboutDialog();
    about_dialog.exec();
}

void Window::handleQuit()
{
    options->stop();
    options->stopWait(5000);
    options->kill();

    exit();
}

void Window::handleRestart()
{
    options->stop();
    options->stopWait(10000);
    options->kill();
    options->start();
}

void Window::closeEvent(QCloseEvent *event)
{
    event->ignore();
    setVisible(false);
}

void Window::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Alt)
        m_menu_bar->setVisible(!m_menu_bar->isVisible());
    else
        QMainWindow::keyReleaseEvent(event);
}

QSystemTrayIcon *Window::createTray()
{
    trayIcon = new QSystemTrayIcon;

#ifdef Q_OS_WIN32
    trayIcon->setIcon(QIcon(u":/images/assets/icon.ico"_s));
#else
    trayIcon->setIcon(QIcon(QStringLiteral(A2T_DATA_DIR) + u"/aria2tray.svg"_s));
#endif // Q_OS_WIN32
    trayIcon->setContextMenu(createTrayMenu());
    trayIcon->setToolTip(tr("Service not running."));
    trayIcon->show();

    connect(trayIcon, &QSystemTrayIcon::activated, this, &Window::iconActivated);
    connect(Process::aria2Instance(), &QProcess::stateChanged, this, &Window::onStateChange);
    return trayIcon;
}

QMenu *Window::createTrayMenu()
{
    trayMenu = new QMenu;
    trayMenu->setMinimumWidth(150);

    auto open_action = new QAction(tr("Open"), this);
    connect(open_action, &QAction::triggered, this, &Window::handleOpen);

    auto about_action = new QAction(tr("About"), this);
    connect(about_action, &QAction::triggered, this, &Window::handleAbout);

    quitAction = new QAction(tr("Quit"));
    connect(quitAction, &QAction::triggered, this, &Window::handleQuit);

    restartAction = new QAction(tr("Restart"), this);
    connect(restartAction, &QAction::triggered, this, &Window::handleRestart);

    trayMenu->addAction(open_action);
    trayMenu->addAction(about_action);
    trayMenu->addSeparator();
    trayMenu->addAction(restartAction);
    trayMenu->addAction(quitAction);
    return trayMenu;
}

QTabWidget *Window::createTabBar()
{
    tabs    = new QTabWidget(this);
    options = new Options(this);
    logs    = new Logs(this);

    options->logsWidget = logs;
    tabs->addTab(options, tr("Options"));
    tabs->addTab(logs, tr("Logs"));

    return tabs;
}

MenuBar *Window::createMenuBar()
{
    auto menu_bar = new MenuBar(this);
    menu_bar->hide();

    return menu_bar;
}

void Window::restoreState()
{
    QSettings settings;
    settings.beginGroup(u"window"_s);
    auto w = settings.value(u"width"_s, INIT_WIDTH).toInt();
    auto h = settings.value(u"height"_s, INIT_HEIGHT).toInt();
    auto x = settings.value(u"x"_s, 0).toInt();
    auto y = settings.value(u"y"_s, 0).toInt();
    settings.endGroup();

#ifdef Q_OS_LINUX
    setFixedSize(w, h);
#else
    resize(w, h);
#endif // Q_OS_LINUX

    if (x || y)
        move(x, y);
}

void Window::exit(int code)
{
    QSettings settings;
    settings.beginGroup(u"window"_s);
    settings.setValue(u"width"_s, width());
    settings.setValue(u"height"_s, height());
    settings.setValue(u"x"_s, x());
    settings.setValue(u"y"_s, y());
    settings.endGroup();

    QApplication::exit(code);
}

MenuBar::MenuBar(QWidget *parent) : QMenuBar(parent)
{
    aboutMenu();
    // TODO: fileMenu();
}

void MenuBar::on_aboutAction_triggered()
{
    auto about_dialog = AboutDialog();
    about_dialog.exec();
}

void MenuBar::aboutMenu()
{
    QMenu *about_menu     = addMenu(u"&Help"_s);
    QAction *about_action = about_menu->addAction(tr("&About") + " aria2Tray");
    about_action->setIcon(QIcon::fromTheme("help-about"));

    connect(about_action, &QAction::triggered, this, &MenuBar::on_aboutAction_triggered);
}

} // namespace Aria2Tray
