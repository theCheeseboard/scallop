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
#include "progresspage.h"
#include "ui_progresspage.h"

#include "flowcontroller.h"
#include "installipcmanager.h"

#include "cactus-install-animation/cactusinstallanimationwindow.h"

ProgressPage::ProgressPage(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ProgressPage) {
    ui->setupUi(this);

    ui->installDescription->setText(tr("Preparing for installation"));
    connect(InstallIpcManager::instance(), &InstallIpcManager::messageChanged, this, [ = ](QString message) {
        ui->installDescription->setText(message);
    });
    connect(InstallIpcManager::instance(), &InstallIpcManager::progressChanged, this, [ = ](int progress) {
        if (progress < 0) {
            ui->progressBar->setMaximum(0);
        } else {
            ui->progressBar->setMaximum(100);
            ui->progressBar->setValue(progress);
        }
    });
    connect(InstallIpcManager::instance(), &InstallIpcManager::success, this, [ = ] {
        FlowController::instance()->nextPage();
    });
    connect(InstallIpcManager::instance(), &InstallIpcManager::failure, this, [ = ] {
        FlowController::instance()->nextPage();
    });

    connect(FlowController::instance(), &FlowController::currentPageChanged, this, [ = ](QWidget * page) {
        //TODO: Check if we should show the animation window
        if (page == this) {
            CactusInstallAnimationWindow* window = new CactusInstallAnimationWindow();
            window->showFullScreen();
        }
    });
}

ProgressPage::~ProgressPage() {
    delete ui;
}
