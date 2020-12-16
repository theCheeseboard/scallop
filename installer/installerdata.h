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
#ifndef INSTALLERDATA_H
#define INSTALLERDATA_H

#include <QObject>

struct InstallerDataPrivate;
class InstallerData : public QObject {
        Q_OBJECT
    public:
        explicit InstallerData(QObject* parent = nullptr);

        static InstallerData* instance();

        static void insert(QString key, QJsonValue value);
        static QJsonValue value(QString key);
        static void remove(QString key);

        static QByteArray exportData();
        static bool importData(QByteArray data);

        static void insertTemp(QString key, QVariant value);
        static QVariant valueTemp(QString key);

        static QString systemName();
        static quint64 minimumMemory();
        static bool isEfi();

    signals:

    private:
        InstallerDataPrivate* d;
};

#endif // INSTALLERDATA_H
