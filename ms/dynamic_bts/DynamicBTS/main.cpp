#include <QApplication>
#include <QFile>
#include <QLibraryInfo>
#include <QTime>
#include <QTranslator>

#include "serialconnector.h"
#include "gui.h"

int main(int argc, char *argv[])
{
    qsrand(QTime::currentTime().msec());

    QApplication app{argc, argv};

    QTranslator builtin_translator;
    builtin_translator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&builtin_translator);

    QTranslator app_translator;
    app_translator.load(QLocale::system().name(), ":translations");
    app.installTranslator(&app_translator);

    {
        // Load QDarkStyleSheet
        QFile style(":qdarkstyle/style.qss");
        style.open(QFile::ReadOnly | QFile::Text);
        app.setStyleSheet(style.readAll());
    }

    QIcon::setThemeName("tango");

    SerialConnector connector;

    GUI gui{&connector};

    gui.show();
    return app.exec();
}
