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
#include "endsession.h"
#include "ui_endsession.h"

#include <tpopover.h>
#include <transparentdialog.h>
#include <statemanager.h>
#include <tscrim.h>

EndSession::EndSession(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::EndSession) {
    ui->setupUi(this);

    ui->centeredWidget->setFixedWidth(SC_DPI(600));
    ui->powerOffButton->setProperty("type", "destructive");
    ui->rebootButton->setProperty("type", "destructive");
    ui->titleLabel->setBackButtonShown(true);
    ui->exitButton->setVisible(false);
}

EndSession::~EndSession() {
    delete ui;
}

tPromise<void>* EndSession::showDialog() {
    return tPromise<void>::runOnSameThread([ = ](tPromiseFunctions<void>::SuccessFunction res, tPromiseFunctions<void>::FailureFunction rej) {
        Q_UNUSED(rej)

        TransparentDialog* dialog = new TransparentDialog();
        dialog->setWindowFlag(Qt::FramelessWindowHint);
        dialog->setWindowFlag(Qt::WindowStaysOnTopHint);
        dialog->showFullScreen();

        tScrim::scrimForWidget(dialog)->setBlurEnabled(false);

        QTimer::singleShot(500, [ = ] {
            EndSession* popoverContents = new EndSession();

            tPopover* popover = new tPopover(popoverContents);
            popover->setPopoverSide(tPopover::Bottom);
            popover->setPopoverWidth(popoverContents->heightForWidth(dialog->width()));
//            popover->setPerformBlur(false);
            connect(popoverContents, &EndSession::done, popover, &tPopover::dismiss);
            connect(popover, &tPopover::dismissed, popoverContents, &EndSession::deleteLater);
            connect(popover, &tPopover::dismissed, [ = ] {
                popover->deleteLater();
                dialog->deleteLater();
                popoverContents->deleteLater();

                res();
            });
            popover->show(dialog);
        });
    });
}
void EndSession::on_powerOffButton_clicked() {
    StateManager::instance()->powerManager()->performPowerOperation(PowerManager::PowerOff);
    emit done();
}

void EndSession::on_rebootButton_clicked() {
    StateManager::instance()->powerManager()->performPowerOperation(PowerManager::Reboot);
    emit done();
}

void EndSession::on_suspendButton_clicked() {
    StateManager::instance()->powerManager()->performPowerOperation(PowerManager::Suspend);
    emit done();
}

void EndSession::on_exitButton_clicked() {
    qTerminate();
}

void EndSession::on_titleLabel_backButtonClicked() {
    emit done();
}
