#include "systemonboarding.h"
#include "ui_systemonboarding.h"

extern Branding branding;
extern float getDPIScaling();

SystemOnboarding::SystemOnboarding(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SystemOnboarding)
{
    ui->setupUi(this);

    ui->tsLogo->setPixmap(QIcon::fromTheme("theshell").pixmap(256, 256));
    ui->tsLogo_2->setPixmap(QIcon::fromTheme("theshell").pixmap(256, 256));
    ui->iconLabel->setPixmap(QIcon(":/icons/icon.svg").pixmap(32 * getDPIScaling(), 32 * getDPIScaling()));

    ui->osNameLabel->setText(branding.name);

    ui->lowerPane->setFixedHeight(0);
    ui->menubar->setFixedHeight(0);

    ui->nextButton->setText(tr("Get Started"));
    ui->backButton->setVisible(false);
    ui->updateCheckFrame->setVisible(false);
    ui->installingUpdatesLabel->setVisible(false);

    //Set up timezone panel
    ui->timezoneList->clear();
    QFile tzInfo("/usr/share/zoneinfo/zone.tab");
    tzInfo.open(QFile::ReadOnly);
    while (!tzInfo.atEnd()) {
        QString tzLine = tzInfo.readLine();
        if (!tzLine.startsWith("#")) {
            QStringList parts = tzLine.trimmed().split("\t", QString::SkipEmptyParts);
            if (parts.length() >= 3) {
                QString region = parts.at(2).left(parts.at(2).indexOf("/"));
                QString city = parts.at(2).mid(parts.at(2).indexOf("/") + 1);

                if (!timezoneData.contains(region)) {
                    QListWidgetItem* i = new QListWidgetItem();
                    i->setText(region);
                    ui->timezoneList->addItem(i);
                    timezoneData.insert(region, QJsonArray());
                }

                QJsonObject cityData;
                cityData.insert("name", city);
                cityData.insert("country", parts.at(0).toLower());
                cityData.insert("descriptor", parts.at(2));

                QJsonArray a = timezoneData.value(region).toArray();
                a.append(cityData);
                timezoneData.insert(region, a);
            }
        }
    }
    tzInfo.close();
}

SystemOnboarding::~SystemOnboarding()
{
    delete ui;
}

void SystemOnboarding::on_nextButton_clicked()
{
    if (ui->pages->currentIndex() == 0) {
        //Check internet connection
        if (ui->networkwidget->hasConnection()) {
            ui->pages->setCurrentIndex(2);
        } else {
            ui->pages->setCurrentIndex(1);
        }
        return;
    } else if (ui->pages->currentIndex() == 1) {
        if (!ui->networkwidget->hasConnection()) {
            ui->pages->setCurrentIndex(3); //Don't bother with updates if there is no connection
        } else {
            ui->pages->setCurrentIndex(2);
        }
        return;
    } else if (ui->pages->currentIndex() == 3) {
        if (ui->password->text() != ui->passwordConfirm->text()) {
            QMessageBox::warning(this, tr("Passwords don't match"), tr("The passwords don't match. Try again."));
            return;
        }
        ui->pages->setCurrentIndex(4);
        return;
    } else if (ui->pages->currentIndex() == 4) {
        finalizeSettings();

        //Wait for updates to finish
        if (!updating) {
            ui->pages->setCurrentIndex(6); //Move onto the completion page
        } else {
            connect(this, &SystemOnboarding::updatesComplete, [=] {
                finishOnboarding();
            });
            ui->pages->setCurrentIndex(5); //Move on to updating page
        }
        return;
    } else if (ui->pages->currentIndex() == 6) {
        finishOnboarding();
        return;
    }
    ui->pages->setCurrentIndex(ui->pages->currentIndex() + 1);
}

