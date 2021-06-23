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
#include "disktypepage.h"
#include "ui_disktypepage.h"

#include <flowcontroller.h>
#include <installerdata.h>
#include <QJsonObject>
#include <QProcess>
#include "popovers/splitpopover.h"
#include <tpromise.h>
#include <tpopover.h>

struct DiskTypePagePrivate {
    uint probeIndex = 0;
    bool probeRunning = false;
    QString probeNext;

    DiskTypePage* parent;

    QList<QWidget*> typesWidgets;

    void probe();
};

DiskTypePage::DiskTypePage(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::DiskTypePage) {
    ui->setupUi(this);

    d = new DiskTypePagePrivate();
    d->parent = this;

    ui->titleLabel->setBackButtonShown(true);
    ui->descriptionLabel->setText(tr("How are we installing %1 today?").arg(InstallerData::systemName()));

    auto shouldSkip = [ = ] {
        //Skip this page if we're using advanced partitioning
        return InstallerData::value("diskType").toString() == QStringLiteral("mount-list");
    };

    FlowController::instance()->setSkipPage(this, shouldSkip);
    connect(FlowController::instance(), &FlowController::currentPageChanged, this, [ = ](QWidget * page) {
        if (page == this && !shouldSkip()) {
            this->probe(InstallerData::value("disk").toObject().value("block").toString());
        }
    });

    ui->eraseDiskButton->setProperty("type", "destructive");
}

DiskTypePage::~DiskTypePage() {
    delete ui;
    delete d;
}

void DiskTypePage::on_titleLabel_backButtonClicked() {
    FlowController::instance()->previousPage();
}

void DiskTypePage::on_eraseDiskButton_clicked() {
    InstallerData::insert("diskType", QStringLiteral("whole-disk"));
    FlowController::instance()->nextPage();
}

void DiskTypePage::probe(QString disk) {
    d->probeNext = disk;

    d->probeIndex++;
    d->probe();
}

void DiskTypePage::probeLine(QString line) {
    QStringList parts = line.split(";");
    if (parts.first() == "ESP") {
        InstallerData::insert("probeEsp", parts.at(1));
    } else if (parts.first() == "OS") {
        QString osName = parts.at(1);
        QString osBlock = parts.at(2);

        QLabel* sectionLabel = new QLabel();
        sectionLabel->setText(tr("Modify %1").arg(osName).toUpper());
        sectionLabel->setMargin(9);

        QFont font = sectionLabel->font();
        font.setBold(true);
        sectionLabel->setFont(font);

        ui->typesLayout->addWidget(sectionLabel);
        d->typesWidgets.append(sectionLabel);

        QCommandLinkButton* shrinkButton = new QCommandLinkButton();
        shrinkButton->setText(tr("Shrink %1 to make space for %2").arg(osName, InstallerData::systemName()));
        shrinkButton->setDescription(tr("Install %1 alongside your existing installation of %2").arg(InstallerData::systemName(), osName));
        connect(shrinkButton, &QCommandLinkButton::clicked, this, [ = ] {
            SplitPopover* jp = new SplitPopover(osName, osBlock);
            tPopover* popover = new tPopover(jp);
            popover->setPopoverWidth(SC_DPI(-200));
            popover->setPopoverSide(tPopover::Bottom);
            connect(jp, &SplitPopover::rejected, popover, &tPopover::dismiss);
            connect(jp, &SplitPopover::accepted, popover, [ = ] {
                popover->dismiss();
                FlowController::instance()->nextPage();
            });
            connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
            connect(popover, &tPopover::dismissed, jp, &SplitPopover::deleteLater);
            popover->show(this->window());
        });
        ui->typesLayout->addWidget(shrinkButton);
        d->typesWidgets.append(shrinkButton);

        if (InstallerData::isEfi()) {
            QCommandLinkButton* replaceButton = new QCommandLinkButton();
            replaceButton->setText(tr("Replace %1 with %2").arg(osName, InstallerData::systemName()));
            replaceButton->setDescription(tr("Erase %1 and install %2 in its place").arg(osName, InstallerData::systemName()));
            replaceButton->setProperty("type", "destructive");
            connect(replaceButton, &QCommandLinkButton::clicked, this, [ = ] {
                InstallerData::insert("diskType", QStringLiteral("probe-replace-block"));
                InstallerData::insert("probeBlock", osBlock);
                FlowController::instance()->nextPage();
            });
            ui->typesLayout->addWidget(replaceButton);
            d->typesWidgets.append(replaceButton);
        }
    }
}

void DiskTypePagePrivate::probe() {
    uint thisProbe = probeIndex;
    if (probeRunning) return;
    if (probeNext.isEmpty()) return;
    probeRunning = true;

    parent->ui->stackedWidget->setCurrentWidget(parent->ui->processingPage);

    QProcess* probeProcess = new QProcess();
    QObject::connect(probeProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), parent, [ = ](int exitCode, QProcess::ExitStatus status) {
        probeRunning = false;

        if (probeIndex != thisProbe) {
            probe();
            return;
        }

        for (QWidget* widget : typesWidgets) {
            parent->ui->typesLayout->removeWidget(widget);
            widget->deleteLater();
        }
        typesWidgets.clear();

        while (probeProcess->canReadLine()) {
            parent->probeLine(probeProcess->readLine().trimmed());
        }

        parent->ui->stackedWidget->setCurrentWidget(parent->ui->typesPage);
    });
    probeProcess->start("sudo", {QCoreApplication::applicationFilePath(), "--probe", probeNext});
}
