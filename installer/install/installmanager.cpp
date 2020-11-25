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
#include "installmanager.h"

#include <installerdata.h>
#include <QTextStream>
#include <QStateMachine>
#include <QFinalState>
#include <QCoreApplication>
#include <QJsonValue>
#include <QProcess>
#include <QDir>

#include "diskmanagementstate.h"
#include "mountstate.h"
#include "unsquashstate.h"
#include "bootloaderstate.h"

struct InstallManagerPrivate {
    QStateMachine* stateMachine;
};

InstallManager::InstallManager(QObject* parent) : QObject(parent) {
    d = new InstallManagerPrivate();
    d->stateMachine = new QStateMachine();

    //Read metadata
    QTextStream(stderr) << "Waiting for metadata...\n";

    //We want to read from stdin
    QTextStream s(stdin);

    QStringList buf;
    QString currentLine;
    do {
        currentLine = s.readLine();
        buf.append(currentLine);
    } while (currentLine != "");

    bool success = InstallerData::importData(buf.join("\n").toUtf8());
    if (success) {
        QTextStream(stderr) << "Metadata received!\n";
    } else {
        QTextStream(stderr) << "Invalid metadata!\n";
        return;
    }

    //Set up the states
    DiskManagementState* diskState = new DiskManagementState();
    MountState* mountState = new MountState();
    UnsquashState* unsquashState = new UnsquashState();
    BootloaderState* bootloaderState = new BootloaderState();
    QState* cleanupState = new QState();
    QFinalState* finalState = new QFinalState();
    QState* errorCleanupState = new QState();
    QFinalState* errorFinalState = new QFinalState();

    diskState->addTransition(diskState, &DiskManagementState::finished, mountState);
    mountState->addTransition(mountState, &MountState::finished, unsquashState);
    unsquashState->addTransition(unsquashState, &UnsquashState::finished, bootloaderState);
    bootloaderState->addTransition(bootloaderState, &BootloaderState::finished, cleanupState);

    diskState->addTransition(diskState, &DiskManagementState::failure, errorCleanupState);
    bootloaderState->addTransition(bootloaderState, &BootloaderState::failure, errorCleanupState);
    cleanupState->addTransition(this, &InstallManager::cleanupDone, finalState);
    errorCleanupState->addTransition(this, &InstallManager::cleanupDone, errorFinalState);

    d->stateMachine->addState(diskState);
    d->stateMachine->addState(mountState);
    d->stateMachine->addState(unsquashState);
    d->stateMachine->addState(bootloaderState);
    d->stateMachine->addState(cleanupState);
    d->stateMachine->addState(finalState);
    d->stateMachine->addState(errorCleanupState);
    d->stateMachine->addState(errorFinalState);

    connect(cleanupState, &QState::entered, this, &InstallManager::cleanup);
    connect(errorCleanupState, &QState::entered, this, &InstallManager::cleanup);

    connect(finalState, &QStateMachine::entered, this, [ = ] {
        QTextStream(stdout) << tr("Installation complete!");
        QTextStream(stdout).flush();
        QCoreApplication::exit();
    });
    connect(errorFinalState, &QStateMachine::entered, this, [ = ] {
        QTextStream(stdout) << tr("Installation failed!");
        QTextStream(stdout).flush();
        QCoreApplication::exit(1);
    });

    //Start running the state machine
    d->stateMachine->setInitialState(diskState);
    d->stateMachine->start();
}

InstallManager::~InstallManager() {
    d->stateMachine->deleteLater();
    delete d;
}

void InstallManager::cleanup() {
    QTextStream(stdout) << tr("Finishing up...") << "\n";

    //Pre-seed settings
    QString sysRootPath = InstallerData::valueTemp("systemRoot").toString();
    if (sysRootPath.isEmpty()) {
        emit cleanupDone();
    } else {
        QDir sysRoot(sysRootPath);
        QFile seedFile(sysRoot.absoluteFilePath("/etc/scallop/seeded-settings"));
        seedFile.open(QFile::WriteOnly);
        seedFile.write("[Scallop]\n");
        seedFile.write(("language=" + InstallerData::value("language").toString()).toUtf8());
        seedFile.close();

        //Unmount all disks
        QProcess* proc = new QProcess();
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [ = ](int exitCode, QProcess::ExitStatus exitStatus) {
            proc->deleteLater();
            emit cleanupDone();
        });
        proc->start("umount", {"-R", sysRoot.path()});
    }
}
