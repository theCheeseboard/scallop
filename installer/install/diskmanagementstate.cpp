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
#include "diskmanagementstate.h"

#include "installerdata.h"
#include <DriveObjects/blockinterface.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/encryptedinterface.h>
#include <DriveObjects/filesysteminterface.h>
#include <DriveObjects/partitioninterface.h>
#include <DriveObjects/partitiontableinterface.h>
#include <QDebug>
#include <QFinalState>
#include <driveobjectmanager.h>
#include <frisbeeexception.h>

struct DiskManagementStatePrivate {
        QJsonObject diskDetails;
        QMap<QString, DiskObject*> disks;
        QList<QPair<QString, QString>> mounts;
};

DiskManagementState::DiskManagementState(QState* parent) :
    QStateMachine(parent) {
    d = new DiskManagementStatePrivate();

    // Determine what to do
    d->diskDetails = InstallerData::value("disk").toObject();
    QString diskType = InstallerData::value("diskType").toString();

    // TODO: Disk encryption?

    QFinalState* finishedState = new QFinalState();
    connect(finishedState, &QFinalState::entered, this, [=] {
        InstallerData::insertTemp("mounts", QVariant::fromValue(d->mounts));
    });

    // Determine what to do
    if (diskType == QStringLiteral("whole-disk")) {
        // Erase the disk
        QState* partitionTableState = new QState();
        connect(partitionTableState, &QState::entered, this, [=]() -> QCoro::Task<> {
            QTextStream(stdout) << tr("Erasing disk %1").arg(d->diskDetails.value("block").toString()) << "\n";
            DiskObject* disk = DriveObjectManager::diskByBlockName(InstallerData::value("disk").toObject().value("block").toString());
            try {
                co_await disk->interface<BlockInterface>()->format("gpt", {});
                d->disks.insert("disk", disk);
                emit nextState();

            } catch (FrisbeeException& ex) {
                QTextStream(stderr) << tr("Failed to erase the disk") << "\n";
                QTextStream(stderr) << ex.response() << "\n";
                emit failure();
            }
        });

        QState* bootPartition = new QState();
        connect(bootPartition, &QState::entered, this, [=]() -> QCoro::Task<> {
            QTextStream(stdout) << tr("Creating Boot Partition") << "\n";

            DiskObject* disk = d->disks.value("disk");
            PartitionTableInterface* partitionTable = disk->interface<PartitionTableInterface>();

            if (InstallerData::isEfi()) {
                // Create an EFI System Partition
                try {
                    auto object = co_await partitionTable->createPartitionAndFormat(0, 536870912 /* 512 MB */, "C12A7328-F81F-11D2-BA4B-00A0C93EC93B" /* EFI System */, "efi", {}, "vfat", {});
                    DiskObject* bootObject = DriveObjectManager::diskForPath(object);
                    d->disks.insert("firmwareboot", bootObject);
                    d->mounts.append({"/boot", object.path()});

                    emit nextState();
                } catch (FrisbeeException& ex) {
                    emit failure();

                    QTextStream(stderr) << tr("Failed to create the boot partition") << "\n";
                    QTextStream(stderr) << ex.response() << "\n";
                }
            } else {
                // Create a BIOS Boot partition
                try {
                    auto object = co_await partitionTable->createPartition(0, 1048576 /* 1 MB */, "21686148-6449-6E6F-744E-656564454649" /* BIOS Boot */, "biosboot", {});
                    DiskObject* bootObject = DriveObjectManager::diskForPath(object);
                    d->disks.insert("firmwareboot", bootObject);

                    emit nextState();
                } catch (FrisbeeException& ex) {
                    emit failure();

                    QTextStream(stderr) << tr("Failed to create the boot partition") << "\n";
                    QTextStream(stderr) << ex.response() << "\n";
                }
            }
        });

        QState* rootPartition = new QState();
        connect(rootPartition, &QState::entered, this, [=]() -> QCoro::Task<> {
            DiskObject* disk = d->disks.value("disk");
            PartitionTableInterface* partitionTable = disk->interface<PartitionTableInterface>();
            DiskObject* bootObject = d->disks.value("firmwareboot");
            quint64 newOffset = bootObject->interface<PartitionInterface>()->offset() + bootObject->interface<PartitionInterface>()->size();

            QVariantMap formatOptions;
            QJsonObject luksConfig = InstallerData::value("luks").toObject();
            if (luksConfig.contains("password")) {
                // Encrypt this partition
                formatOptions.insert("encrypt.passphrase", luksConfig.value("password").toString());
                formatOptions.insert("encrypt.type", "luks1");
            }

            QTextStream(stdout) << tr("Creating Root Partition") << "\n";
            try {
                auto object = co_await partitionTable->createPartitionAndFormat(newOffset, 0, "4F68BCE3-E8CD-4DB1-96E7-FBCAF984B709" /* Linux Root */, "root", {}, "ext4", formatOptions);
                DiskObject* rootObject = DriveObjectManager::diskForPath(object);
                EncryptedInterface* encrypted = rootObject->interface<EncryptedInterface>();
                if (encrypted) {
                    rootObject = encrypted->cleartextDevice();
                }
                d->mounts.append({"/", rootObject->path().path()});
                d->disks.insert("root", rootObject);

                emit nextState();
            } catch (FrisbeeException& ex) {
                emit failure();

                QTextStream(stderr) << tr("Failed to create the root partition") << "\n";
                QTextStream(stderr) << ex.response() << "\n";
            }
        });

        this->addState(partitionTableState);
        this->addState(bootPartition);
        this->addState(rootPartition);
        this->addState(finishedState);

        this->setInitialState(partitionTableState);
        partitionTableState->addTransition(this, &DiskManagementState::nextState, bootPartition);
        bootPartition->addTransition(this, &DiskManagementState::nextState, rootPartition);
        rootPartition->addTransition(this, &DiskManagementState::nextState, finishedState);
    } else if (diskType == QStringLiteral("probe-replace-block") || diskType == QStringLiteral("probe-resize-block")) {
        // Replace a partition

        QString rootBlockName = InstallerData::value("probeBlock").toString();
        QString bootBlockName = InstallerData::value("probeEsp").toString();

        DiskObject* oldBlock = DriveObjectManager::diskByBlockName(rootBlockName);
        PartitionInterface* oldPartition = oldBlock->interface<PartitionInterface>();
        d->disks.insert("disk", oldPartition->parentTable());

        // Erase the disk
        QState* editOldPartitionState = new QState();
        if (diskType == QStringLiteral("probe-replace-block")) {
            quint64 offset = oldPartition->offset();
            InstallerData::insertTemp("newBlocksOffset", offset);
            QTextStream(stderr) << tr("Replacing partition %1 with boot block %2; first partition at offset %3").arg(rootBlockName, bootBlockName).arg(offset) << "\n";

            connect(editOldPartitionState, &QState::entered, this, [=]() -> QCoro::Task<> {
                QTextStream(stdout) << tr("Erasing partition %1").arg(oldBlock->interface<BlockInterface>()->blockName()) << "\n";
                try {
                    co_await oldPartition->deletePartition();
                    emit nextState();
                } catch (FrisbeeException& ex) {
                    QTextStream(stderr) << tr("Failed to erase the partition") << "\n";
                    QTextStream(stderr) << ex.response() << "\n";
                    emit failure();
                }
            });
        } else {
            quint64 newSize = InstallerData::value("newSize").toString().toULongLong();
            QTextStream(stderr) << tr("Resizing partition %1 with boot block %2; partition will be %3 long").arg(rootBlockName, bootBlockName).arg(newSize) << "\n";

            connect(editOldPartitionState, &QState::entered, this, [=]() -> QCoro::Task<> {
                QTextStream(stdout) << tr("Resizing partition %1").arg(oldBlock->interface<BlockInterface>()->blockName()) << "\n";

                try {
                    co_await oldBlock->interface<FilesystemInterface>()->resize(newSize);
                    co_await oldPartition->resize(newSize);
                    quint64 newOffset = oldPartition->offset() + oldBlock->interface<BlockInterface>()->size();
                    InstallerData::insertTemp("newBlocksOffset", newOffset);
                    QTextStream(stderr) << tr("New partitions will be located at %1").arg(newOffset) << "\n";
                    emit nextState();
                } catch (FrisbeeException& ex) {
                    QTextStream(stderr) << tr("Failed to erase the partition") << "\n";
                    QTextStream(stderr) << ex.response() << "\n";
                    emit failure();
                }
            });
        }

        QState* bootPartition = new QState();
        connect(bootPartition, &QState::entered, this, [=]() -> QCoro::Task<> {
            QTextStream(stdout) << tr("Creating Boot Partition") << "\n";

            DiskObject* disk = d->disks.value("disk");
            PartitionTableInterface* partitionTable = disk->interface<PartitionTableInterface>();

            quint64 offset = InstallerData::valueTemp("newBlocksOffset").toULongLong();

            if (InstallerData::isEfi()) {
                // Create an XBOOTLDR Partition
                try {
                    auto object = co_await partitionTable->createPartitionAndFormat(offset, 536870912 /* 512 MB */, "BC13C2FF-59E6-4262-A352-B275FD6F7172" /* XBOOTLDR */, "xbl", {}, "vfat", {});
                    DiskObject* bootObject = DriveObjectManager::diskForPath(object);
                    d->disks.insert("firmwareboot", bootObject);
                    d->mounts.append({"/boot", object.path()});

                    emit nextState();
                } catch (FrisbeeException& ex) {
                    emit failure();

                    QTextStream(stderr) << tr("Failed to create the boot partition") << "\n";
                    QTextStream(stderr) << ex.response() << "\n";
                }
            } else {
                // Create a BIOS Boot partition if required
                if (partitionTable->type() == "dos") {
                    emit nextState();
                } else {
                    try {
                        auto object = co_await partitionTable->createPartition(offset, 1048576 /* 1 MB */, "21686148-6449-6E6F-744E-656564454649" /* BIOS Boot */, "biosboot", {});
                        DiskObject* bootObject = DriveObjectManager::diskForPath(object);
                        d->disks.insert("firmwareboot", bootObject);

                        emit nextState();
                    } catch (FrisbeeException& ex) {
                        emit failure();

                        QTextStream(stderr) << tr("Failed to create the boot partition") << "\n";
                        QTextStream(stderr) << ex.response() << "\n";
                    }
                }
            }
        });

        QState* rootPartition = new QState();
        connect(rootPartition, &QState::entered, this, [=]() -> QCoro::Task<> {
            DiskObject* disk = d->disks.value("disk");
            PartitionTableInterface* partitionTable = disk->interface<PartitionTableInterface>();
            DiskObject* bootObject = d->disks.value("firmwareboot");
            quint64 offset = InstallerData::valueTemp("newBlocksOffset").toULongLong();
            quint64 newOffset = offset + (bootObject ? bootObject->interface<PartitionInterface>()->size() : 0);

            QVariantMap formatOptions;
            QJsonObject luksConfig = InstallerData::value("luks").toObject();
            if (luksConfig.contains("password")) {
                // Encrypt this partition
                formatOptions.insert("encrypt.passphrase", luksConfig.value("password").toString());
                formatOptions.insert("encrypt.type", "luks1");
            }

            QTextStream(stdout) << tr("Creating Root Partition") << "\n";

            QString type;
            QString name;

            if (partitionTable->type() == "dos") {
                type = "0x83";
            } else {
                type = "4F68BCE3-E8CD-4DB1-96E7-FBCAF984B709"; /* Linux Root */
                name = "root";
            }
            try {
                auto object = co_await partitionTable->createPartitionAndFormat(newOffset, 0, type, name, {}, "ext4", formatOptions);
                DiskObject* rootObject = DriveObjectManager::diskForPath(object);
                EncryptedInterface* encrypted = rootObject->interface<EncryptedInterface>();
                if (encrypted) {
                    rootObject = encrypted->cleartextDevice();
                }
                d->mounts.append({"/", rootObject->path().path()});
                d->disks.insert("root", rootObject);

                emit nextState();
            } catch (FrisbeeException& ex) {
                emit failure();

                QTextStream(stderr) << tr("Failed to create the root partition") << "\n";
                QTextStream(stderr) << ex.response() << "\n";
            }
        });

        if (InstallerData::isEfi()) {
            d->mounts.append({"/efi", DriveObjectManager::diskByBlockName(bootBlockName)->path().path()});
        }

        this->addState(editOldPartitionState);
        this->addState(bootPartition);
        this->addState(rootPartition);
        this->addState(finishedState);

        this->setInitialState(editOldPartitionState);
        editOldPartitionState->addTransition(this, &DiskManagementState::nextState, bootPartition);
        bootPartition->addTransition(this, &DiskManagementState::nextState, rootPartition);
        rootPartition->addTransition(this, &DiskManagementState::nextState, finishedState);

    } else if (diskType == QStringLiteral("mount-list")) {
        // Figure out the mounts
        QJsonArray mounts = d->diskDetails.value("mounts").toArray();
        for (QJsonValue mount : mounts) {
            QJsonObject mountObject = mount.toObject();
            d->mounts.append({mountObject.value("mountPoint").toString(), mountObject.value("block").toString()});
        }

        this->addState(finishedState);
        this->setInitialState(finishedState);
    }
}

DiskManagementState::~DiskManagementState() {
    delete d;
}
