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
#include "readypage.h"
#include "ui_readypage.h"

#include "flowcontroller.h"
#include "installerdata.h"
#include "installipcmanager.h"

#include <QJsonValue>
#include <QJsonObject>
#include <QTimer>

#include "popovers/eraseconfirmpopover.h"
#include <tpopover.h>

ReadyPage::ReadyPage(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ReadyPage) {
    ui->setupUi(this);

    ui->titleLabel->setBackButtonShown(true);
    ui->nextButton->setText(tr("Install %1").arg(InstallerData::systemName()));
}

ReadyPage::~ReadyPage() {
    delete ui;
}

void ReadyPage::on_titleLabel_backButtonClicked() {
    FlowController::instance()->previousPage();
}

void ReadyPage::on_nextButton_clicked() {
    if (InstallerData::value("disk").toObject().value("type").toString() == QStringLiteral("whole-disk")) {
        //Perform final confirmations
        EraseConfirmPopover* jp = new EraseConfirmPopover();
        tPopover* popover = new tPopover(jp);
        popover->setPopoverWidth(SC_DPI(-200));
        popover->setPopoverSide(tPopover::Bottom);
        connect(jp, &EraseConfirmPopover::rejected, popover, &tPopover::dismiss);
        connect(jp, &EraseConfirmPopover::accepted, popover, [ = ] {
            doInstall();
            popover->dismiss();
        });
        connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
        connect(popover, &tPopover::dismissed, jp, &EraseConfirmPopover::deleteLater);
        popover->show(this->window());
        return;
    }

    doInstall();
}

void ReadyPage::doInstall() {
    InstallerData::insert("language", QLocale().name());
    FlowController::instance()->nextPage();
    QTimer::singleShot(1000, [ = ] {
        InstallIpcManager::startInstalling();
    });
}
