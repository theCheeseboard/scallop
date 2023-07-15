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
#include <QCoreApplication>

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QTemporaryFile>
#include <QThread>
#include <grp.h>
#include <iostream>
#include <pwd.h>
#include <sys/wait.h>
#include <unistd.h>

#include <X11/Xlib.h>

QList<QDBusObjectPath> users() {
    // Determine if there are any accounts on this system
    QDBusMessage listMessage = QDBusMessage::createMethodCall("org.freedesktop.Accounts", "/org/freedesktop/Accounts", "org.freedesktop.Accounts", "ListCachedUsers");
    QDBusMessage listReply = QDBusConnection::systemBus().call(listMessage);
    QDBusArgument listArg = listReply.arguments().at(0).value<QDBusArgument>();
    QList<QDBusObjectPath> userPaths;
    listArg >> userPaths;

    QList<QDBusObjectPath> remove;

    for (const QDBusObjectPath& path : qAsConst(userPaths)) {
        QDBusInterface interface("org.freedesktop.Accounts", path.path(), "org.freedesktop.Accounts.User", QDBusConnection::systemBus());
        if (interface.property("SystemAccount").toBool()) remove.append(path);
    }

    for (const QDBusObjectPath& path : remove) {
        userPaths.removeOne(path);
    }

    return userPaths;
}

int main(int argc, char* argv[]) {
    QCoreApplication::setSetuidAllowed(true);
    QCoreApplication a(argc, argv);

    bool haveUser = !users().isEmpty();

    if (!haveUser) {
        auto display = XOpenDisplay(nullptr);
        if (!display) {
            QTextStream(stderr) << "Failed to open display\n";
            return 1;
        }

        QByteArray serverAddressType("localuser");
        QByteArray serverAddressValue("setup");
        XServerInterpretedAddress serverInterpretedAddress;
        serverInterpretedAddress.type = serverAddressType.data();
        serverInterpretedAddress.typelength = serverAddressType.length();
        serverInterpretedAddress.value = serverAddressValue.data();
        serverInterpretedAddress.valuelength = serverAddressValue.length();

        XHostAddress hostAddress;
        hostAddress.family = FamilyServerInterpreted;
        hostAddress.address = reinterpret_cast<char*>(&serverInterpretedAddress);
        XAddHost(display, &hostAddress);
        //        XCloseDisplay(display);

        QTemporaryFile xauthFile;
        xauthFile.open();

        QFile oldxauthFile(qgetenv("XAUTHORITY"));
        oldxauthFile.open(QFile::ReadOnly);
        xauthFile.write(oldxauthFile.readAll());
        xauthFile.close();
        oldxauthFile.close();

        QProcess chmod;
        chmod.start("chmod", {"+r", xauthFile.fileName()});
        chmod.waitForFinished();

        struct passwd* setupUserInformation = getpwnam("setup");
        int setupUserGroupCount = 0;
        getgrouplist(setupUserInformation->pw_name, setupUserInformation->pw_gid, nullptr, &setupUserGroupCount);
        __gid_t setupUserGroups[setupUserGroupCount];
        getgrouplist(setupUserInformation->pw_name, setupUserInformation->pw_gid, setupUserGroups, &setupUserGroupCount);

        // Start up lightdm with the scallop onboarding session
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("QT_QPA_PLATFORMTHEME", "thedesk-platform");
        env.insert("XAUTHORITY", xauthFile.fileName());
        env.insert("LD_LIBRARY_PATH", "/opt/KF5-qt6/lib");
        env.insert("HOME", setupUserInformation->pw_dir);
        env.insert("XDG_DATA_DIRS", "/usr/share");
        //        env.insert("DISPLAY", ":0");

        QProcess proc;
        proc.setChildProcessModifier([&setupUserInformation, &setupUserGroupCount, &setupUserGroups] {
            setgroups(setupUserGroupCount, setupUserGroups);
            int gidSetReturn = setresgid(setupUserInformation->pw_gid, setupUserInformation->pw_gid, setupUserInformation->pw_gid);
            if (gidSetReturn != 0) {
                std::cerr << "setresgid failed\n";
                std::cerr << "Could not drop privileges\n";
                ::_exit(1);
            }
            int uidSetReturn = setresuid(setupUserInformation->pw_uid, setupUserInformation->pw_uid, setupUserInformation->pw_uid);
            if (uidSetReturn != 0) {
                std::cerr << "setresuid failed\n";
                std::cerr << "Could not drop privileges\n";
                ::_exit(1);
            }
        });
        proc.setProcessChannelMode(QProcess::ForwardedChannels);
        proc.setProcessEnvironment(env);
        proc.start("/usr/bin/scallop-onboarding", QStringList());
        proc.waitForStarted(-1);
        proc.waitForFinished(-1);

        if (proc.exitCode() != 0) {
            QTextStream(stderr) << "scallop-onboarding crashed\n";
            return 0; // Bail
        }

        // Copy the created configuration files to the new user account
        QList<QDBusObjectPath> users = ::users();
        if (users.isEmpty()) {
            //???
            qDebug() << "No user found.";
        } else {
            // Use the user with the lowest uid
            qulonglong uid = -1;
            for (const QDBusObjectPath& path : qAsConst(users)) {
                QDBusInterface interface("org.freedesktop.Accounts", path.path(), "org.freedesktop.Accounts.User", QDBusConnection::systemBus());
                qulonglong newUid = interface.property("Uid").toULongLong();
                if (newUid < uid) uid = newUid;
            }

            struct passwd* newUserInformation = getpwuid(uid);
            struct passwd* setupUserInformation = getpwnam("setup");

            qDebug() << "Configuring for " << newUserInformation->pw_name;

            QDir userHome(newUserInformation->pw_dir);
            QDir setupHome(setupUserInformation->pw_dir);

            setupHome.rename(".config", userHome.absoluteFilePath(".config"));

            // Set the permissions
            QProcess chown;
            chown.start("chown", {"-R", QStringLiteral("%1:%2").arg(newUserInformation->pw_uid).arg(newUserInformation->pw_gid), userHome.absoluteFilePath(".config")});
            chown.waitForFinished();
        }
    }

    return 0;
}