void SystemOnboarding::showFullScreen() {
    QMainWindow::showFullScreen();

    QTimer::singleShot(1000, [=] {
        {
            tVariantAnimation* anim = new tVariantAnimation();
            anim->setStartValue(0);
            anim->setEndValue(ui->lowerPane->sizeHint().height());
            anim->setEasingCurve(QEasingCurve::OutCubic);
            anim->setDuration(500);
            connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
                ui->lowerPane->setFixedHeight(value.toInt());
            });
            connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
            anim->start();
        }

        {
            tVariantAnimation* anim = new tVariantAnimation();
            anim->setStartValue(0);
            anim->setEndValue(ui->menubar->sizeHint().height());
            anim->setEasingCurve(QEasingCurve::OutCubic);
            anim->setDuration(500);
            connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
                ui->menubar->setFixedHeight(value.toInt());
            });
            connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
            anim->start();
        }
    });
}

void SystemOnboarding::on_backButton_clicked()
{
    ui->pages->setCurrentIndex(ui->pages->currentIndex() - 1);
}

void SystemOnboarding::on_pages_currentChanged(int arg1)
{
    ui->backButton->setVisible(true);
    ui->nextButton->setVisible(true);
    ui->backButton->setEnabled(true);
    ui->nextButton->setEnabled(true);
    ui->nextButton->setText(tr("Next"));

    switch (arg1) {
        case 0: {
            ui->backButton->setVisible(false);
            ui->nextButton->setText(tr("Get Started"));
            break;
        }
        case 1: { //Network page
            ui->nextButton->setText(tr("Skip"));
            break;
        }
        case 2: { //Updates page
            if (!ui->updateCheckProgressBar->isVisible() && ui->updateCheckFrame->isVisible()) {
                ui->nextButton->setVisible(true);
            }
            ui->nextButton->setVisible(false);
            break;
        }
        case 3: { //User page
            checkUserPage();
            break;
        }
        case 4: { //Timezone page
            ui->nextButton->setVisible(false);
            break;
        }
        case 5: { //Update Page
            ui->backButton->setVisible(false);
            ui->nextButton->setVisible(false);
            break;
        }
        case 6: { //End page
            ui->backButton->setVisible(false);
            ui->nextButton->setText(tr("Finish"));
        }
    }
}

void SystemOnboarding::on_networkwidget_networkAvailable(bool available)
{
    if (available && ui->pages->currentIndex() == 1) {
        ui->pages->setCurrentIndex(2);
    }
}

void SystemOnboarding::on_updateLater_clicked()
{
    ui->pages->setCurrentIndex(3);
}

void SystemOnboarding::on_updateNow_clicked()
{
    ui->backButton->setVisible(false);
    ui->nextButton->setVisible(false);

    QProcess* updateChecker = new QProcess();
    updateChecker->start("checkupdates");
    connect(updateChecker, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus) {
        QStringList updates = QString(updateChecker->readAll()).split("\n");
        updates.removeAll("");
        ui->updateCheckProgressBar->setVisible(false);
        ui->nextButton->setVisible(true);
        ui->backButton->setVisible(true);

        if (updates.count() == 0) {
            ui->updateCheckDescription->setText(tr("There aren't any updates available."));
        } else {
            ui->updateCheckDescription->setText(tr("Updates are available and they'll be installed in the background."));
            ui->installingUpdatesLabel->setVisible(true);
        }

        QProcess* updater = new QProcess();
        updater->start("pacman -Syu --noconfirm");
        connect(updater, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus) {
            ui->installingUpdatesLabel->setVisible(false);
            updater->deleteLater();
            updating = false;
            emit updatesComplete();
        });
        updating = true;

        updateChecker->deleteLater();
    });

    ui->updateCheckButtons->setVisible(false);
    ui->updateCheckFrame->setVisible(true);
}

void SystemOnboarding::on_timezoneList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    ui->timezoneCityList->clear();
    if (current != NULL) {
        QJsonArray a = timezoneData.value(current->text()).toArray();
        for (QJsonValue v : a) {
            QListWidgetItem* i = new QListWidgetItem();
            QJsonObject cityData = v.toObject();
            i->setText(cityData.value("name").toString().replace("_", " "));
            i->setData(Qt::UserRole, cityData.value("descriptor").toString());
            i->setIcon(QIcon::fromTheme("flag-" + cityData.value("country").toString(), QIcon::fromTheme("flag")));
            ui->timezoneCityList->addItem(i);
            if (cityData.value("selected").toBool()) {
                i->setSelected(true);
            }
        }
    }
}

