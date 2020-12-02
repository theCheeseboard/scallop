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
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QShortcut>
#include <QMessageBox>
#include <QScreen>
#include <tcsdtools.h>
#include "installerdata.h"

struct MainWindowPrivate {
    tCsdTools csd;
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    this->setWindowTitle(tr("Install %1").arg(InstallerData::systemName()));

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
    connect(closeButton, &QToolButton::clicked, this, [ = ] {
        this->close();
    });

    if (tCsdGlobal::windowControlsEdge() == tCsdGlobal::Left) {
        ui->leftCsdLayout->addWidget(closeButton);
    } else {
        ui->rightCsdLayout->addWidget(closeButton);
    }

    ui->menuButton->setIcon(QIcon::fromTheme("scallop-installer", QIcon(":/icons/scallop-installer.svg")));
    ui->menuButton->setIconSize(SC_DPI_T(QSize(24, 24), QSize));
}

MainWindow::~MainWindow() {
    delete ui;
    delete d;
}

void MainWindow::closeEvent(QCloseEvent* event) {
    switch (ui->mainWidget->closeAction()) {
        case MainWidget::Confirm: {
            if (QMessageBox::question(this, tr("Exit?"), tr("%1 is not completely installed. Do you still want to exit?").arg(InstallerData::systemName()), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
                event->ignore();
            }
            break;
        }
        case MainWidget::Disallow:
            QMessageBox::warning(this, tr("Installation is ongoing"), tr("Sorry, installation cannot be cancelled at this stage."), QMessageBox::Ok, QMessageBox::Ok);
            event->ignore();
            break;
        case MainWidget::Close:
            event->accept();
            break;
    }
}
