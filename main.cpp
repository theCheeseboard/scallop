#include "mainwindow.h"
#include "systemonboarding.h"
#include "reset.h"
#include <QApplication>
#include <QSettings>
#include <QDesktopWidget>
#include <QLibraryInfo>
#include <QDBusMetaType>

#undef bool

Branding branding;
QTranslator *qtTranslator, *tsTranslator;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Load branding details
    QSettings brandingSettings("/etc/scallop/branding.conf");
    brandingSettings.beginGroup("branding");
    branding.name = brandingSettings.value("name", "theShell OS").toString();
    brandingSettings.endGroup();

    qtTranslator = new QTranslator;
    qtTranslator->load("qt_" + QLocale().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(qtTranslator);

    tsTranslator = new QTranslator;
    tsTranslator->load(QLocale().name(), "/usr/share/scallop/translations");
    a.installTranslator(tsTranslator);

    qDBusRegisterMetaType<QList<QVariantMap>>();
    qDBusRegisterMetaType<QMap<QString, QVariantMap>>();

    //Determine mode
    if (QFile("/etc/scallop-live").exists() || a.arguments().contains("--install")) {
        //Installation mode
        MainWindow* w = new MainWindow;
        w->show();
    } else if (a.arguments().contains("--onboard")) {
        //OEM Onboarding mode
        SystemOnboarding* w = new SystemOnboarding;
        w->showFullScreen();
    } else if (a.arguments().contains("--reset")) {
        //Reset mode
        Reset* w = new Reset;
        w->showFullScreen();
    } else {
        //Configuration mode
    }

    return a.exec();
}

bool isEFI() {
    return QDir("/sys/firmware/efi").exists();
}

float getDPIScaling() {
    float currentDPI = QApplication::desktop()->logicalDpiX();
    return currentDPI / (float) 96;
}
