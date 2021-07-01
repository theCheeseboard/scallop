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
#include "cactusinstallanimationwindow.h"
#include "ui_cactusinstallanimationwindow.h"

#include <QShortcut>
#include <tlogger.h>

#include "cactusanimationstage.h"
#include "installipcmanager.h"

#include "stages/animationstages.h"
#include "sequencer/soundelement.h"

#include <tscrim.h>
#include "pages/finishedpage.h"
#include <QTimer>

struct CactusInstallAnimationWindowPrivate {
    QList<CactusAnimationStage*> stages;
    int currentStage = 0;

    QWidget* drawWidget;
};

CactusInstallAnimationWindow::CactusInstallAnimationWindow(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::CactusInstallAnimationWindow) {
    ui->setupUi(this);
    d = new CactusInstallAnimationWindowPrivate();

    QShortcut* debugLogShortcut = new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_L), this);
    connect(debugLogShortcut, &QShortcut::activated, this, [ = ] {
        tLogger::openDebugLogWindow();
    });

    d->drawWidget = new QWidget(this);
    d->drawWidget->move(0, 0);
    d->drawWidget->resize(this->size());
    d->drawWidget->show();
    d->drawWidget->installEventFilter(this);
    d->drawWidget->lower();

    FinishedPage* finishedPage = new FinishedPage();
    finishedPage->setVisible(false);

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Lift);

    auto showFinishedPage = [ = ] {
//        tScrim::scrimForWidget(this)->setBlurEnabled(false);
        tScrim::scrimForWidget(this)->show();

        QFrame* frame = new QFrame(this);
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFixedSize(SC_DPI_T(QSize(800, 600), QSize));

        QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(finishedPage);
        frame->setLayout(layout);

        QRect geom;
        geom.setSize(frame->size());
        geom.moveCenter(QRect(QPoint(0, 0), this->size()).center());
        frame->setGeometry(geom);

        frame->setAutoFillBackground(true);
        frame->show();
        frame->raise();
        finishedPage->show();
    };

    ui->installDescription->setText(tr("Preparing for installation"));
    connect(InstallIpcManager::instance(), &InstallIpcManager::messageChanged, this, [ = ](QString message) {
        ui->installDescription->setText(message);
    });
    connect(InstallIpcManager::instance(), &InstallIpcManager::progressChanged, this, [ = ](int progress) {
        if (progress < 0) {
            ui->progressBar->setMaximum(0);
        } else {
            ui->progressBar->setMaximum(100);
            ui->progressBar->setValue(progress);
        }
    });
    connect(InstallIpcManager::instance(), &InstallIpcManager::success, this, [ = ] {
        //Cue the success animation
        ui->stackedWidget->setCurrentWidget(ui->installCompletePage);
    });
    connect(InstallIpcManager::instance(), &InstallIpcManager::failure, this, [ = ] {
        showFinishedPage();
    });

//    QTimer::singleShot(60000, InstallIpcManager::instance(), &InstallIpcManager::success);

    QPalette pal = this->palette();
    pal.setColor(QPalette::Window, Qt::black);
    pal.setColor(QPalette::WindowText, Qt::white);
    this->setPalette(pal);

    d->stages = {
        new AnimationStage1(),
        new AnimationStage2(),
        new AnimationStage3(),
        new AnimationStage4(),
        new AnimationStage5(),
        new AnimationStage6(),
        new AnimationStage7(),
        new AnimationStage8()
    };
    for (CactusAnimationStage* stage : d->stages) {
        connect(stage, &CactusAnimationStage::stageComplete, this, [ = ] {
            if (d->currentStage == d->stages.count() - 1) {
                showFinishedPage();
            } else {
                d->currentStage++;
                d->stages.at(d->currentStage)->start();
                this->update();
            }
        });
        connect(stage, &CactusAnimationStage::requestRender, this, [ = ] {
            this->update();
        });
    }

    tVariantAnimation* anim = new tVariantAnimation(this);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setDuration(500);
    connect(anim, &tVariantAnimation::valueChanged, this, [ = ](QVariant value) {
        this->setWindowOpacity(value.toDouble());
    });
    connect(anim, &tVariantAnimation::finished, this, [ = ] {
        anim->deleteLater();
        d->stages.first()->start();
    });
    anim->start();
    this->setWindowOpacity(0);
}

CactusInstallAnimationWindow::~CactusInstallAnimationWindow() {
    delete d;
    delete ui;
}

void CactusInstallAnimationWindow::on_rebootButton_clicked() {
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Reboot");
    message.setArguments({true});
    QDBusConnection::systemBus().call(message);
}


void CactusInstallAnimationWindow::on_powerOffButton_clicked() {
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "PowerOff");
    message.setArguments({true});
    QDBusConnection::systemBus().call(message);
}



bool CactusInstallAnimationWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == d->drawWidget && event->type() == QEvent::Paint) {
        QPainter painter(d->drawWidget);
        d->stages.at(d->currentStage)->render(&painter, d->drawWidget->size());
    }
    return QDialog::eventFilter(watched, event);
}

void CactusInstallAnimationWindow::resizeEvent(QResizeEvent* event) {
    d->drawWidget->resize(this->size());
}

void CactusInstallAnimationWindow::on_muteButton_toggled(bool checked) {
    SoundElement::setMute(checked);
}

