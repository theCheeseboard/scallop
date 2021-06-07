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

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QDBusArgument>
#include <QDBusInterface>
#include <QProcess>
#include <QThread>
#include <QDebug>
#include <QTemporaryFile>
#include <QDebug>
#include <QDir>
#include <unistd.h>
#include <pwd.h>

QList<QDBusObjectPath> users() {
    //Determine if there are any accounts on this system
    QDBusMessage listMessage = QDBusMessage::createMethodCall("org.freedesktop.Accounts", "/org/freedesktop/Accounts", "org.freedesktop.Accounts", "ListCachedUsers");
    QDBusMessage listReply = QDBusConnection::systemBus().call(listMessage);
    QDBusArgument listArg = listReply.arguments().first().value<QDBusArgument>();
    QList<QDBusObjectPath> userPaths;
    listArg >> userPaths;

    QList<QDBusObjectPath> remove;

    for (QDBusObjectPath path : userPaths) {
        QDBusInterface interface("org.freedesktop.Accounts", path.path(), "org.freedesktop.Accounts.User", QDBusConnection::systemBus());
        if (interface.property("SystemAccount").toBool()) remove.append(path);
    }

    for (QDBusObjectPath path : remove) {
        userPaths.removeOne(path);
    }

    return userPaths;
}

int main(int argc, char* argv[]) {
    QCoreApplication::setSetuidAllowed(true);
    QCoreApplication a(argc, argv);

    bool haveUser = !users().isEmpty();

    if (!haveUser) {
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

        //Start up lightdm with the scallop onboarding session
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("QT_QPA_PLATFORMTHEME", "thedesk-platform");
        env.insert("XAUTHORITY", xauthFile.fileName());
//        env.insert("DISPLAY", ":0");

        QProcess proc;
        proc.setProcessChannelMode(QProcess::ForwardedChannels);
        proc.setProcessEnvironment(env);
        proc.start("scallop-onboarding", QStringList());
        proc.waitForFinished(-1);

        if (proc.exitCode() != 0) return 0; //Bail

        //Copy the created configuration files to the new user account
        QList<QDBusObjectPath> users = ::users();
        if (users.isEmpty()) {
            //???
            qDebug() << "No user found.";
        } else {
            //Use the user with the lowest uid
            qulonglong uid = -1;
            for (QDBusObjectPath path : users) {
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

            //Set the permissions
            QProcess chown;
            chown.start("chown", {"-R", QStringLiteral("%1:%2").arg(newUserInformation->pw_uid).arg(newUserInformation->pw_gid), userHome.absoluteFilePath(".config")});
            chown.waitForFinished();
        }
    }

    return 0;
}
