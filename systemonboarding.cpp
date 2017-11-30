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
    ui->nextButton->setText(tr("Next"));

    switch (arg1) {
        case 0: {
            ui->backButton->setVisible(false);
            ui->nextButton->setText(tr("Get Started"));
            break;
        }
        case 1: {
            ui->nextButton->setText(tr("Skip"));
            break;
        }
        case 2: {
            if (ui->updateCheckProgressBar->isVisible()) {
                ui->nextButton->setVisible(false);
            }
            break;
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
        });

        updateChecker->deleteLater();
    });

    ui->updateCheckButtons->setVisible(false);
    ui->updateCheckFrame->setVisible(true);
}
