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
#include "mountpointpopover.h"
#include "ui_mountpointpopover.h"

#include <DriveObjects/diskobject.h>
#include <terrorflash.h>
#include <QMenu>

#define ADD_MOUNT_MENU_ITEM(mount) menu->addAction(mount, [=] { \
        ui->mountPointEdit->setText(mount); \
    });

MountPointPopover::MountPointPopover(DiskObject* disk, QString currentMountPoint, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::MountPointPopover) {
    ui->setupUi(this);

    ui->titleLabel->setText(disk->displayName());
    ui->mountPointEdit->setText(currentMountPoint);
    ui->titleLabel->setBackButtonShown(true);
    ui->mountPointWidget->setFixedWidth(SC_DPI(600));
    ui->okButton->setFixedWidth(SC_DPI(600));

    QMenu* menu = new QMenu();
    menu->addSection(tr("Common Mount Points"));
    ADD_MOUNT_MENU_ITEM("/")
    ADD_MOUNT_MENU_ITEM("/boot")
    ADD_MOUNT_MENU_ITEM("/efi")
    ADD_MOUNT_MENU_ITEM("/home")
    ADD_MOUNT_MENU_ITEM("/var")
    ADD_MOUNT_MENU_ITEM("/etc")
    ADD_MOUNT_MENU_ITEM("/usr")
    ui->mountPointsButton->setMenu(menu);
}

MountPointPopover::~MountPointPopover() {
    delete ui;
}

void MountPointPopover::on_titleLabel_backButtonClicked() {
    emit rejected();
}

void MountPointPopover::on_okButton_clicked() {
    if (!ui->mountPointEdit->text().startsWith("/") && !ui->mountPointEdit->text().isEmpty()) {
        tErrorFlash::flashError(ui->mountPointEdit);
        return;
    }

    emit accepted(ui->mountPointEdit->text());
}
