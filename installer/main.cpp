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
#include "install/installmanager.h"

#include <tapplication.h>
#include <QCommandLineParser>
#include <scalloplib.h>

int main(int argc, char* argv[]) {
    tApplication a(argc, argv);
    a.setShareDir("/usr/share/scallop/install-system");
    a.installTranslators();

    ScallopLib::init();

    QCommandLineParser parser;
    parser.addOption({"install", "Install the system using a descriptor of the install process."});
    parser.addHelpOption();
    parser.process(a);

    if (parser.isSet("install")) {
        InstallManager* i = new InstallManager();
    } else {
        MainWindow* w = new MainWindow();
        w->show();
    }

    return a.exec();
}
