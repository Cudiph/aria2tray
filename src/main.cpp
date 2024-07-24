#include <QApplication>
#include <QCommandLineParser>
#include <QLibraryInfo>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLocale>
#include <QSettings>
#include <QStringLiteral>
#include <QTranslator>
#include <QtLogging>

#include "win.h"

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    const QString KEY = u"nPABAaUkVyDiQjBfhWfmTFvb"_s;
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setOrganizationName("Aria2Tray");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    qSetMessagePattern(u"%{time yyyyMMdd-h:mm:ss.zzz} [%{type}] %{message}"_s);

    // client
    QLocalSocket socket;
    socket.connectToServer(KEY);
    socket.waitForConnected(3000);
    if (socket.state() == QLocalSocket::ConnectedState) {
        socket.close();
        return 0;
    }

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

    Aria2Tray::Window win;
    if (!parser.isSet("hide-window"))
        win.show();

    // server
    QLocalServer server;
    QObject::connect(&server, &QLocalServer::newConnection, [&win] { win.show(); });
    QLocalServer::removeServer(KEY);
    server.listen(KEY);

    return app.exec();
}
