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
#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

namespace Ui {
    class MainWidget;
}

struct MainWidgetPrivate;
class MainWidget : public QWidget {
        Q_OBJECT

    public:
        explicit MainWidget(QWidget* parent = nullptr);
        ~MainWidget();

        enum CloseAction {
            Confirm,
            Disallow,
            Close
        };

        CloseAction closeAction();

        bool shouldSkipPage(int page);

    private slots:
        void on_exitButton_clicked();

    private:
        Ui::MainWidget* ui;
        MainWidgetPrivate* d;
};

#endif // MAINWIDGET_H
