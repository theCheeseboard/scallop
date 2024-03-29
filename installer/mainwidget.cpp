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
#include "mainwidget.h"
#include "ui_mainwidget.h"

#include <QShortcut>
#include <tlogger.h>

#include "flowcontroller.h"
#include <libcontemporary_global.h>

#include "pages/diskpage.h"
#include "pages/disktypepage.h"
#include "pages/encryptpage.h"
#include "pages/finishedpage.h"
#include "pages/issuespage.h"
#include "pages/networkpage.h"
#include "pages/progresspage.h"
#include "pages/readypage.h"
#include "pages/welcomepage.h"

struct MainWidgetPrivate {
        QSet<QWidget*> skipped;
        QMap<QWidget*, std::function<bool()>> skipFunctions;
};

MainWidget::MainWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::MainWidget) {
    ui->setupUi(this);

    d = new MainWidgetPrivate();
    //    this->setFixedSize(SC_DPI_T(QSize(800, 600), QSize));
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);

    QShortcut* debugLogShortcut = new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_L), this);
    connect(debugLogShortcut, &QShortcut::activated, this, [=] {
        tLogger::openDebugLogWindow();
    });

    connect(FlowController::instance(), &FlowController::nextPage, this, [=] {
        int showPage = ui->stackedWidget->currentIndex() + 1;
        while (shouldSkipPage(showPage)) {
            showPage++;
        }

        if (showPage >= ui->stackedWidget->count()) {
            // Do something!
            tWarn("MainWidget") << "Reached the end of the flow!";
        } else {
            ui->stackedWidget->setCurrentIndex(showPage);
        }
    });
    connect(FlowController::instance(), &FlowController::previousPage, this, [=] {
        int showPage = ui->stackedWidget->currentIndex() - 1;
        while (shouldSkipPage(showPage)) {
            showPage--;
        }

        if (showPage < 0) {
            // Do something!
            tWarn("MainWidget") << "Reached the start of the flow!";
        } else {
            ui->stackedWidget->setCurrentIndex(showPage);
        }
    });
    connect(FlowController::instance(), QOverload<QWidget*, bool>::of(&FlowController::setSkipPage), this, [=](QWidget* page, bool skip) {
        if (skip) {
            d->skipped.insert(page);
        } else {
            d->skipped.remove(page);
        }
    });
    connect(FlowController::instance(), QOverload<QWidget*, std::function<bool()>>::of(&FlowController::setSkipPage), this, [=](QWidget* page, std::function<bool()> function) {
        d->skipFunctions.insert(page, function);
    });

    connect(ui->stackedWidget, &tStackedWidget::switchingFrame, this, [=](int page) {
        emit FlowController::instance()->currentPageChanged(ui->stackedWidget->widget(page));
    });

    ui->stackedWidget->addWidget(new WelcomePage());
    ui->stackedWidget->addWidget(new NetworkPage());
    ui->stackedWidget->addWidget(new IssuesPage());
    ui->stackedWidget->addWidget(new DiskPage());
    ui->stackedWidget->addWidget(new DiskTypePage());
    ui->stackedWidget->addWidget(new EncryptPage());
    ui->stackedWidget->addWidget(new ReadyPage());
    ui->stackedWidget->addWidget(new ProgressPage());
    ui->stackedWidget->addWidget(new FinishedPage());
}

MainWidget::~MainWidget() {
    delete ui;
}

MainWidget::CloseAction MainWidget::closeAction() {
    QWidget* currentWidget = ui->stackedWidget->widget(ui->stackedWidget->currentIndex());
    if (qobject_cast<ProgressPage*>(currentWidget)) {
        return Disallow;
    } else if (qobject_cast<FinishedPage*>(currentWidget)) {
        return Close;
    } else {
        return Confirm;
    }
}

bool MainWidget::shouldSkipPage(int page) {
    QWidget* w = ui->stackedWidget->widget(page);
    if (d->skipped.contains(w)) return true;
    if (d->skipFunctions.contains(w) && d->skipFunctions.value(w)()) return true;
    return false;
}

void MainWidget::on_exitButton_clicked() {
    QApplication::exit();
}
