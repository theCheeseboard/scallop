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
#include "plymouth.h"

#include <QTextStream>
#include <QProcess>

Plymouth::Plymouth(QObject* parent) : QObject(parent), output(stdout) {

}

void Plymouth::setSystemUpgradeMode() {
    runCommand({"change-mode", "--system-upgrade"});
}

void Plymouth::setProgress(int progress) {
    runCommand({"system-update", QStringLiteral("--progress=%1").arg(progress)});

    this->output << "Resetting system. " << progress << "% complete.\n";
    this->output.flush();
}

void Plymouth::runCommand(QStringList args) {
    QProcess* proc = new QProcess();
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), proc, &QProcess::deleteLater);
    proc->start("plymouth", args);
}
