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
#include "fstabstate.h"

#include <QProcess>
#include <QTextStream>
#include <QDir>
#include "installerdata.h"

FstabState::FstabState(QState* parent) : QState(parent) {

}


void FstabState::onEntry(QEvent* event) {
    //Start unsquashing the filesystem
    QTextStream(stdout) << tr("Generating fstab") << "\n";

    QProcess* proc = new QProcess();
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [ = ](int exitCode, QProcess::ExitStatus exitStatus) {
        QFile fstabFile(QDir(InstallerData::valueTemp("systemRoot").toString()).absoluteFilePath("etc/fstab"));
        fstabFile.open(QFile::WriteOnly);
        fstabFile.write(proc->readAll());
        fstabFile.close();

        QTextStream(stderr) << tr("Wrote fstab to") << " " << fstabFile.fileName() << "\n";

        emit finished();
    });
    proc->start("genfstab", {"-U", InstallerData::valueTemp("systemRoot").toString()});
}
