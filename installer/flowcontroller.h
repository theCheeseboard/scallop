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
#ifndef FLOWCONTROLLER_H
#define FLOWCONTROLLER_H

#include <functional>
#include <QObject>

class FlowController : public QObject {
        Q_OBJECT
    public:
        explicit FlowController(QObject* parent = nullptr);

        static FlowController* instance();

    signals:
        void nextPage();
        void previousPage();

        void currentPageChanged(QWidget* page);

        void setSkipPage(QWidget* page, bool skip);
        void setSkipPage(QWidget* page, std::function<bool()> skip);
};

#endif // FLOWCONTROLLER_H
