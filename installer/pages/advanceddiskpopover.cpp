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
#include "advanceddiskpopover.h"
#include "ui_advanceddiskpopover.h"

#include <tpopover.h>
#include <QProcess>
#include "installerdata.h"
#include "diskmodel.h"
#include "popovers/mountpointpopover.h"

#include <driveobjectmanager.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/blockinterface.h>
#include <DriveObjects/driveinterface.h>
#include <DriveObjects/loopinterface.h>

struct AdvancedDiskPopoverPrivate {
    DiskModel* disks;

    QProcess frisbeeProcess;
    QStringList requiredPartitions;
};

AdvancedDiskPopover::AdvancedDiskPopover(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::AdvancedDiskPopover) {
    ui->setupUi(this);

    d = new AdvancedDiskPopoverPrivate();
    ui->titleLabel->setBackButtonShown(true);

    d->requiredPartitions.append("/");
    if (InstallerData::isEfi()) d->requiredPartitions.append("/boot");

    ui->partitionRequirementsLabel->setText(tr("To continue, you'll need to set at least a partition for %1.").arg(QLocale().createSeparatedList(d->requiredPartitions)));

    d->disks = new DiskModel();
    ui->treeView->setModel(d->disks);
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->treeView->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    connect(d->disks, &DiskModel::mountPointsChanged, this, &AdvancedDiskPopover::updateState);

    updateState();

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Fade);
    connect(&d->frisbeeProcess, &QProcess::started, this, [ = ] {
        ui->stackedWidget->setCurrentWidget(ui->editPartitonsPage);
    });
    connect(&d->frisbeeProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [ = ] {
        ui->stackedWidget->setCurrentWidget(ui->mountPointsPage);
    });

    if (InstallerData::isEfi()) {
        ui->bootloaderWidget->setVisible(false);
    } else {
        for (DiskObject* disk : DriveObjectManager::rootDisks()) {
            if (disk->interface<BlockInterface>()->drive() && disk->interface<BlockInterface>()->drive()->isOpticalDrive()) continue;
            if (disk->interface<BlockInterface>()->drive() && disk->interface<BlockInterface>()->drive()->isRemovable()) continue;
            if (disk->interface<LoopInterface>()) continue;

            ui->bootloaderBox->addItem(disk->displayName(), disk->interface<BlockInterface>()->blockName());
        }
    }
}

AdvancedDiskPopover::~AdvancedDiskPopover() {
    delete ui;
    delete d;
}

void AdvancedDiskPopover::on_titleLabel_backButtonClicked() {
    emit rejected();
}

void AdvancedDiskPopover::on_editPartitionsButton_clicked() {
    d->frisbeeProcess.start("thefrisbee", QStringList());
}

void AdvancedDiskPopover::on_treeView_activated(const QModelIndex& index) {
    DiskObject* disk = static_cast<DiskObject*>(index.internalPointer());
    if (!disk->isInterfaceAvailable(DiskInterface::Filesystem)) return;

    //Open advanced disk pane
    MountPointPopover* advancedDisks = new MountPointPopover(disk, d->disks->mountPoints().value(disk));
    tPopover* popover = new tPopover(advancedDisks);
    popover->setPopoverWidth(-SC_DPI(300));
    popover->setPopoverSide(tPopover::Bottom);
    connect(advancedDisks, &MountPointPopover::rejected, popover, &tPopover::dismiss);
    connect(advancedDisks, &MountPointPopover::accepted, popover, [ = ](QString mountPoint) {
        d->disks->setMountPoint(index.row(), index.parent(), mountPoint);
        popover->dismiss();
    });
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, advancedDisks, &MountPointPopover::deleteLater);
    popover->show(this);
}

void AdvancedDiskPopover::updateState() {
    bool success = true;
    QStringList usedMountPoints = d->disks->mountPoints().values();
    if (usedMountPoints.removeDuplicates() > 0) success = false;

    for (QString mountPoint : d->requiredPartitions) {
        if (!usedMountPoints.contains(mountPoint)) success = false;
    }

    ui->acceptButton->setEnabled(success);
}

void AdvancedDiskPopover::on_acceptButton_clicked() {
    QMap<DiskObject*, QString> selectedMounts = d->disks->mountPoints();

    QJsonObject diskInformation;
    diskInformation.insert("type", QStringLiteral("mount-list"));

    QJsonArray mounts;
    for (DiskObject* disk : selectedMounts.keys()) {
        mounts.append(QJsonObject({
            {"block", disk->interface<BlockInterface>()->blockName()},
            {"mountPoint", selectedMounts.value(disk)}
        }));
    }
    diskInformation.insert("mounts", mounts);

    if (ui->bootloaderBox->isVisible() && ui->bootloaderCheckbox->isChecked()) {
        //Add bootloader information
        diskInformation.insert("bootloaderDestination", ui->bootloaderBox->currentData().toString());
    }

    emit accepted(diskInformation);
}

void AdvancedDiskPopover::on_bootloaderCheckbox_toggled(bool checked) {
    ui->bootloaderBox->setEnabled(checked);
}
