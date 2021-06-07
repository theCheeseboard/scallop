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
#ifndef ENDSESSION_H
#define ENDSESSION_H

#include <QWidget>
#include <tpromise.h>
#include <powermanager.h>

namespace Ui {
    class EndSession;
}

class EndSession : public QWidget {
        Q_OBJECT

    public:
        explicit EndSession(QWidget* parent = nullptr);
        ~EndSession();

        static tPromise<void>* showDialog();

    signals:
        void done();

    private slots:
        void on_powerOffButton_clicked();

        void on_rebootButton_clicked();

        void on_suspendButton_clicked();

        void on_exitButton_clicked();

        void on_titleLabel_backButtonClicked();

    private:
        Ui::EndSession* ui;
};

#endif // ENDSESSION_H
