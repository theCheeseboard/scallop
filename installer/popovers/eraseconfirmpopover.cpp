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
#include "eraseconfirmpopover.h"
#include "ui_eraseconfirmpopover.h"

#include "installerdata.h"

EraseConfirmPopover::EraseConfirmPopover(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::EraseConfirmPopover) {
    ui->setupUi(this);

    ui->titleLabel->setText(tr("Erase Disk and Install %1").arg(InstallerData::systemName()));
    ui->titleLabel->setBackButtonShown(true);
    ui->doEraseButton->setProperty("type", "destructive");
    ui->eraseConfirmWidget->setFixedWidth(SC_DPI(600));
}

EraseConfirmPopover::~EraseConfirmPopover() {
    delete ui;
}

void EraseConfirmPopover::on_titleLabel_backButtonClicked() {
    emit rejected();
}

void EraseConfirmPopover::on_doEraseButton_clicked() {
    emit accepted();
}
