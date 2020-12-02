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
#include "diskmodel.h"

#include <driveobjectmanager.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/blockinterface.h>
#include <DriveObjects/partitiontableinterface.h>

struct DiskModelPrivate {
    QMap<DiskObject*, QModelIndex> indices;
    QMap<DiskObject*, QString> mountPoints;
};

DiskModel::DiskModel(QObject* parent)
    : QAbstractItemModel(parent) {
    d = new DiskModelPrivate();

    connect(DriveObjectManager::instance(), &DriveObjectManager::rootDisksChanged, this, &DiskModel::updateList);
}

DiskModel::~DiskModel() {
    delete d;
}

QModelIndex DiskModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) return QModelIndex();

    if (!parent.isValid()) {
        QModelIndex index = createIndex(row, column, DriveObjectManager::rootDisks().value(row));
        d->indices.insert(static_cast<DiskObject*>(index.internalPointer()), index);
        return index;
    } else {
        DiskObject* parentDisk = static_cast<DiskObject*>(parent.internalPointer());
        PartitionTableInterface* partitionTable = parentDisk->interface<PartitionTableInterface>();
        if (partitionTable) {
            QModelIndex index = createIndex(row, column, partitionTable->partitions().value(row));
            d->indices.insert(static_cast<DiskObject*>(index.internalPointer()), index);
            return index;
        }
    }

    return QModelIndex();
}

QModelIndex DiskModel::parent(const QModelIndex& index) const {
    for (int row = 0; row < rowCount(); row++) {
        QModelIndex checkingIndex = this->index(row, 0, QModelIndex());
        DiskObject* disk = static_cast<DiskObject*>(checkingIndex.internalPointer());
        PartitionTableInterface* partitionTable = disk->interface<PartitionTableInterface>();
        if (partitionTable) {
            if (partitionTable->partitions().contains(static_cast<DiskObject*>(index.internalPointer()))) {
                return checkingIndex;
            }
        }
    }
    return QModelIndex();
}

int DiskModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        DiskObject* parentDisk = static_cast<DiskObject*>(parent.internalPointer());
        PartitionTableInterface* partitionTable = parentDisk->interface<PartitionTableInterface>();
        if (partitionTable) {
            return partitionTable->partitions().count();
        } else {
            return 0;
        }
    }

    return DriveObjectManager::rootDisks().count();
}

int DiskModel::columnCount(const QModelIndex& parent) const {
    return 2;
}

QVariant DiskModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();

    DiskObject* object = static_cast<DiskObject*>(index.internalPointer());
    switch (role) {
        case Qt::DisplayRole:
            if (index.column() == 0) {
                return object->displayName();
            } else {
                return d->mountPoints.value(object);
            }
    }

    return QVariant();
}

void DiskModel::setMountPoint(int row, const QModelIndex& parent, QString mountPoint) {
    QModelIndex index = this->index(row, 0, parent);
    if (!index.isValid()) return;

    DiskObject* object = static_cast<DiskObject*>(index.internalPointer());
    if (mountPoint.isEmpty()) {
        d->mountPoints.remove(object);
    } else {
        d->mountPoints.insert(object, mountPoint);
    }

    emit dataChanged(index, index);
    emit mountPointsChanged();
}

QMap<DiskObject*, QString> DiskModel::mountPoints() const {
    return d->mountPoints;
}

void DiskModel::updateList() {
    emit beginResetModel();
    d->mountPoints.clear();
    emit endResetModel();
}

QVariant DiskModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return tr("Disk");
            case 1:
                return tr("Mount Point");
        }
    }
    return QVariant();
}
