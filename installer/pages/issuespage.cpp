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
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Fade);

    d->diskFrame = new tStatusFrame(this);
    d->diskFrame->setTitle(tr("Disk Space"));
    d->diskFrame->setText(tr("This device doesn't have enough disk space to install %1.").arg(InstallerData::systemName()));
    d->diskFrame->setState(tStatusFrame::Error);
    ui->issuesLayout->addWidget(d->diskFrame);

    d->memoryFrame = new tStatusFrame(this);
    d->memoryFrame->setTitle(tr("Memory"));
    d->memoryFrame->setText(tr("This system does not meet the minimum memory requirement of %1. Performance on the installed system may suffer as a result.").arg(QLocale().formattedDataSize(InstallerData::minimumMemory())));
    d->memoryFrame->setState(tStatusFrame::Warning);
    ui->issuesLayout->addWidget(d->memoryFrame);

    d->powerFrame = new tStatusFrame(this);
    d->powerFrame->setTitle(tr("Power"));
    d->powerFrame->setText(tr("It is highly recommended that you connect this device to power before you start installing."));
    d->powerFrame->setState(tStatusFrame::Warning);
    ui->issuesLayout->addWidget(d->powerFrame);

    d->vmFrame = new tStatusFrame(this);
    d->vmFrame->setTitle(tr("Virtual Machine"));
    d->vmFrame->setText(tr("Looks like you're installing %1 on a virtual machine. Performance on the installed system may suffer as a result.").arg(InstallerData::systemName()));
    d->vmFrame->setState(tStatusFrame::Warning);
    ui->issuesLayout->addWidget(d->vmFrame);

    QDBusConnection::systemBus().connect("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(reloadIssues()));

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
    int issues = 0;
    bool blocking = false;

    //TODO: Check for disk space
    d->diskFrame->setVisible(false);

    //TODO: Check for memory
    d->memoryFrame->setVisible(false);

    //Check for power
    QDBusInterface upowerInterface("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.UPower", QDBusConnection::systemBus());
    if (upowerInterface.property("OnBattery").toBool()) {
        d->powerFrame->setVisible(true);
        issues++;
    } else {
        d->powerFrame->setVisible(false);
    }

    //Check for VM
    QDBusInterface hostnameInterface("org.freedesktop.hostname1", "/org/freedesktop/hostname1", "org.freedesktop.hostname1", QDBusConnection::systemBus());
    if (hostnameInterface.property("Chassis").toString() == QStringLiteral("vm")) {
        d->vmFrame->setVisible(true);
        issues++;
    } else {
        d->vmFrame->setVisible(false);
    }

    ui->nextButton->setEnabled(!blocking);
    FlowController::instance()->setSkipPage(this, issues == 0);
    ui->stackedWidget->setCurrentWidget(issues ? ui->issuesPage : ui->noIssuesPage);
    ui->nextButton->setText(issues ? tr("Ignore and Continue") : tr("Next"));

    if (blocking) {
        ui->descriptionLabel->setText(tr("We're unable to install %1 on this device right now. Solve the issues below and then give it another go.").arg(InstallerData::systemName()));
    } else {
        ui->descriptionLabel->setText(tr("The following issues may impact the installation. You should solve them before we continue."));
    }
}
