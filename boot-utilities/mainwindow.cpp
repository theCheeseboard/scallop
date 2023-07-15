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
#include "ui_mainwindow.h"

#include "languagepopover.h"
#include "powerpopover.h"
#include <Background/backgroundcontroller.h>
#include <QFile>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QProcess>
#include <TimeDate/desktoptimedate.h>
#include <Wm/desktopwm.h>
#include <tapplication.h>
#include <tpopover.h>
#include <tvariantanimation.h>

struct MainWindowPrivate {
        QGraphicsOpacityEffect* effect;
        QGraphicsOpacityEffect* exitEffect;
        BackgroundController* bg;
        QPixmap background;

        QTranslator* localTranslator;
};

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    d = new MainWindowPrivate();

    d->effect = new QGraphicsOpacityEffect();
    d->effect->setOpacity(1);
    ui->frame->setGraphicsEffect(d->effect);

    d->exitEffect = new QGraphicsOpacityEffect();
    d->exitEffect->setOpacity(1);
    ui->exitButton->setGraphicsEffect(d->exitEffect);

    d->localTranslator = new QTranslator();
    updateTranslations(QLocale());

    QPalette pal = ui->bar->palette();
    pal.setColor(QPalette::WindowText, Qt::white);
    ui->bar->setPalette(pal);

    DesktopTimeDate::makeTimeLabel(ui->clock, DesktopTimeDate::Time);
    DesktopTimeDate::makeTimeLabel(ui->date, DesktopTimeDate::StandardDate);

    updateLabels();

    DesktopWm::setSystemWindow(this, DesktopWm::SystemWindowTypeDesktop);

    d->bg = new BackgroundController(BackgroundController::Desktop);
    connect(d->bg, &BackgroundController::currentBackgroundChanged, this, &MainWindow::updateBackground);
    updateBackground();

    ui->centralwidget->installEventFilter(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_exitButton_clicked() {
    PowerPopover* powerPopover = new PowerPopover();
    tPopover* popover = new tPopover(powerPopover);
    popover->setPopoverWidth(SC_DPI(400));
    connect(powerPopover, &PowerPopover::done, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, powerPopover, &PowerPopover::deleteLater);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    popover->show(this);
    powerPopover->setFocus();
}

void MainWindow::on_installButton_clicked() {
    QProcess* proc = new QProcess();
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
        setUtilitiesAvailable(true);
    });
    proc->start("scallop-installer", QStringList());

    setUtilitiesAvailable(false);
}

void MainWindow::setUtilitiesAvailable(bool utilitiesAvailable) {
    ui->frame->setVisible(true);
    ui->exitButton->setVisible(true);

    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(d->effect->opacity());
    anim->setEndValue(utilitiesAvailable ? 1.0 : 0.0);
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        d->effect->setOpacity(value.toReal());
        d->exitEffect->setOpacity(value.toReal());
    });
    connect(anim, &tVariantAnimation::finished, this, [=] {
        ui->frame->setVisible(utilitiesAvailable);
        ui->exitButton->setVisible(utilitiesAvailable);
    });
    anim->start();
}

QCoro::Task<> MainWindow::updateBackground() {
    auto backgroundData = co_await d->bg->getCurrentBackground(this->size());
    d->background = backgroundData.px;
    ui->centralwidget->update();
}

void MainWindow::updateLabels() {
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
            if (value.startsWith("\"") && value.endsWith("\"")) value = value.mid(1, value.size() - 2);
            values.insert(key, value);
        }
        information.close();

        QString systemName = values.value("PRETTY_NAME", tr("Unknown"));
        ui->titleLabel->setText(tr("Welcome to %1!").arg(systemName));
        ui->installButton->setText(tr("Install %1").arg(systemName));
    }
}

void MainWindow::updateTranslations(QLocale locale) {
    d->localTranslator->load(locale.name(), tApplication::shareDirs().at(0) + "/translations");
    tApplication::installTranslator(d->localTranslator);
}

void MainWindow::on_terminalButton_clicked() {
    QProcess::startDetached("theterminal", {});
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::Paint) {
        QPainter painter(ui->centralwidget);
        painter.drawPixmap(QRect(0, 0, this->width(), this->height()), d->background);

        QLinearGradient grad;
        grad.setStart(0, 0);
        grad.setFinalStop(0, ui->bar->height() * 2);
        grad.setColorAt(0, QColor(0, 0, 0, 127));
        grad.setColorAt(1, QColor(0, 0, 0, 0));

        painter.setPen(Qt::transparent);
        painter.setBrush(grad);
        painter.drawRect(0, 0, this->width(), this->height());
    }
    return false;
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    updateBackground();
}

void MainWindow::on_disksButton_clicked() {
    QProcess::startDetached("thefrisbee", {});
}

void MainWindow::on_viewFilesButton_clicked() {
    QProcess::startDetached("thefile", {});
}

void MainWindow::on_languageButton_clicked() {
    LanguagePopover* powerPopover = new LanguagePopover();
    tPopover* popover = new tPopover(powerPopover);
    popover->setPopoverWidth(SC_DPI(400));
    connect(powerPopover, &LanguagePopover::rejected, popover, &tPopover::dismiss);
    connect(powerPopover, &LanguagePopover::accepted, popover, [=](QLocale locale) {
        QLocale::setDefault(locale);

        QString localeName = locale.name() + ".UTF-8";
        qputenv("LANG", localeName.toUtf8());
        qputenv("LANGUAGE", localeName.toUtf8());
        static_cast<tApplication*>(tApplication::instance())->installTranslators();

        updateTranslations(locale);

        popover->dismiss();
    });
    connect(popover, &tPopover::dismissed, powerPopover, &LanguagePopover::deleteLater);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    popover->show(this);
    powerPopover->setFocus();
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        updateLabels();
    }
}
