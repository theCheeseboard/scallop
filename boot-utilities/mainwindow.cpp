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

#include <QFile>
#include <QProcess>
#include <QPainter>
#include <QGraphicsOpacityEffect>
#include <Wm/desktopwm.h>
#include <Background/backgroundcontroller.h>
#include <tvariantanimation.h>

struct MainWindowPrivate {
    QGraphicsOpacityEffect* effect;
    BackgroundController* bg;
    QPixmap background;
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    d = new MainWindowPrivate();
    d->effect = new QGraphicsOpacityEffect();
    d->effect->setOpacity(1);
    ui->frame->setGraphicsEffect(d->effect);

    //Get distribution information
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
            if (value.startsWith("\"") && value.endsWith("\"")) value = value.mid(1, value.count() - 2);
            values.insert(key, value);
        }
        information.close();

        QString systemName = values.value("PRETTY_NAME", tr("Unknown"));
        ui->titleLabel->setText(tr("Welcome to %1!").arg(systemName));
        ui->installButton->setText(tr("Install %1").arg(systemName));
    }

    DesktopWm::setSystemWindow(this, DesktopWm::SystemWindowTypeDesktop);

    d->bg = new BackgroundController(BackgroundController::Desktop);
    connect(d->bg, &BackgroundController::currentBackgroundChanged, this, &MainWindow::updateBackground);
    updateBackground();
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::on_exitButton_clicked() {
    QApplication::exit();
}

void MainWindow::on_installButton_clicked() {
    QProcess* proc = new QProcess();
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [ = ](int exitCode, QProcess::ExitStatus exitStatus) {
        setUtilitiesAvailable(true);
    });
    proc->start("scallop-install-system", QStringList());

    setUtilitiesAvailable(false);
}

void MainWindow::setUtilitiesAvailable(bool utilitiesAvailable) {
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(d->effect->opacity());
    anim->setEndValue(utilitiesAvailable ? 1.0 : 0.0);
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, this, [ = ](QVariant value) {
        d->effect->setOpacity(value.toReal());
    });
    anim->start();
}

void MainWindow::updateBackground() {
    d->bg->getCurrentBackground(this->size())->then([ = ](BackgroundController::BackgroundData backgroundData) {
        d->background = backgroundData.px;
        this->update();
    });
}

void MainWindow::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.drawPixmap(QRect(0, 0, this->width(), this->height()), d->background);
}

void MainWindow::on_terminalButton_clicked() {
    QProcess::startDetached("theterminal", {});
}
