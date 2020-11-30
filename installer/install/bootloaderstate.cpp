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
#include "bootloaderstate.h"

#include <QTextStream>
#include <QProcess>
#include <QFinalState>
#include <QJsonObject>
#include "installerdata.h"

BootloaderState::BootloaderState(QState* parent) : QStateMachine(parent) {
    QFinalState* finalState = new QFinalState();
    this->addState(finalState);

    QState* configState = new QState();
    connect(configState, &QState::entered, this, [ = ] {
        QString systemRoot = InstallerData::valueTemp("systemRoot").toString();
        QList<QPair<QString, QString>> mounts = InstallerData::valueTemp("mounts").value<QList<QPair<QString, QString>>>();

        //Generate GRUB Config
        QTextStream(stdout) << tr("Configuring Bootloader...") << "\n";
        QProcess* proc = new QProcess();
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [ = ](int exitCode, QProcess::ExitStatus exitStatus) {
            if (exitCode == 0) {
                emit nextState();
            } else {
                QTextStream(stderr) << tr("Failed to create the GRUB Configuration File") << "\n";
                emit failure();
            }
            proc->deleteLater();
        });
        proc->start("arch-chroot", {systemRoot, "grub-mkconfig", "-o", "/boot/grub/grub.cfg"});
    });
    this->addState(configState);
    configState->addTransition(this, &BootloaderState::nextState, finalState);

    if (InstallerData::isEfi()) {
        QState* installEfiState = new QState();
        connect(installEfiState, &QState::entered, this, [ = ] {
            QString systemRoot = InstallerData::valueTemp("systemRoot").toString();
            QList<QPair<QString, QString>> mounts = InstallerData::valueTemp("mounts").value<QList<QPair<QString, QString>>>();

            //Install GRUB as EFI
            QTextStream(stdout) << tr("Installing the bootloader...") << "\n";

            QProcess* proc = new QProcess();
            connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [ = ](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitCode == 0) {
                    emit nextState();
                } else {
                    QTextStream(stderr) << tr("Failed to install GRUB") << "\n";
                    emit failure();
                }
            });
            proc->start("arch-chroot", {systemRoot, "grub-install", "--target=x86_64-efi", "--efi-directory=/boot/efi", "--bootloader-id=grub"});
        });

        this->addState(installEfiState);
        this->setInitialState(installEfiState);
        installEfiState->addTransition(this, &BootloaderState::nextState, configState);
    } else {
        QState* installEfiState = new QState();
        connect(installEfiState, &QState::entered, this, [ = ] {
            QString bootloaderDisk;
            QJsonObject diskDetails = InstallerData::value("disk").toObject();
            if (diskDetails.value("type").toString() == QStringLiteral("whole-disk")) {
                //Install on the disk we're installing the OS on
                bootloaderDisk = diskDetails.value("block").toString();
            }


            QString systemRoot = InstallerData::valueTemp("systemRoot").toString();
            QList<QPair<QString, QString>> mounts = InstallerData::valueTemp("mounts").value<QList<QPair<QString, QString>>>();

            //Install GRUB as BIOS
            QTextStream(stdout) << tr("Installing the bootloader...") << "\n";

            QProcess* proc = new QProcess();
            connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [ = ](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitCode == 0) {
                    emit nextState();
                } else {
                    QTextStream(stderr) << tr("Failed to install GRUB") << "\n";
                    emit failure();
                }
            });
            proc->start("arch-chroot", {systemRoot, "grub-install", "--target=i386-pc", bootloaderDisk});
        });

        this->addState(installEfiState);
        this->setInitialState(installEfiState);
        installEfiState->addTransition(this, &BootloaderState::nextState, configState);
    }
}
