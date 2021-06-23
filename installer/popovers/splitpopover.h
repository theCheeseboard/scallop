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
#ifndef SPLITPOPOVER_H
#define SPLITPOPOVER_H

#include <QWidget>

namespace Ui {
    class SplitPopover;
}

struct SplitPopoverPrivate;
class SplitPopover : public QWidget {
        Q_OBJECT

    public:
        explicit SplitPopover(QString otherSystemName, QString otherSystemBlock, QWidget* parent = nullptr);
        ~SplitPopover();

    signals:
        void accepted();
        void rejected();

    private slots:
        void on_titleLabel_backButtonClicked();

        void on_okButton_clicked();

        void on_splitter_splitterMoved(int pos, int index);

    private:
        Ui::SplitPopover* ui;
        SplitPopoverPrivate* d;

        void updateSplitter();
};

#endif // SPLITPOPOVER_H
