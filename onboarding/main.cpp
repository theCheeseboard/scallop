/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "mainwindow.h"

#include <tapplication.h>
#include <statemanager.h>
#include <localemanager.h>
#include <powermanager.h>
#include <onboardingmanager.h>
#include <tsettings.h>
#include <QProcess>
#include <plugins/pluginmanager.h>
#include <onboarding/onboardingcontroller.h>
#include <unistd.h>
#include <pwd.h>

#include "endsession.h"

#include "OnboardingPages/onboardingcompleteoobe.h"

#include <tlogger.h>

int main(int argc, char* argv[]) {
    if (getuid() != 0) {
        QTextStream(stdout) << "This program must be run as root.\n";
        return 1;
    }

    struct passwd* setupUserInformation = getpwnam("setup");

    setgid(setupUserInformation->pw_gid);
    setuid(setupUserInformation->pw_uid);
    qputenv("HOME", setupUserInformation->pw_dir);

    QTemporaryDir runDir;
    qputenv("XDG_RUNTIME_DIR", runDir.path().toUtf8());

    //Start a D-Bus daemon
    QProcess dbus;
    dbus.start("dbus-launch", QStringList());
    dbus.waitForFinished();
    QString dbusOutput = dbus.readAll();
    qDebug() << dbusOutput;
    for (QString line : dbusOutput.split("\n")) {
        QStringList parts = line.split("=");
        if (parts.count() == 2) qputenv(parts.first().toUtf8().data(), parts.at(1).toUtf8());
    }

    QProcess pulseProc;
//    pulseProc.start("start-pulseaudio-x11", QStringList());
    pulseProc.start("pulseaudio", {"--daemonize"});
    pulseProc.waitForStarted();

    tApplication a(argc, argv);
    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("vicr123.com");
    a.setApplicationName("theDesk");

    QProcess kwinProc;
    kwinProc.start("kwin_x11", QStringList());
    kwinProc.waitForStarted();

    StateManager::instance();
    StateManager::localeManager()->addTranslationSet({
        a.applicationDirPath() + "/translations",
        "/usr/share/thedesk/translations"
    });

    tSettings::registerDefaults(a.applicationDirPath() + "/defaults.conf");
    tSettings::registerDefaults("/etc/scallop/onboarding/defaults.conf");
    tSettings::registerDefaults("/etc/theSuite/theDesk/defaults.conf");

    //Read seeded settings
    if (QFile::exists("/etc/scallop/seeded-settings")) {
        QSettings seedFile("/etc/scallop/seeded-settings", QSettings::IniFormat);
        QString language = seedFile.value("Scallop/language").toString();


        QLocale locale(language);
        LocaleManager* localeManager = StateManager::localeManager();
        if (localeManager->locales().contains(locale)) localeManager->removeLocale(locale);
        localeManager->prependLocale(locale);
    }

    QObject::connect(StateManager::powerManager(), &PowerManager::powerOffConfirmationRequested, [ = ] {
        EndSession::showDialog();
    });
    QObject::connect(StateManager::onboardingManager(), &OnboardingManager::onboardingRequired, [ = ] {
        StateManager::onboardingManager()->addOnboardingStep(new OnboardingCompleteOobe());

        StateManager::onboardingManager()->setDateVisible(false);
    });

    PluginManager::instance()->scanPlugins();
    OnboardingController::performOnboarding();

    return 0;
}
