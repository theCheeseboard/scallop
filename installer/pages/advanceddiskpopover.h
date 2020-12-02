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
#ifndef ADVANCEDDISKPOPOVER_H
#define ADVANCEDDISKPOPOVER_H

#include <QWidget>
#include <QJsonObject>

namespace Ui {
    class AdvancedDiskPopover;
}

struct AdvancedDiskPopoverPrivate;
class AdvancedDiskPopover : public QWidget {
        Q_OBJECT

    public:
        explicit AdvancedDiskPopover(QWidget* parent = nullptr);
        ~AdvancedDiskPopover();

    signals:
        void rejected();
        void accepted(QJsonObject diskInformation);

    private slots:
        void on_titleLabel_backButtonClicked();

        void on_editPartitionsButton_clicked();

        void on_treeView_activated(const QModelIndex& index);

        void on_acceptButton_clicked();

        void on_bootloaderCheckbox_toggled(bool checked);

    private:
        Ui::AdvancedDiskPopover* ui;
        AdvancedDiskPopoverPrivate* d;

        void updateState();
};

#endif // ADVANCEDDISKPOPOVER_H
