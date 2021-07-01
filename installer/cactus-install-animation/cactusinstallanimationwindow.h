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
#ifndef CACTUSINSTALLANIMATIONWINDOW_H
#define CACTUSINSTALLANIMATIONWINDOW_H

#include <QDialog>

namespace Ui {
    class CactusInstallAnimationWindow;
}

struct CactusInstallAnimationWindowPrivate;
class CactusInstallAnimationWindow : public QDialog {
        Q_OBJECT

    public:
        explicit CactusInstallAnimationWindow(QWidget* parent = nullptr);
        ~CactusInstallAnimationWindow();

    private:
        Ui::CactusInstallAnimationWindow* ui;
        CactusInstallAnimationWindowPrivate* d;

    private slots:
        void on_rebootButton_clicked();
        void on_powerOffButton_clicked();

        // QObject interface
        void on_muteButton_toggled(bool checked);

    public:
        bool eventFilter(QObject* watched, QEvent* event);

        // QWidget interface
    protected:
        void resizeEvent(QResizeEvent* event);
};

#endif // CACTUSINSTALLANIMATIONWINDOW_H
