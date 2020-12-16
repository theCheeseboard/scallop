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
#include "encryptpage.h"
#include "ui_encryptpage.h"

#include "flowcontroller.h"
#include "popovers/encryptpopover.h"
#include "installerdata.h"
#include <tpopover.h>
#include <QJsonValue>
#include <QJsonObject>

EncryptPage::EncryptPage(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::EncryptPage) {
    ui->setupUi(this);

    ui->titleLabel->setBackButtonShown(true);
    ui->encryptButton->setProperty("type", "positive");

    ui->protectionFrame->setTitle(tr("Your data will be protected"));
    ui->protectionFrame->setText(tr("The installation disk will not be accessible by anyone without the password."));
    ui->protectionFrame->setState(tStatusFrame::Good);

    ui->performanceFrame->setTitle(tr("Performance may be affected"));
    ui->performanceFrame->setText(tr("Your disk may be ever so slightly slower while disk encryption is enabled."));
    ui->performanceFrame->setState(tStatusFrame::Warning);

    ui->enablementFrame->setTitle(tr("It's now or never"));
    ui->enablementFrame->setText(tr("Once disk encryption is enabled, it can't be disabled. It also can't be enabled later, unless you reinstall %1.").arg(InstallerData::systemName()));
    ui->enablementFrame->setState(tStatusFrame::Error);

    ui->requirementsFrame->setTitle(tr("Remember the password"));
    ui->requirementsFrame->setText(tr("It'll be required each time this device is powered on."));
    ui->requirementsFrame->setState(tStatusFrame::NoState);

    FlowController::instance()->setSkipPage(this, [ = ] {
        if (!InstallerData::isEfi()) return true; //Don't support encryption on non EFI systems for now
        return InstallerData::value("disk").toObject().value("type").toString() != "whole-disk";
    });
}

EncryptPage::~EncryptPage() {
    delete ui;
}

void EncryptPage::on_titleLabel_backButtonClicked() {
    FlowController::instance()->previousPage();
}

void EncryptPage::on_encryptButton_clicked() {
    //Perform final confirmations
    EncryptPopover* jp = new EncryptPopover();
    tPopover* popover = new tPopover(jp);
    popover->setPopoverWidth(SC_DPI(-200));
    popover->setPopoverSide(tPopover::Bottom);
    connect(jp, &EncryptPopover::rejected, popover, &tPopover::dismiss);
    connect(jp, &EncryptPopover::accepted, popover, [ = ] {
        popover->dismiss();
        FlowController::instance()->nextPage();
    });
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, jp, &EncryptPopover::deleteLater);
    popover->show(this->window());
}

void EncryptPage::on_noEncryptButton_clicked() {

    InstallerData::remove("luks");
    FlowController::instance()->nextPage();
}
