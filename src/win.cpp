#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QStringLiteral>
#include <QTabBar>
#include <QVBoxLayout>
#include <cstdlib>

#include "ipc/wsserver.h"
#include "process.h"
#include "win.h"

using namespace Qt::Literals::StringLiterals;

namespace Aria2Tray {
const int initw = 680;
const int inith = 540;

Window::Window()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        auto msgBox = QMessageBox();
        msgBox.setText(tr("System tray is not available in this desktop."));
        msgBox.exec();
        exit(EXIT_FAILURE);
    }

    createTray();

    auto centralWidget = new QWidget(this);
    auto vLayout       = new QVBoxLayout(centralWidget);
    vLayout->addWidget(createTabBar());
    setCentralWidget(centralWidget);

    // move to center of the screen (work same without)
    auto screenSize = QApplication::primaryScreen()->size();
    auto xPos       = screenSize.width() / 2 - initw / 2;
    auto yPos       = screenSize.height() / 2 - inith / 2;
    move(xPos, yPos);

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
        trayIcon->setToolTip(tr("Service isn't running."));
        break;
    case QProcess::Starting:
        trayIcon->setToolTip(tr("Service is starting..."));
        break;
    case QProcess::Running:
        trayIcon->setToolTip(tr("Service is running."));
        break;
    }
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

QSystemTrayIcon *Window::createTray()
{
    trayIcon = new QSystemTrayIcon;

#ifdef Q_OS_WIN32
    trayIcon->setIcon(QIcon(QApplication::applicationDirPath() + u"/icon.ico"_s));
#else
    trayIcon->setIcon(QIcon(QStringLiteral(A2T_DATA_DIR) + u"/aria2tray.svg"_s));
#endif // Q_OS_WIN32
    trayIcon->setContextMenu(createTrayMenu());
    trayIcon->setToolTip(tr("Service not running."));
    trayIcon->show();

    connect(trayIcon, &QSystemTrayIcon::activated, this, &Window::iconActivated);
    connect(Process::instance(), &QProcess::stateChanged, this, &Window::onStateChange);
    return trayIcon;
}

QMenu *Window::createTrayMenu()
{
    trayMenu = new QMenu;
    trayMenu->setMinimumWidth(150);

    quitAction = new QAction;
    quitAction->setText(tr("Quit"));
    quitAction->setChecked(false);
    connect(quitAction, &QAction::triggered, this, &Window::handleQuit);

    restartAction = new QAction;
    restartAction->setText(tr("Restart"));
    connect(restartAction, &QAction::triggered, this, &Window::handleRestart);

    trayMenu->addAction(restartAction);
    trayMenu->addSeparator();
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

void Window::restoreState()
{
    QSettings settings;
    settings.beginGroup(u"window"_s);
    auto w = settings.value(u"width"_s, initw).toInt();
    auto h = settings.value(u"height"_s, inith).toInt();
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

} // namespace Aria2Tray
