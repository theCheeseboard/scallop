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
#include <QDir>
#include <QDirIterator>
#include "installerdata.h"

BootloaderState::BootloaderState(QState* parent) : QStateMachine(parent) {
    QFinalState* finalState = new QFinalState();
    this->addState(finalState);

    QState* configState = new QState();
    connect(configState, &QState::entered, this, [ = ] {
        QString systemRoot = InstallerData::valueTemp("systemRoot").toString();
        if (InstallerData::isEfi()) {
            QTextStream(stdout) << tr("Configuring Bootloader...") << "\n";

            //Read the machine ID
            QFile machineIdFile(QDir(systemRoot).absoluteFilePath("etc/machine-id"));
            machineIdFile.open(QFile::ReadOnly);
            QString machineId = machineIdFile.readAll();
            machineIdFile.close();

            //Copy over bootloader files
            QDir bootloaderFilesDir("/usr/share/scallop/install-system/systemd-boot-config");
            QDir destinationDir(QDir(systemRoot).absoluteFilePath("boot/loader"));
            QDirIterator bootloaderFiles(bootloaderFilesDir.path(), QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (bootloaderFiles.hasNext()) {
                bootloaderFiles.next();
                QString relativePath = bootloaderFilesDir.relativeFilePath(bootloaderFiles.filePath());
                QString destination = destinationDir.absoluteFilePath(relativePath);

                QTextStream(stderr) << bootloaderFiles.filePath() << " -> " << destinationDir.absoluteFilePath(relativePath) << "\n";
                if (QFile::exists(destination)) QFile::remove(destination);

                QFile input(bootloaderFiles.filePath());
                input.open(QFile::ReadOnly);
                QString contents = input.readAll();
                input.close();

                contents.replace("%{SYSTEM-NAME}", InstallerData::systemName()).replace("%{MACHINE-ID}", machineId);

                QFile output(destination);
                output.open(QFile::WriteOnly);
                output.write(contents.toUtf8());
                output.close();
            }

            nextState();
        } else {
            if (InstallerData::valueTemp("bootloaderInstalled").toBool()) {
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
            } else {
                //Skip bootloader config generation
                nextState();
            }
        }
    });
    this->addState(configState);
    configState->addTransition(this, &BootloaderState::nextState, finalState);

    QState* generateMachineIdState = new QState();
    connect(generateMachineIdState, &QState::entered, this, [ = ] {
        QString systemRoot = InstallerData::valueTemp("systemRoot").toString();

        QTextStream(stdout) << tr("Generating machine ID...") << "\n";
        QProcess* proc = new QProcess();
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [ = ](int exitCode, QProcess::ExitStatus exitStatus) {
            if (exitCode == 0) {
                emit nextState();
            } else {
                QTextStream(stderr) << tr("Failed to install systemd-boot") << "\n";
                emit failure();
            }
        });
        proc->start("arch-chroot", {systemRoot, "systemd-machine-id-setup"});
    });
    this->addState(generateMachineIdState);
    this->setInitialState(generateMachineIdState);

    if (InstallerData::isEfi()) {
        QState* installEfiState = new QState();
        connect(installEfiState, &QState::entered, this, [ = ] {
            QString systemRoot = InstallerData::valueTemp("systemRoot").toString();

            //Install sd-boot as EFI
            QTextStream(stdout) << tr("Installing the bootloader...") << "\n";

            QProcess* proc = new QProcess();
            connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [ = ](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitCode == 0) {
                    emit nextState();
                } else {
                    QTextStream(stderr) << tr("Failed to install systemd-boot") << "\n";
                    emit failure();
                }
            });
            proc->start("arch-chroot", {systemRoot, "bootctl", "install"});
        });

        InstallerData::insertTemp("bootloaderInstalled", true);

        this->addState(installEfiState);
        generateMachineIdState->addTransition(this, &BootloaderState::nextState, installEfiState);
        installEfiState->addTransition(this, &BootloaderState::nextState, configState);
    } else {
        QState* installMbrState = new QState();
        connect(installMbrState, &QState::entered, this, [ = ] {
            bool installBootloader = true;
            QString bootloaderDisk;
            QJsonObject diskDetails = InstallerData::value("disk").toObject();
            QString diskType = InstallerData::value("diskType").toString();
            if (diskType == QStringLiteral("whole-disk") || diskType == QStringLiteral("probe-replace-block") || diskType == QStringLiteral("probe-resize-block")) {
                //Install on the disk we're installing the OS on
                bootloaderDisk = diskDetails.value("block").toString();
            } else if (diskType == QStringLiteral("mount-list")) {
                //Install on the disk that was selected
                if (diskDetails.contains("bootloaderDestination")) {
                    bootloaderDisk = diskDetails.value("bootloaderDestination").toString();
                } else {
                    installBootloader = false;
                }
            }

            if (installBootloader) {
                QTextStream(stderr) << tr("Installing bootloader to") << " " << bootloaderDisk << "\n";

                QString systemRoot = InstallerData::valueTemp("systemRoot").toString();
                QList<QPair<QString, QString>> mounts = InstallerData::valueTemp("mounts").value<QList<QPair<QString, QString>>>();

                //Enable os-prober
                QFile grubConfig(QDir(systemRoot).absoluteFilePath("etc/default/grub"));
                grubConfig.open(QFile::WriteOnly | QFile::Append);
                grubConfig.write("\n\nGRUB_DISABLE_OS_PROBER=false");
                grubConfig.close();

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

                InstallerData::insertTemp("bootloaderInstalled", true);
            } else {
                QTextStream(stderr) << tr("Skipping bootloader installation as requested by user") << "\n";

                InstallerData::insertTemp("bootloaderInstalled", false);
                emit nextState();
            }
        });

        this->addState(installMbrState);
        generateMachineIdState->addTransition(this, &BootloaderState::nextState, installMbrState);
        installMbrState->addTransition(this, &BootloaderState::nextState, configState);
    }
}
