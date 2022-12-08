/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
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

#include <PolkitQt1/Authority>
#include <PolkitQt1/Subject>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

int main(int argc, char* argv[]) {
    QCoreApplication::setSetuidAllowed(true);
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({"trigger", "Reset the system upon reboot"});
    parser.addOption({"undo-trigger", "Don't reset the system upon reboot"});
    parser.addOption({"root-filesystem", "Path to the root filesystem", "path"});
    parser.addOption({"reboot", "Also reboot the system"});
    parser.process(a);

    PolkitQt1::UnixProcessSubject subject(getppid());

    QTextStream out(stdout);
    if (parser.isSet("trigger")) {
        out << "Triggering system reset\n";
        out.flush();

        if (PolkitQt1::Authority::instance()->checkAuthorizationSync("com.vicr123.scallop.reset.trigger-reset", subject, PolkitQt1::Authority::AllowUserInteraction) == PolkitQt1::Authority::Yes) {
            if (QDir("/var/lib/scallop-reset/offline").exists()) QDir("/var/lib/scallop-reset/offline").removeRecursively();

            QDir::root().mkpath("/var/lib/scallop-reset/offline");

            QString rootfs = parser.value("root-filesystem");
            if (rootfs.isEmpty()) rootfs = SCALLOP_PACKAGED_LOCATION;
            QFile::link(rootfs, "/var/lib/scallop-reset/offline/rootfs.squashfs");
            QFile::link("/var/lib/scallop-reset/offline", "/system-update");

            if (parser.isSet("reboot")) {
                // Reboot the system

                QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Reboot");
                message.setArguments({false});
                QDBusConnection::systemBus().call(message);
            }
        } else {
            out << "Not authorized to perform action.\n";
            out.flush();
        }
    } else if (parser.isSet("undo-trigger")) {
        out << "Not triggering system reset\n";
        QFile::remove("/system-update");
    } else {
        out << "--trigger or --undo-trigger argument missing\n";
        return 1;
    }

    return 0;
}
