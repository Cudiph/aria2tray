#include <QApplication>
#include <QCommandLineParser>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>
#include <QStringLiteral>
#include <QTranslator>
#include <QUrl>
#include <QWebSocketServer>
#include <QtLogging>

#include "ipc/wsclient.h"
#include "ipc/wsserver.h"
#include "win.h"

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setOrganizationName("Aria2Tray");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    qSetMessagePattern(u"%{time yyyyMMdd-h:mm:ss.zzz} [%{type}] %{message}"_s);

    // l10n
    QTranslator qtTranslator;
    if (qtTranslator.load(QLocale::system(), u"qtbase"_s, u"_"_s,
                          QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&qtTranslator);
    }

    QTranslator appTranslator;
#ifdef A2T_DATA_DIR
    if (appTranslator.load(u"aria2tray_"_s + QLocale::system().name(),
                           QStringLiteral(A2T_DATA_DIR) + u"/translations"_s)) {
#else
    if (appTranslator.load(u"aria2tray_"_s + QLocale::system().name(),
                           QApplication::applicationDirPath() % "/translations")) {
#endif // A2T_DATA_DIR
        app.installTranslator(&appTranslator);
    }

    // options parsing
    QCommandLineParser parser;
    parser.addOptions({
        {"hide-window", app.translate("main", "Hide main windows when opening")},
    });
    parser.process(app);

    qInfo() << "Starting application.";

    Aria2Tray::WSServer wsServer(A2T_IPC_PORT);
    if (!wsServer.isListening()) {
        QUrl url;
        url.setScheme("ws");
        url.setHost("127.0.0.1");
        url.setPort(A2T_IPC_PORT);
        Aria2Tray::WSClient wsClient(url);
        return app.exec();
    }

    Aria2Tray::Window win;
    wsServer.setWindow(&win);
    win.connectWSServer(&wsServer);

    if (!parser.isSet("hide-window"))
        win.show();

    return app.exec();
}
