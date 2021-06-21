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
#include "finalresetpopover.h"
#include "ui_finalresetpopover.h"

FinalResetPopover::FinalResetPopover(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::FinalResetPopover) {
    ui->setupUi(this);

    ui->titleLabel->setText(tr("Reset this device"));
    ui->titleLabel->setBackButtonShown(true);
    ui->doResetButton->setProperty("type", "destructive");
    ui->eraseConfirmWidget->setFixedWidth(SC_DPI(600));
}

FinalResetPopover::~FinalResetPopover() {
    delete ui;
}

void FinalResetPopover::on_titleLabel_backButtonClicked() {
    emit rejected();
}

void FinalResetPopover::on_doResetButton_clicked() {
    emit accepted();
}

