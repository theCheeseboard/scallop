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
#ifndef NETWORKPAGE_H
#define NETWORKPAGE_H

#include <QWidget>

namespace Ui {
    class NetworkPage;
}

class QListWidgetItem;
struct NetworkPagePrivate;
class NetworkPage : public QWidget {
        Q_OBJECT

    public:
        explicit NetworkPage(QWidget* parent = nullptr);
        ~NetworkPage();

    private slots:
        void on_nextButton_clicked();

        void on_titleLabel_backButtonClicked();

        void on_networkList_itemActivated(QListWidgetItem* item);

    private:
        Ui::NetworkPage* ui;
        NetworkPagePrivate* d;

        void updateWirelessDevice();
        void updateAPs();
};

#endif // NETWORKPAGE_H
