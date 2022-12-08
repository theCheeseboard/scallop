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
#include "onboardinghostname.h"
#include "ui_onboardinghostname.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <onboardingmanager.h>
#include <statemanager.h>

OnboardingHostname::OnboardingHostname(QWidget *parent)
    : OnboardingPage(parent), ui(new Ui::OnboardingHostname) {
  ui->setupUi(this);

  ui->titleLabel->setBackButtonShown(true);
}

OnboardingHostname::~OnboardingHostname() { delete ui; }

void OnboardingHostname::on_titleLabel_backButtonClicked() {
  StateManager::onboardingManager()->previousStep();
}

void OnboardingHostname::on_nextButton_clicked() {
  QDBusMessage prettyMessage = QDBusMessage::createMethodCall(
      "org.freedesktop.hostname1", "/org/freedesktop/hostname1",
      "org.freedesktop.hostname1", "SetPrettyHostname");
  prettyMessage.setArguments({ui->hostnameEdit->text(), true});
  QDBusConnection::systemBus().asyncCall(prettyMessage);

  QDBusMessage staticMessage = QDBusMessage::createMethodCall(
      "org.freedesktop.hostname1", "/org/freedesktop/hostname1",
      "org.freedesktop.hostname1", "SetStaticHostname");
  staticMessage.setArguments(
      {generateStaticHostname(ui->hostnameEdit->text()), true});
  QDBusConnection::systemBus().asyncCall(staticMessage);

  StateManager::onboardingManager()->nextStep();
}

QString OnboardingHostname::generateStaticHostname(QString hostname) {
  QString allowedCharacters =
      QStringLiteral("abcdefghijklmnopqrstuvwxyz0123456789-");
  QString ignoredCharacters = QStringLiteral("'\"");
  QString staticHostname;
  bool allowHyphen = false;
  for (QChar c : hostname) {
    c = c.toLower();

    if (ignoredCharacters.contains(c))
      continue;

    if (allowedCharacters.contains(c)) {
      staticHostname.append(c);
      allowHyphen = true;
      continue;
    }

    bool haveCharacter = false;
    for (QChar ch : c.decomposition()) {
      if (allowedCharacters.contains(ch)) {
        staticHostname.append(ch);
        allowHyphen = true;
        haveCharacter = true;
        continue;
      }
    }
    if (haveCharacter)
      continue;

    if (allowHyphen) {
      allowHyphen = false;
      staticHostname.append("-");
    }
  }

  if (staticHostname.endsWith("-"))
    staticHostname.remove(staticHostname.length() - 1, 1);
  staticHostname.truncate(63);
  if (staticHostname.isEmpty())
    return QStringLiteral("localhost");
  return staticHostname;
}

QString OnboardingHostname::name() {
  return QStringLiteral("OnboardingHostname");
}

QString OnboardingHostname::displayName() { return tr("Device Name"); }

void OnboardingHostname::on_hostnameEdit_textChanged(const QString &arg1) {
  if (arg1.isEmpty()) {
    ui->nextButton->setEnabled(false);
    ui->networkCompatibleName->setText("");
  } else {
    ui->nextButton->setEnabled(true);
    ui->networkCompatibleName->setText(generateStaticHostname(arg1));
  }
}
