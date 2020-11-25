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
#include "finishedpage.h"
#include "ui_finishedpage.h"

#include "installerdata.h"
#include "installipcmanager.h"

FinishedPage::FinishedPage(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::FinishedPage) {
    ui->setupUi(this);

    ui->failureDescription->setText(tr("Sorry, we couldn't install %1.").arg(InstallerData::systemName()));

    connect(InstallIpcManager::instance(), &InstallIpcManager::success, this, [ = ] {
        ui->stackedWidget->setCurrentWidget(ui->successPage);
    });
    connect(InstallIpcManager::instance(), &InstallIpcManager::failure, this, [ = ] {
        ui->stackedWidget->setCurrentWidget(ui->failurePage);
    });
}

FinishedPage::~FinishedPage() {
    delete ui;
}

void FinishedPage::on_exitButton_clicked() {
    QApplication::exit();
}

void FinishedPage::on_startOverButton_clicked() {

}

void FinishedPage::on_rebootButton_2_clicked() {

}

void FinishedPage::on_rebootButton_clicked() {

}

void FinishedPage::on_powerOffButton_clicked() {

}
