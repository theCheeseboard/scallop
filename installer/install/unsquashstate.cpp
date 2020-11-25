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
#include "unsquashstate.h"

#include <QTextStream>
#include <QProcess>
#include "installerdata.h"

UnsquashState::UnsquashState(QState* parent) : QState(parent) {

}


void UnsquashState::onEntry(QEvent* event) {
    //Start unsquashing the filesystem
    QTextStream(stdout) << tr("Unsquashing Filesystem") << "\n";

    QProcess* proc = new QProcess();
    proc->setProcessChannelMode(QProcess::MergedChannels);
    connect(proc, &QProcess::readyRead, this, [ = ] {
        QString data = proc->readAll();

        for (QString part : data.split(" ", Qt::SkipEmptyParts)) {
            if (part.contains("/")) {
                QStringList numberParts = part.split("/");
                if (numberParts.count() == 2) {
                    QTextStream(stdout) << "\r" << tr("Unsquashing Filesystem") << " | " << QString::number(static_cast<int>(numberParts.first().toDouble() / numberParts.at(1).toDouble() * 100)).rightJustified(3) + "%";
                }
            }
        }

        QTextStream(stdout).flush();
    });
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [ = ](int exitCode, QProcess::ExitStatus exitStatus) {
        QTextStream(stdout) << "\r" << tr("Unsquash Complete!") << " | 100%\n";
        emit finished();
    });
    proc->start("unsquashfs", {"-f", "-d", InstallerData::valueTemp("systemRoot").toString(), "/opt/cactus-recovery-media/rootfs.squashfs"});
}