void SystemOnboarding::on_username_textEdited(const QString &arg1)
{
    ui->username->setText(arg1.toLower());
    checkUserPage();
}

void SystemOnboarding::on_fullName_textChanged(const QString &arg1)
{
    if (arg1.contains(" ")) {
        ui->username->setText(arg1.left(arg1.indexOf(" ")).toLower());
    } else {
        ui->username->setText(arg1.toLower());
    }
    checkUserPage();
}

void SystemOnboarding::on_timezoneCityList_currentRowChanged(int currentRow)
{
    if (currentRow != -1) {
        ui->nextButton->setEnabled(true);
    } else {
        ui->nextButton->setEnabled(false);
    }
}

void SystemOnboarding::on_actionReboot_triggered()
{
    if (QMessageBox::question(this, tr("Reboot"), tr("Reboot the system now?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        //Reboot

        QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Reboot");
        QList<QVariant> arguments;
        arguments.append(true);
        message.setArguments(arguments);
        QDBusConnection::systemBus().send(message);
    }
}

void SystemOnboarding::on_actionPower_Off_triggered()
{
    if (QMessageBox::question(this, tr("Power Off"), tr("Power off the system now?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        //Reboot

        QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "PowerOff");
        QList<QVariant> arguments;
        arguments.append(true);
        message.setArguments(arguments);
        QDBusConnection::systemBus().send(message);
    }
}

void SystemOnboarding::checkUserPage() {
    ui->nextButton->setEnabled(false);
    if (ui->username->text() == "") return;
    if (ui->username->text().contains(" ")) return;
    if (ui->fullName->text() == "") return;
    if (ui->password->text() == "") return;
    if (ui->passwordConfirm->text() == "") return;
    if (ui->hostname->text() == "") return;
    if (ui->hostname->text().contains(" ")) return;

    ui->nextButton->setEnabled(true);
}

void SystemOnboarding::on_password_textChanged(const QString &arg1)
{
    checkUserPage();
}

void SystemOnboarding::on_passwordConfirm_textChanged(const QString &arg1)
{
    checkUserPage();
}

void SystemOnboarding::finalizeSettings() {
    //Create a new user
    QProcess::execute("useradd -g wheel -m " + ui->username->text());
    QProcess::execute("chfn -f \"" + ui->fullName->text() + "\" " + ui->username->text());

    QProcess chpasswd;
    chpasswd.start("chpasswd");
    chpasswd.write(QString("root:" + ui->password->text() + "\n").toUtf8());
    chpasswd.write(QString(ui->username->text() + ":" + ui->password->text() + "\n").toUtf8());
    chpasswd.closeWriteChannel();
    chpasswd.waitForFinished(-1);

    QFile hostnameFile("/etc/hostname");
    hostnameFile.open(QFile::WriteOnly);
    hostnameFile.write(ui->hostname->text().toUtf8());
    hostnameFile.close();

    //Set the timezone

    QDBusMessage getMessage = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "ListActivatableNames");
    QDBusReply<QStringList> reply = QDBusConnection::systemBus().call(getMessage);
    if (!reply.value().contains("org.freedesktop.timedate1")) {
        qDebug() << "Can't set date and time";
        return;
    }

    QDBusMessage launchMessage = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "StartServiceByName");
    QVariantList args;
    args.append("org.freedesktop.timedate1");
    args.append((uint) 0);
    launchMessage.setArguments(args);

    QDBusConnection::systemBus().call(launchMessage);

    QDBusInterface dateTimeInterface("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.timedate1", QDBusConnection::systemBus());
    dateTimeInterface.call(QDBus::NoBlock, "SetTimezone", ui->timezoneCityList->currentItem()->data(Qt::UserRole), true);
}

void SystemOnboarding::finishOnboarding() {
    //Enable SDDM and disable Scallop
    QProcess::execute("systemctl disable scallop-onboarding");
    QProcess::execute("systemctl enable sddm-plymouth");

    //Reboot the computer
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Reboot");
    QList<QVariant> arguments;
    arguments.append(true);
    message.setArguments(arguments);
    QDBusConnection::systemBus().send(message);
}

void SystemOnboarding::on_hostname_textEdited(const QString &arg1)
{
    checkUserPage();
}
