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
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "downloadprogress.h"
#include "finalresetpopover.h"
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QScreen>
#include <QToolButton>
#include <tcsdtools.h>
#include <tpopover.h>

#include <PolkitQt1/Authority>
#include <PolkitQt1/Subject>

struct MainWindowPrivate {
        tCsdTools csd;
};

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    QRect geometry;
    geometry.setSize((SC_DPI_T(QSize(800, 600), QSize)));
    geometry.moveCenter(QApplication::screens().first()->geometry().center());
    this->setGeometry(geometry);
    this->setFixedSize(geometry.size());

    d = new MainWindowPrivate();
    d->csd.installMoveAction(ui->topWidget);
    d->csd.installResizeAction(this);

    QToolButton* closeButton = new QToolButton();
    closeButton->setIcon(QIcon(":/tcsdtools/close.svg"));
    closeButton->setIconSize(SC_DPI_T(QSize(24, 24), QSize));
    connect(closeButton, &QToolButton::clicked, this, [=] {
        this->close();
    });

    if (tCsdGlobal::windowControlsEdge() == tCsdGlobal::Left) {
        ui->leftCsdLayout->addWidget(closeButton);
    } else {
        ui->rightCsdLayout->addWidget(closeButton);
    }

    ui->menuButton->setIcon(QIcon::fromTheme("scallop-reset", QIcon(":/icons/scallop-reset.svg")));
    ui->menuButton->setIconSize(SC_DPI_T(QSize(24, 24), QSize));

    // Get distribution information
    QString systemName;
    QString osreleaseFile = "";
    if (QFile("/etc/os-release").exists()) {
        osreleaseFile = "/etc/os-release";
    } else if (QFile("/usr/lib/os-release").exists()) {
        osreleaseFile = "/usr/lib/os-release";
    }

    if (osreleaseFile != "") {
        QMap<QString, QString> values;

        QFile information(osreleaseFile);
        information.open(QFile::ReadOnly);

        while (!information.atEnd()) {
            QString line = information.readLine().trimmed();
            int equalsIndex = line.indexOf("=");

            QString key = line.left(equalsIndex);
            QString value = line.mid(equalsIndex + 1);
            if (value.startsWith("\"") && value.endsWith("\"")) value = value.mid(1, value.count() - 2);
            values.insert(key, value);
        }
        information.close();

        systemName = values.value("PRETTY_NAME", tr("Unknown"));
    } else {
        systemName = tr("Unknown");
    }

    const char* untranslated = QT_TR_NOOP(
        "**BUCKLE UP!**\n\n"
        "Resetting this device will erase all personal data **for all accounts.** This includes:\n"
        "- Documents\n"
        "- Pictures\n"
        "- Installed apps\n"
        "- Any changed settings\n\n"
        "**READY TO DO THIS?**\n\n"
        "To get the ball rolling, we'll restart this device and start removing all the data on it. Once the reset is complete, a new copy of %1 will be installed. This process can take a while.");

    ui->descriptionLabel->setText(tr(untranslated).arg(systemName));
    ui->descriptionLabel->setTextFormat(Qt::MarkdownText);

    ui->resetButton->setProperty("type", "destructive");
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_noResetButton_clicked() {
    this->close();
}

void MainWindow::on_resetButton_clicked() {
    PolkitQt1::UnixProcessSubject subject(QApplication::applicationPid());

    if (PolkitQt1::Authority::instance()->checkAuthorizationSync("com.vicr123.scallop.reset.trigger-reset", subject, PolkitQt1::Authority::AllowUserInteraction) == PolkitQt1::Authority::Yes) {
        FinalResetPopover* jp = new FinalResetPopover();
        tPopover* popover = new tPopover(jp);
        popover->setPopoverWidth(SC_DPI(-200));
        popover->setPopoverSide(tPopover::Bottom);
        connect(jp, &FinalResetPopover::rejected, popover, &tPopover::dismiss);
        connect(jp, &FinalResetPopover::accepted, popover, [=] {
            // Perform the reset
            bool requireDownload = !QFile::exists(SCALLOP_PACKAGED_LOCATION);
            QStringList resetTriggerArgs = {"--trigger"};

            QString destFile;
            if (requireDownload) {
                destFile = QDir::home().absoluteFilePath(".scallop-reset-root.squashfs");
                resetTriggerArgs.append("--root-filesystem");
                resetTriggerArgs.append(destFile);
            } else {
                resetTriggerArgs.append("--reboot");
            }

            QProcess* proc = new QProcess();
            proc->start("scallop-reset-trigger", resetTriggerArgs);

            if (requireDownload) {
                DownloadProgress* prog = new DownloadProgress(destFile);
                prog->showFullScreen();
                this->hide();
            }
        });
        connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
        connect(popover, &tPopover::dismissed, jp, &FinalResetPopover::deleteLater);
        popover->show(this->window());
    }
}
