#include "mainwindow.h"
#include "systemonboarding.h"
#include "reset.h"
#include <QApplication>
#include <QSettings>
#include <QDesktopWidget>
#include <QLibraryInfo>
#include <QDBusMetaType>
#include <iostream>
#include <thread>
#include <chrono>
#include <QMovie>

#undef bool

Branding branding;
QTranslator *qtTranslator, *tsTranslator;

int main(int argc, char *argv[])
{
    qputenv("XDG_SESSION_CLASS", "greeter");
    qputenv("QT_QPA_PLATFORMTHEME", "ts");
    QProcess* XServerProcess = NULL;

    if (argc > 1) {
        QString firstArg = argv[1];
        if (firstArg == "--help" || firstArg == "-h") {
            std::cout << "Scallop System Configuration Tool\n";
            std::cout << "Usage: scallop [vtx] [options]\n";
            std::cout << "       vtx: The virtual terminal to start the X server on, in the form of vtx.\n";
            std::cout << "\n";
            std::cout << "Options:";
            std::cout << "  -h, --help                   Show this help output\n";
            std::cout << "                               This must be the first argument passed to Scallop.\n";
            std::cout << "      --install                Open Scallop in system installation mode\n";
            std::cout << "      --onboard                Open Scallop in Onboarding mode\n";
            std::cout << "      --reset                  Open Scallop in system reset mode\n";
            std::cout << "\n";
            std::cout << "If vtx is not passed, and $DISPLAY=\"\", theDM will start an X server on the "
                         "current virtual terminal.\n";
            return 0;
        }
    }

    //Start the X server if it's not already started
    if (qgetenv("DISPLAY") == "") {
        //Start the X server
        QString currentVt = "";
        if (argc > 1) {
            QString firstArg = argv[1];
            if (firstArg.contains("vt")) {
                currentVt = firstArg;

                //Switch to the required VT
                QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1/seat/self", "org.freedesktop.login1.Seat", "SwitchTo");
                QList<QVariant> args;
                args.append(firstArg.remove("vt").toUInt());
                message.setArguments(args);
                QDBusConnection::systemBus().call(message);
            }
        }

        if (currentVt == "") {
            QProcess vtGet;
            vtGet.start("fgconsole");
            vtGet.waitForFinished();
            currentVt = "vt" + QString(vtGet.readAll());
        }

        bool serverStarted = false;
        int display = 0;
        do {
            qDebug() << QString("Starting the X Server as display :" + QString::number(display) + " on " + currentVt).toStdString().data();
            XServerProcess = new QProcess();
            XServerProcess->setProcessChannelMode(QProcess::ForwardedChannels);
            XServerProcess->start("/usr/bin/X :" + QString::number(display) + " " + currentVt);
            XServerProcess->waitForFinished(1000);

            if (XServerProcess->state() != QProcess::Running) {
                display++;
            } else {
                if (!qputenv("DISPLAY", QString(":" + QString::number(display)).toUtf8())) {
                    qDebug() << "Could not set DISPLAY environment variable.";
                    XServerProcess->kill();
                    return 1;
                }

                serverStarted = true;
            }
        } while (!serverStarted);
    }

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
