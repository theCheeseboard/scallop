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
#include "onboardingcompleteoobe.h"
#include "ui_onboardingcompleteoobe.h"

#include <QFile>
#include <libcontemporary_global.h>
#include <onboardingmanager.h>
#include <statemanager.h>

OnboardingCompleteOobe::OnboardingCompleteOobe(QWidget *parent)
    : OnboardingPage(parent), ui(new Ui::OnboardingCompleteOobe) {
  ui->setupUi(this);

  // Get distribution information
  QString osreleaseFile = "";
  if (QFile("/etc/os-release").exists()) {
    osreleaseFile = "/etc/os-release";
  } else if (QFile("/usr/lib/os-release").exists()) {
    osreleaseFile = "/usr/lib/os-release";
  }

  if (osreleaseFile != "") {
    QMap<QString, QString> values;

    QFile information(osreleaseFile);
    information.open(QFile::ReadOnly);

    while (!information.atEnd()) {
      QString line = information.readLine().trimmed();
      int equalsIndex = line.indexOf("=");

      QString key = line.left(equalsIndex);
      QString value = line.mid(equalsIndex + 1);
      if (value.startsWith("\"") && value.endsWith("\""))
        value = value.mid(1, value.count() - 2);
      values.insert(key, value);
    }
    information.close();

    ui->thankYouMessage->setText(
        tr("We hope you enjoy using %1!")
            .arg(values.value("PRETTY_NAME", tr("your new device"))));

    // Attempt to find a sane icon for the OS logo
    // Arch (and by extension, Cactus) puts it under /usr/share/pixmaps so we
    // search there too
    ui->iconLabel->setPixmap(
        QIcon::fromTheme(values.value("LOGO"),
                         QIcon(QStringLiteral("/usr/share/pixmaps/%1.svg")
                                   .arg(values.value("LOGO"))))
            .pixmap(SC_DPI_T(QSize(128, 128), QSize)));
  } else {
    ui->thankYouMessage->setText(
        tr("We hope you enjoy using %1!").arg(tr("your new device")));
  }
}

OnboardingCompleteOobe::~OnboardingCompleteOobe() { delete ui; }

QString OnboardingCompleteOobe::name() {
  return QStringLiteral("OnboardingCompleteOobe");
}

QString OnboardingCompleteOobe::displayName() { return tr("Done"); }

void OnboardingCompleteOobe::on_finishButton_clicked() {
  StateManager::onboardingManager()->nextStep();
}
