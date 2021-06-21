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

#include <QTextStream>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QFileInfo>
#include <QTimer>
#include <QDir>
#include <QProcess>
#include <unistd.h>
#include <sys/reboot.h>
#include "plymouth.h"

int prog = 0;

int main(int argc, char* argv[]) {
    QCoreApplication a(argc, argv);

    if (geteuid() != 0) {
        QTextStream out(stderr);
        out << "This utility must be run as root.\n";
        out.flush();
        return 1;
    }

    QFileInfo checkSymlink("/system-update");

    //Ensure the symlink exists
    if (!checkSymlink.exists()) return 0;
    if (!checkSymlink.isSymLink()) return 0;

    //Ensure the symlink points to the correct place
    if (checkSymlink.symLinkTarget() != "/opt/cactus-recovery-media") return 0;

    QFile::remove(checkSymlink.filePath());

    Plymouth* plymouth = new Plymouth();
    plymouth->setSystemUpgradeMode();

    QTextStream out(stdout);
    out << "Resetting this device.\n";
    out.flush();

    //Perform the system reset!
    QDir::root().mkpath("/tmp/scallop-reset-root");

    QProcess mountProc;
    mountProc.start("mount", {"/opt/cactus-recovery-media/rootfs.squashfs", "/tmp/scallop-reset-root"});
    mountProc.waitForFinished();

    QStringList rsyncArgs = {
        "-a",
        "--info=progress2",
        "--no-inc-recursive",
        "--delete"
    };

    for (const QString& exclusion : QStringList({"/tmp", "/proc", "/sys", "/media", "/opt/cactus-recovery-media", "/boot", "/apps", "/host"})) {
        rsyncArgs.append(QStringLiteral("--exclude=%1").arg(exclusion));
    }

    rsyncArgs.append({
        "/tmp/scallop-reset-root/",
        "/"
    });
//    rsyncArgs.append({
//        "/tmp/scallop-reset-root/",
//        "/test"
//    });

    QProcess* rsyncProc = new QProcess();
//    rsyncProc->setProcessChannelMode(QProcess::ForwardedChannels);
    rsyncProc->start("rsync", rsyncArgs);
    QObject::connect(rsyncProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [ = ] {
        QCoreApplication::exit();
    });
    QObject::connect(rsyncProc, &QProcess::readyRead, [ = ] {
        QStringList lines = QString(rsyncProc->readAll()).split("\r");
        for (QString line : lines) {
            QStringList parts = line.split(" ", Qt::SkipEmptyParts);
            if (parts.count() >= 2) {
                QString perc = parts.at(1);
                if (perc.endsWith("%")) {
                    perc = perc.remove("%");

                    bool ok;
                    int percInt = perc.toInt(&ok);
                    if (ok) plymouth->setProgress(percInt);
                }
            }
        }
    });

    a.exec();

    //Reboot the computer
    //Don't use systemd as it might not be working right now
    sync();
    reboot(RB_AUTOBOOT);

//    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Reboot");
//    message.setArguments({false});
//    QDBusConnection::systemBus().call(message);

    return 0;
}
