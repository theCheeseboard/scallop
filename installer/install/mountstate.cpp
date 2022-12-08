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
#include "mountstate.h"

#include "installerdata.h"
#include <DriveObjects/blockinterface.h>
#include <DriveObjects/diskobject.h>
#include <QFinalState>
#include <driveobjectmanager.h>

struct MountStatePrivate {
        QTemporaryDir mountDir;
};

MountState::MountState(QState* parent) :
    QStateMachine(parent) {
    d = new MountStatePrivate();
    d->mountDir.setAutoRemove(false);
}

MountState::~MountState() {
    delete d;
}

void MountState::onEntry(QEvent* event) {
    // Add states
    QList<QPair<QString, QString>> mounts = InstallerData::valueTemp("mounts").value<QList<QPair<QString, QString>>>();

    // Sort the mounts by length
    std::sort(mounts.begin(), mounts.end(), [=](QPair<QString, QString> first, QPair<QString, QString> second) {
        return first.first.length() < second.first.length();
    });

    InstallerData::insertTemp("systemRoot", d->mountDir.path());

    QState* previousState = nullptr;
    for (QPair<QString, QString> mount : mounts) {
        QState* state = new QState();
        connect(state, &QState::entered, this, [=] {
            DiskObject* mountDrive;

            if (mount.second.startsWith("/org/freedesktop/UDisks2")) {
                mountDrive = DriveObjectManager::diskForPath(QDBusObjectPath(mount.second));
            } else {
                mountDrive = DriveObjectManager::diskByBlockName(mount.second);
            }

            QString mountPath = d->mountDir.path() + "/" + mount.first;
            QString block = mountDrive->interface<BlockInterface>()->blockName();

            QDir::root().mkpath(mountPath);

            // Mount the drive
            QTextStream(stderr) << tr("Mounting:") << " " << block << " -> " << mountPath << "\n";

            QProcess* proc = new QProcess();
            proc->setProcessChannelMode(QProcess::ForwardedChannels);
            proc->start("mount", {block, mountPath});
            connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitCode == 0) {
                    emit mountNext();
                } else {
                    qDebug() << "Error";
                    emit failure();
                }
                proc->deleteLater();
            });
        });
        this->addState(state);

        if (previousState) {
            previousState->addTransition(this, &MountState::mountNext, state);
        } else {
            this->setInitialState(state);
        }

        previousState = state;
    }

    QFinalState* finalState = new QFinalState();
    this->addState(finalState);
    if (previousState) {
        previousState->addTransition(this, &MountState::mountNext, finalState);
    } else {
        this->setInitialState(finalState);
    }

    QStateMachine::onEntry(event);
}
