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
#include "splitpopover.h"
#include "ui_splitpopover.h"

#include <driveobjectmanager.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/blockinterface.h>
#include "installerdata.h"

struct SplitPopoverPrivate {
    QString otherSystemBlock;
    quint64 totalSize;
    quint64 oldSystemSize;
};

SplitPopover::SplitPopover(QString otherSystemName, QString otherSystemBlock, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SplitPopover) {
    ui->setupUi(this);
    d = new SplitPopoverPrivate();
    d->otherSystemBlock = otherSystemBlock;

    ui->titleLabel->setBackButtonShown(true);
    ui->splitWidget->setFixedWidth(SC_DPI(600));

    DiskObject* otherBlock = DriveObjectManager::diskByBlockName(otherSystemBlock);
    d->totalSize = otherBlock->interface<BlockInterface>()->size();
//    ui->splitSlider->setMaximum(totalSize);
//    ui->splitSlider->setValue(totalSize / 2);

    ui->splitter->setSizes({1, 1});

    ui->otherSystemName->setText(otherSystemName);
    ui->thisSystemName->setText(InstallerData::systemName());

    QPalette oldPal = ui->otherSystemBlock->palette();
    oldPal.setColor(QPalette::Window, oldPal.color(QPalette::Window).darker());
    ui->otherSystemBlock->setPalette(oldPal);
    QPalette newPal = ui->otherSystemBlock->palette();
    newPal.setColor(QPalette::Window, newPal.color(QPalette::Highlight));
    ui->thisSystemBlock->setPalette(newPal);

    updateSplitter();
}

SplitPopover::~SplitPopover() {
    delete ui;
    delete d;
}

void SplitPopover::on_titleLabel_backButtonClicked() {
    emit rejected();
}


void SplitPopover::on_okButton_clicked() {
    InstallerData::insert("diskType", QStringLiteral("probe-resize-block"));
    InstallerData::insert("newSize", QString::number(d->oldSystemSize));
    InstallerData::insert("probeBlock", d->otherSystemBlock);
    emit accepted();
}


void SplitPopover::on_splitter_splitterMoved(int pos, int index) {
    updateSplitter();
}

void SplitPopover::updateSplitter() {
    int totalSize = 0;
    for (int size : ui->splitter->sizes()) totalSize += size;

    double percentage = static_cast<double>(ui->splitter->sizes().first()) / totalSize;
    d->oldSystemSize = d->totalSize * percentage;
    quint64 newSystemSize = d->totalSize - d->oldSystemSize;

    ui->otherSystemSize->setText(QLocale().formattedDataSize(d->oldSystemSize));
    ui->thisSystemSize->setText(QLocale().formattedDataSize(newSystemSize));
}

