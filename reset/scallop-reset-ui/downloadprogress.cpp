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
#include "downloadprogress.h"
#include "ui_downloadprogress.h"

#include <QFile>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QMessageBox>
#include <QProcess>
#include <QDBusMessage>
#include <the-libs_global.h>

struct DownloadProgressPrivate {
    QNetworkAccessManager mgr;
};

DownloadProgress::DownloadProgress(QString destFileName, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::DownloadProgress) {
    ui->setupUi(this);
    d = new DownloadProgressPrivate();

    this->setCursor(QCursor(Qt::BlankCursor));

    QFile* destFile = new QFile(destFileName);
    destFile->open(QFile::WriteOnly);

    QNetworkRequest request(QUrl(SCALLOP_ROOTFS_LOCATION));
    QNetworkReply* reply = d->mgr.get(request);
    connect(reply, &QNetworkReply::finished, this, [ = ] {
        destFile->close();

        if (reply->error() == QNetworkReply::NoError) {
            //Proceed to reboot the device
            QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Reboot");
            message.setArguments({false});
            QDBusConnection::systemBus().call(message);
        } else {
            //Undo the reset trigger
            QProcess::startDetached("scallop-reset-trigger", {"--undo-trigger"});
            destFile->remove();
            QMessageBox::critical(this, tr("Reset Failed"), tr("We weren't able to download the recovery image. Please try again later."));
            QApplication::exit();
        }
    });
    connect(reply, &QNetworkReply::downloadProgress, this, [ = ](qint64 bytesReceived, qint64 bytesTotal) {
        ui->downloadProgress->setMaximum(bytesTotal);
        ui->downloadProgress->setValue(bytesReceived);
        ui->downloadProgressLabel->setText(tr("Downloaded %1 of %2").arg(QLocale().formattedDataSize(bytesReceived), QLocale().formattedDataSize(bytesTotal)));
    });
    connect(reply, &QNetworkReply::readyRead, this, [ = ] {
        destFile->write(reply->readAll());
    });

    QPalette pal = this->palette();
    pal.setColor(QPalette::Window, Qt::black);
    pal.setColor(QPalette::WindowText, Qt::white);
    this->setPalette(pal);

    pal.setColor(QPalette::Window, QColor::fromRgb(0xc0c0c0));
    pal.setColor(QPalette::WindowText, Qt::transparent);
    ui->downloadProgress->setPalette(pal);
    ui->downloadProgress->setAutoFillBackground(true);

    ui->vendorWatermark->setPixmap(QPixmap(SCALLOP_VENDOR_WATERMARK_LOCATION));
    ui->downloadProgress->setFixedHeight(SC_DPI(5));
}

DownloadProgress::~DownloadProgress() {
    delete d;
    delete ui;
}


void DownloadProgress::resizeEvent(QResizeEvent* event) {
    ui->topSpacer->changeSize(0, this->height() * 0.35, QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->bottomSpacer->changeSize(0, this->height() * 0.2, QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->downloadProgress->setFixedWidth(this->width() * 0.4);
}
