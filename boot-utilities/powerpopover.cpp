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
#include "powerpopover.h"
#include "ui_powerpopover.h"

PowerPopover::PowerPopover(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::PowerPopover) {
    ui->setupUi(this);

    ui->titleLabel->setBackButtonShown(true);
    ui->powerOffButton->setProperty("type", "destructive");
    ui->rebootButton->setProperty("type", "destructive");
}

PowerPopover::~PowerPopover() {
    delete ui;
}

void PowerPopover::on_powerOffButton_clicked() {
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "PowerOff");
    message.setArguments({true});
    QDBusConnection::systemBus().call(message);
}

void PowerPopover::on_rebootButton_clicked() {
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Reboot");
    message.setArguments({true});
    QDBusConnection::systemBus().call(message);
}

void PowerPopover::on_titleLabel_backButtonClicked() {
    emit done();
}
