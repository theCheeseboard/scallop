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
#include "probemanager.h"

#include <QTextStream>
#include <QTemporaryDir>
#include <QProcess>
#include <driveobjectmanager.h>

#include <DriveObjects/diskobject.h>
#include <DriveObjects/partitiontableinterface.h>
#include <DriveObjects/partitioninterface.h>
#include <DriveObjects/blockinterface.h>
#include <DriveObjects/filesysteminterface.h>

const char* EFI_SYSTEM_PARTITION = "c12a7328-f81f-11d2-ba4b-00a0c93ec93b";

ProbeManager::ProbeManager(QObject* parent) : QObject(parent) {

}

void ProbeManager::probe(QString disk) {
    QTextStream output(stdout);
    QTextStream errOutput(stderr);

    QTemporaryDir mountDir;
    if (!mountDir.isValid()) {
        errOutput << tr("Could not create temporary directory").arg(disk) << "\n";
        return;
    }


    DiskObject* rootDisk = DriveObjectManager::diskByBlockName(disk);
    if (!rootDisk) {
        errOutput << tr("%1 is not a disk").arg(disk) << "\n";
        return;
    }

    //Try to find the ESP
    if (!rootDisk->isInterfaceAvailable(DiskInterface::PartitionTable)) {
        errOutput << tr("%1 is not a partition table").arg(disk) << "\n";
        return;
    }

    PartitionTableInterface* partitionTable = rootDisk->interface<PartitionTableInterface>();
    for (DiskObject* partition : partitionTable->partitions()) {
        if (!partition->isInterfaceAvailable(DiskInterface::Filesystem)) continue;

        BlockInterface* block = partition->interface<BlockInterface>();
        PartitionInterface* p = partition->interface<PartitionInterface>();
        if (p->type() == EFI_SYSTEM_PARTITION) {
            //We found a valid ESP!
            output << QStringLiteral("ESP;%1\n").arg(block->blockName());
        } else {
            FilesystemInterface* fs = partition->interface<FilesystemInterface>();

            QProcess mountProc;
            mountProc.start("mount", {block->blockName(), mountDir.path()});
            mountProc.waitForFinished();

            if (mountProc.exitCode() != 0) {
                errOutput << tr("Could not mount %1").arg(block->blockName()) << "\n";
                continue;
            }

            if (QFileInfo(mountDir.filePath("Windows/System32")).isDir()) {
                //We found Windows!
                output << QStringLiteral("OS;Windows;%1\n").arg(block->blockName());
            } else if (QFileInfo(mountDir.filePath("etc/os-release")).isFile()) {
                //We found a Linux of some sort!
            }

            QProcess umountProc;
            umountProc.start("umount", {mountDir.path()});
            umountProc.waitForFinished();
        }
    }
}
