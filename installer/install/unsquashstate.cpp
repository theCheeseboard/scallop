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
#include "unsquashstate.h"

#include <QTextStream>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "installerdata.h"

UnsquashState::UnsquashState(QState* parent) : QState(parent) {

}


void UnsquashState::onEntry(QEvent* event) {
    QString squashfsFile = SCALLOP_PACKAGED_LOCATION;
    if (QFile::exists(squashfsFile)) {
        performUnsquash(squashfsFile);
    } else {
        performDownload();
    }
}

void UnsquashState::performUnsquash(QString squashfsFile) {
    //Start unsquashing the filesystem
    QTextStream(stdout) << tr("Unsquashing Filesystem") << "\n";

    QProcess* proc = new QProcess();
    proc->setProcessChannelMode(QProcess::MergedChannels);
    connect(proc, &QProcess::readyRead, this, [ = ] {
        QString data = proc->readAll();

        for (QString part : data.split(" ", Qt::SkipEmptyParts)) {
            if (part.contains("/")) {
                QStringList numberParts = part.split("/");
                if (numberParts.count() == 2) {
                    QTextStream(stdout) << "\r" << tr("Unsquashing Filesystem") << " | " << QString::number(static_cast<int>(numberParts.first().toDouble() / numberParts.at(1).toDouble() * 100)).rightJustified(3) + "%";
                }
            }
        }

        QTextStream(stdout).flush();
    });
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [ = ](int exitCode, QProcess::ExitStatus exitStatus) {
        QTextStream(stdout) << "\r" << tr("Unsquash Complete!") << " | 100%\n";
        emit finished();
    });
    proc->start("unsquashfs", {"-f", "-d", InstallerData::valueTemp("systemRoot").toString(), squashfsFile});
}

void UnsquashState::performDownload() {
    //We need to download the squashfs file!
    QTextStream(stdout) << tr("Downloading %1").arg(InstallerData::systemName()) << "\n";

    QFile* temporarySquashFile = new QFile(QDir(InstallerData::valueTemp("systemRoot").toString()).absoluteFilePath("rootfs.squashfs"));
    temporarySquashFile->open(QFile::WriteOnly);

    QNetworkAccessManager* mgr = new QNetworkAccessManager();
    QNetworkReply* reply = mgr->get(QNetworkRequest(QUrl(SCALLOP_ROOTFS_LOCATION)));
    connect(reply, &QNetworkReply::downloadProgress, this, [ = ](qint64 progress, qint64 total) {
        QString percentage = QString::number(static_cast<int>(static_cast<double>(progress) / total * 100));
        QTextStream(stdout) << "\r" << tr("Downloading %1 (%2 of %3)").arg(InstallerData::systemName(), QLocale().formattedDataSize(progress), QLocale().formattedDataSize(total)) << " | " << percentage.rightJustified(3) + "%";
    });
    connect(reply, &QNetworkReply::readyRead, this, [ = ] {
        temporarySquashFile->write(reply->readAll());
    });
    connect(reply, &QNetworkReply::finished, this, [ = ] {
        temporarySquashFile->close();

        if (reply->error() == QNetworkReply::NoError) {
            performUnsquash(temporarySquashFile->fileName());
            connect(this, &UnsquashState::finished, this, [ = ] {
                temporarySquashFile->remove();
            });
        } else {
            emit failure();
        }
    });
}
