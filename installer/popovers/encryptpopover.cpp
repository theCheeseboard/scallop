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
#include "encryptpopover.h"
#include "ui_encryptpopover.h"

#include "installerdata.h"
#include <QJsonObject>
#include <terrorflash.h>

EncryptPopover::EncryptPopover(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::EncryptPopover) {
    ui->setupUi(this);

    ui->titleLabel->setBackButtonShown(true);
    ui->mainWidget->setFixedWidth(SC_DPI(600));

    ui->warningFrame->setTitle(tr("Heads up!"));
    ui->warningFrame->setText(tr("It is imperative that you remember this password. You <b>WILL</b> lose your data if you forget it."));
    ui->warningFrame->setState(tStatusFrame::Error);

    this->setFocusProxy(ui->passwordBox);
}

EncryptPopover::~EncryptPopover() {
    delete ui;
}

void EncryptPopover::on_titleLabel_backButtonClicked() {
    emit rejected();
}

void EncryptPopover::on_acceptButton_clicked() {
    if (ui->passwordBox->text().isEmpty()) {
        tErrorFlash::flashError(ui->passwordBox);
        return;
    }

    if (ui->passwordBox->text() != ui->confirmPasswordBox->text()) {
        tErrorFlash::flashError(ui->confirmPasswordBox);
        return;
    }

    InstallerData::insert("luks", QJsonObject({
        {"password", ui->passwordBox->text()},
    }));

    emit accepted();
}
