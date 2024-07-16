#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>
#include <QTranslator>
#include <QtLogging>

#include "win.h"

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator qtTranslator;

    if (qtTranslator.load(QLocale::system(), u"qtbase"_s, u"_"_s,
                          QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&qtTranslator);
    }

    QTranslator translator;
    if (translator.load(u"aria2tray_"_s + QLocale::system().name(),
                        QApplication::applicationDirPath() % "/translations")) {
        app.installTranslator(&translator);
    }

    QSettings::setDefaultFormat(QSettings::IniFormat);
    app.setQuitOnLastWindowClosed(false);
    app.setOrganizationName("Aria2Tray");
    qSetMessagePattern(u"%{time yyyyMMdd-h:mm:ss.zzz} [%{type}] %{message}"_s);
    qInfo() << "Starting application.";

    Aria2Tray::Window win;
    win.show();

    return app.exec();
}
