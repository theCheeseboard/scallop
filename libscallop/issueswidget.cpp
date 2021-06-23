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
#include "issueswidget.h"
#include "ui_issueswidget.h"

struct IssuesWidgetPrivate {
    QList<tStatusFrame*> frames;
    bool haveError = false;
};

IssuesWidget::IssuesWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::IssuesWidget) {
    ui->setupUi(this);

    d = new IssuesWidgetPrivate();
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Fade);

    connect(this, &IssuesWidget::hasIssuesChanged, this, [ = ] {
        ui->stackedWidget->setCurrentWidget(this->hasIssues() ? ui->issuesPage : ui->noIssuesPage);
    });
}

IssuesWidget::~IssuesWidget() {
    delete d;
    delete ui;
}

void IssuesWidget::clearIssues() {
    for (tStatusFrame* frame : d->frames) {
        ui->issuesLayout->removeWidget(frame);
        frame->deleteLater();
    }

    d->frames.clear();
    d->haveError = false;

    emit hasIssuesChanged();
    emit hasErrorIssueChanged();
}

bool IssuesWidget::hasIssues() {
    return !d->frames.isEmpty();
}

void IssuesWidget::addIssue(QString title, QString text, tStatusFrame::State type) {
    tStatusFrame* frame = new tStatusFrame(this);
    frame->setTitle(title);
    frame->setText(text);
    frame->setState(type);
    ui->issuesLayout->addWidget(frame);
    d->frames.append(frame);

    emit hasIssuesChanged();
    if (type == tStatusFrame::Error) {
        d->haveError = true;
        emit hasErrorIssueChanged();
    }
}

bool IssuesWidget::hasErrorIssue() {
    return d->haveError;
}
