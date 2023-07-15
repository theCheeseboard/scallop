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

#include <QProcess>
#include <grp.h>
#include <localemanager.h>
#include <onboarding/onboardingcontroller.h>
#include <onboardingmanager.h>
#include <plugins/pluginmanager.h>
#include <powermanager.h>
#include <pwd.h>
#include <statemanager.h>
#include <tapplication.h>
#include <tsettings.h>
#include <unistd.h>

#include "endsession.h"

#include "OnboardingPages/onboardingcompleteoobe.h"
#include "OnboardingPages/onboardinghostname.h"

#include <tlogger.h>

#include <PulseAudioQt/Context>
#include <PulseAudioQt/Sink>

int main(int argc, char* argv[]) {
    bool debug = false;
    for (auto i = 0; i < argc; i++) {
        auto arg = argv[i];
        if (strcmp(arg, "--debug") == 0) debug = true;
    }

    if (!debug) {
        //        struct passwd* setupUserInformation = getpwnam("setup");
        //        initgroups(setupUserInformation->pw_name, setupUserInformation->pw_gid);

        QTemporaryDir runDir;
        qputenv("XDG_RUNTIME_DIR", runDir.path().toUtf8());

        // Start a D-Bus daemon
        QProcess dbus;
        dbus.start("dbus-launch", QStringList());
        dbus.waitForFinished();
        QString dbusOutput = dbus.readAll();
        for (const QString& line : dbusOutput.split("\n")) {
            QStringList parts = line.split("=");
            if (parts.count() == 2)
                qputenv(parts.first().toUtf8().data(), parts.at(1).toUtf8());
        }

        QProcess pulseProc;
        pulseProc.start("pulseaudio", QStringList());
        pulseProc.waitForStarted();

        QTextStream(stderr) << "Xauthority:";
        QTextStream(stderr) << qEnvironmentVariable("XAUTHORITY");
    }

    tApplication a(argc, argv);
    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("vicr123.com");
    a.setApplicationName("theDesk");
    a.setApplicationShareDir("scallop/onboarding");
    a.installTranslators();

    if (!debug) {
        QProcess kwinProc;
        kwinProc.start("kwin_x11", QStringList());
        kwinProc.waitForStarted();
    }

    StateManager::instance();
    StateManager::localeManager()->addTranslationSet(
        {a.applicationDirPath() + "/translations",
            "/usr/share/thedesk/translations"});

    for (const auto& shareDir : a.systemShareDirs()) {
        tSettings::registerDefaults(QStringLiteral("%1/defaults/scalloponboarding.conf").arg(shareDir));
        tSettings::registerDefaults(QStringLiteral("%1/defaults/thedesk.conf").arg(shareDir));
    }

    if (!debug) {
        // Turn up the volume
        QObject::connect(PulseAudioQt::Context::instance(),
            &PulseAudioQt::Context::sinkAdded,
            [=](PulseAudioQt::Sink* sink) {
            sink->setVolume(PulseAudioQt::normalVolume() * 0.5);
            sink->setMuted(false);
        });
        for (PulseAudioQt::Sink* sink : PulseAudioQt::Context::instance()->sinks()) {
            sink->setVolume(PulseAudioQt::normalVolume() * 0.5);
            sink->setMuted(false);
        }
    }

    // Read seeded settings
    if (QFile::exists("/etc/scallop/seeded-settings")) {
        QSettings seedFile("/etc/scallop/seeded-settings", QSettings::IniFormat);
        QString language = seedFile.value("Scallop/language").toString();

        QLocale locale(language);
        LocaleManager* localeManager = StateManager::localeManager();
        if (localeManager->locales().contains(locale))
            localeManager->removeLocale(locale);
        localeManager->prependLocale(locale);
    }

    QObject::connect(StateManager::powerManager(),
        &PowerManager::powerOffConfirmationRequested,
        [=] {
        EndSession::showDialog();
    });
    QObject::connect(StateManager::onboardingManager(),
        &OnboardingManager::onboardingRequired, [=] {
            StateManager::onboardingManager()->addOnboardingStep(
                new OnboardingCompleteOobe());
            StateManager::onboardingManager()->addOnboardingStep(
                new OnboardingHostname());

            StateManager::onboardingManager()->setDateVisible(false);
        });

    PluginManager::instance()->scanPlugins();
    OnboardingController::performOnboarding(true);

    return 0;
}
