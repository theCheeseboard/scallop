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
#include "issuespage.h"
#include "ui_issuespage.h"

#include "flowcontroller.h"
#include "installerdata.h"
#include <tstatusframe.h>
#include <QDBusInterface>

struct IssuesPagePrivate {
    tStatusFrame* diskFrame;
    tStatusFrame* memoryFrame;
    tStatusFrame* powerFrame;
    tStatusFrame* vmFrame;
};

IssuesPage::IssuesPage(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::IssuesPage) {
    ui->setupUi(this);
    d = new IssuesPagePrivate();

    ui->titleLabel->setBackButtonShown(true);

    QDBusConnection::systemBus().connect("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(reloadIssues()));

    connect(ui->issuesWidget, &IssuesWidget::hasIssuesChanged, this, [ = ] {
        FlowController::instance()->setSkipPage(this, !ui->issuesWidget->hasIssues());
        ui->nextButton->setText(ui->issuesWidget->hasIssues() ? tr("Ignore and Continue") : tr("Next"));
    });
    connect(ui->issuesWidget, &IssuesWidget::hasErrorIssueChanged, this, [ = ] {
        ui->nextButton->setEnabled(!ui->issuesWidget->hasErrorIssue());

        if (ui->issuesWidget->hasErrorIssue()) {
            ui->descriptionLabel->setText(tr("We're unable to install %1 on this device right now. Solve the issues below and then give it another go.").arg(InstallerData::systemName()));
        } else {
            ui->descriptionLabel->setText(tr("The following issues may impact the installation. You should solve them before we continue."));
        }
    });
    ui->issuesWidget->hasIssuesChanged();
    ui->issuesWidget->hasErrorIssueChanged();

    reloadIssues();
}

IssuesPage::~IssuesPage() {
    delete ui;
    delete d;
}

void IssuesPage::on_titleLabel_backButtonClicked() {
    FlowController::instance()->previousPage();
}

void IssuesPage::on_nextButton_clicked() {
    FlowController::instance()->nextPage();
}

void IssuesPage::reloadIssues() {
    ui->issuesWidget->clearIssues();

    //TODO: Check for disk space
    //TODO: Check for memory

    //Check for power
    QDBusInterface upowerInterface("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.UPower", QDBusConnection::systemBus());
    if (upowerInterface.property("OnBattery").toBool()) {
        ui->issuesWidget->addIssue(tr("Power"), tr("It is highly recommended that you connect this device to power before you start installing."), tStatusFrame::Warning);
    }

    //Check for VM
    QDBusInterface hostnameInterface("org.freedesktop.hostname1", "/org/freedesktop/hostname1", "org.freedesktop.hostname1", QDBusConnection::systemBus());
    if (hostnameInterface.property("Chassis").toString() == QStringLiteral("vm")) {
        ui->issuesWidget->addIssue(tr("Virtual Machine"), tr("Looks like you're installing %1 on a virtual machine. Performance on the installed system may suffer as a result.").arg(InstallerData::systemName()), tStatusFrame::Warning);
    }

}
