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
#include "installerdata.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>

struct InstallerDataPrivate {
    QJsonObject data;
    QVariantMap tempData;
    QString systemName;
};

InstallerData::InstallerData(QObject* parent) : QObject(parent) {
    d = new InstallerDataPrivate();

    //Get distribution information
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

        d->systemName = values.value("PRETTY_NAME", tr("Unknown"));
    } else {
        d->systemName = tr("Unknown");
    }
}

InstallerData* InstallerData::instance() {
    static InstallerData* data = new InstallerData();
    return data;
}

void InstallerData::insert(QString key, QJsonValue value) {
    instance()->d->data.insert(key, value);
}

QJsonValue InstallerData::value(QString key) {
    return instance()->d->data.value(key);
}

void InstallerData::remove(QString key) {
    instance()->d->data.remove(key);
}

QString InstallerData::systemName() {
    return instance()->d->systemName;
}

quint64 InstallerData::minimumMemory() {
    //TODO: use a description file
    return 4294967296;
}

bool InstallerData::isEfi() {
    return QDir("/sys/firmware/efi").exists();
}

QByteArray InstallerData::exportData() {
    return QJsonDocument(instance()->d->data).toJson();
}

bool InstallerData::importData(QByteArray data) {
    QJsonParseError error;
    instance()->d->data = QJsonDocument::fromJson(data, &error).object();
    return error.error == QJsonParseError::NoError;
}

void InstallerData::insertTemp(QString key, QVariant value) {
    instance()->d->tempData.insert(key, value);
}

QVariant InstallerData::valueTemp(QString key) {
    return instance()->d->tempData.value(key);
}
