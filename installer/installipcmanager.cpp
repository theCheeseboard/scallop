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
#include "installipcmanager.h"

#include <QCoreApplication>
#include "installerdata.h"
#include <QProcess>
#include <tlogger.h>

struct InstallIpcManagerPrivate {
    QProcess* installerProcess;
    bool finishedSuccessfully = false;
};

InstallIpcManager::InstallIpcManager(QObject* parent) : QObject(parent) {
    d = new InstallIpcManagerPrivate();

    connect(this, &InstallIpcManager::success, this, [ = ] {
        d->finishedSuccessfully = true;
    });
}

InstallIpcManager* InstallIpcManager::instance() {
    static InstallIpcManager* instance = new InstallIpcManager();
    return instance;
}

void InstallIpcManager::startInstalling() {
    emit instance()->messageChanged(tr("Starting Installation..."));
    emit instance()->progressChanged(-1);

    QProcess* installerProcess = new QProcess();
    instance()->d->installerProcess = installerProcess;

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert("QT_QPA_PLATFORM", "offscreen");

    connect(installerProcess, &QProcess::readyRead, [ = ] {
        QString line = installerProcess->readAll().trimmed();
        line = line.split("\n").last();
        line = line.split("\r").last();

        QString message;
        int progress = -1;

        if (line.contains("|")) {
            message = line.left(line.indexOf("|")).trimmed();
            QString progressString = line.mid(line.indexOf("|") + 1).trimmed();
            progressString.remove("%");
            progress = progressString.toInt();
        } else {
            message = line;
        }

        emit instance()->messageChanged(message);
        emit instance()->progressChanged(progress);
    });
    connect(installerProcess, &QProcess::readyReadStandardError, [ = ] {
        QString standardError = installerProcess->readAllStandardError();
        for (QString line : standardError.split("\n", Qt::SkipEmptyParts)) {
            tDebug("InstallProcess") << line;
        }
    });
    connect(installerProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [ = ](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == 0) {
            emit instance()->success();
        } else {
            emit instance()->failure();
        }
    });
    installerProcess->setProcessEnvironment(environment);
    installerProcess->start("sudo", {QCoreApplication::applicationFilePath(), "--install"});
    installerProcess->write(InstallerData::exportData());
    installerProcess->closeWriteChannel();
}

bool InstallIpcManager::finishedSuccessfully() {
    return instance()->d->finishedSuccessfully;
}
