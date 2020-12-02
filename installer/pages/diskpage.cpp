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
#include "diskpage.h"
#include "ui_diskpage.h"

#include <tlogger.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/blockinterface.h>
#include <DriveObjects/driveinterface.h>
#include <DriveObjects/loopinterface.h>
#include <DriveObjects/partitioninterface.h>
#include <DriveObjects/partitiontableinterface.h>
#include <driveobjectmanager.h>
#include <tpopover.h>
#include <QMessageBox>
#include "flowcontroller.h"
#include "installerdata.h"
#include "advanceddiskpopover.h"

DiskPage::DiskPage(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::DiskPage) {
    ui->setupUi(this);

    ui->titleLabel->setBackButtonShown(true);
    ui->descriptionLabel->setText(tr("Select a location to install %1 to.").arg(InstallerData::systemName()));

    reloadDisks();
}

DiskPage::~DiskPage() {
    delete ui;
}

void DiskPage::on_nextButton_clicked() {
    QJsonObject diskInfo = InstallerData::value("disk").toObject();
    if (diskInfo.value("type").toString() == QStringLiteral("whole-disk")) {
        DiskObject* disk = DriveObjectManager::diskByBlockName(diskInfo.value("block").toString());

        bool allowDisk = true;
        if (disk->interface<BlockInterface>()->drive() && disk->interface<BlockInterface>()->drive()->isOpticalDrive()) allowDisk = false;
        if (disk->interface<BlockInterface>()->drive() && disk->interface<BlockInterface>()->drive()->isRemovable()) allowDisk = false;
        if (disk->interface<LoopInterface>()) allowDisk = false;

        if (!allowDisk) {
            //Warn the user
            if (QMessageBox::warning(this, tr("Nonstandard Disk"), tr("You are installing %1 to a nonstandard disk. Installation is likely to fail. Do you wish to attempt to install to this disk anyway?").arg(InstallerData::systemName()), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) return;
        }
    }

    FlowController::instance()->nextPage();
}

void DiskPage::on_titleLabel_backButtonClicked() {
    FlowController::instance()->previousPage();
}

void DiskPage::on_listWidget_currentRowChanged(int currentRow) {
    QListWidgetItem* item = ui->listWidget->item(currentRow);

    if (item) {
        InstallerData::insert("disk", QJsonObject({
            {"type", QStringLiteral("whole-disk")},
            {"block", item->data(Qt::UserRole).toString()}
        }));
        ui->nextButton->setEnabled(true);
    } else {
        ui->nextButton->setEnabled(false);
    }
}

void DiskPage::on_showAllDrivesBox_toggled(bool checked) {
    reloadDisks();
}

void DiskPage::reloadDisks() {
    ui->listWidget->clear();
    for (DiskObject* disk : DriveObjectManager::rootDisks()) {
        if (!ui->showAllDrivesBox->isChecked()) {
            if (disk->interface<BlockInterface>()->drive() && disk->interface<BlockInterface>()->drive()->isOpticalDrive()) continue;
            if (disk->interface<BlockInterface>()->drive() && disk->interface<BlockInterface>()->drive()->isRemovable()) continue;
            if (disk->interface<LoopInterface>()) continue;
        }

        QListWidgetItem* item = new QListWidgetItem();
        item->setText(QStringLiteral("%1 (%2) · %3").arg(disk->displayName()).arg(disk->interface<BlockInterface>()->blockName()).arg(QLocale().formattedDataSize(disk->interface<BlockInterface>()->size())));
        item->setData(Qt::UserRole, disk->interface<BlockInterface>()->blockName());
        item->setIcon(disk->icon());
        ui->listWidget->addItem(item);
    }
}

void DiskPage::on_advancedButton_clicked() {
    //Open advanced disk pane
    AdvancedDiskPopover* advancedDisks = new AdvancedDiskPopover();
    tPopover* popover = new tPopover(advancedDisks);
    popover->setPopoverWidth(-1);
    popover->setPopoverSide(tPopover::Bottom);
    connect(advancedDisks, &AdvancedDiskPopover::rejected, popover, &tPopover::dismiss);
    connect(advancedDisks, &AdvancedDiskPopover::accepted, popover, [ = ](QJsonObject diskInformation) {
        popover->dismiss();
        InstallerData::insert("disk", diskInformation);
        on_nextButton_clicked();
    });
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, advancedDisks, &AdvancedDiskPopover::deleteLater);
    popover->show(this->window());
}
