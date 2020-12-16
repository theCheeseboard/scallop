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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

struct MainWindowPrivate;
class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        MainWindow(QWidget* parent = nullptr);
        ~MainWindow();

    private slots:
        void on_exitButton_clicked();

        void on_installButton_clicked();

        void on_terminalButton_clicked();

        void on_disksButton_clicked();

        void on_viewFilesButton_clicked();

        void on_languageButton_clicked();

    private:
        Ui::MainWindow* ui;
        MainWindowPrivate* d;

        void setUtilitiesAvailable(bool utilitiesAvailable);
        void updateBackground();
        void updateLabels();
        void updateTranslations(QLocale locale);

        // QObject interface
    public:
        bool eventFilter(QObject* watched, QEvent* event);

        // QWidget interface
    protected:
        void resizeEvent(QResizeEvent* event);
        void changeEvent(QEvent* event);
};
#endif // MAINWINDOW_H
