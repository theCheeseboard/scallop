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
#include "mainwindow.h"

#include <QProcess>
#include <Wm/desktopwm.h>
#include <tapplication.h>

int main(int argc, char* argv[]) {
    qputenv("QT_QPA_PLATFORMTHEME", "thedesk-platform");
    tApplication a(argc, argv);
    a.setApplicationShareDir("scallop/boot-utilities");
    a.installTranslators();

    DesktopWm::instance();

    QProcess::startDetached("kwin_x11", {});
    QProcess::startDetached("/usr/lib/td-polkitagent", {});

    MainWindow w;
    w.showFullScreen();
    return a.exec();
}
