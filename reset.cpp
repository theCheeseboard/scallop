#include "reset.h"
#include "ui_reset.h"

extern Branding branding;
extern float getDPIScaling();

Reset::Reset(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Reset)
{
    ui->setupUi(this);

    ui->iconLabel->setPixmap(QIcon(":/icons/icon.svg").pixmap(32 * getDPIScaling(), 32 * getDPIScaling()));

    ui->osNameLabel->setText(branding.name);

    ui->lowerPane->setFixedHeight(0);
    ui->menubar->setFixedHeight(0);

    ui->resetEverything->setProperty("type", "destructive");
}

Reset::~Reset()
{
    delete ui;
}

void Reset::showFullScreen() {
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

void Reset::on_resetEverything_clicked()
{
    ui->pages->setCurrentIndex(1);
}


void Reset::on_backButton_clicked()
{
    switch (ui->pages->currentIndex()) {
        default:
            ui->pages->setCurrentIndex(ui->pages->currentIndex() - 1);
            break;
    }
}

void Reset::on_pages_currentChanged(int arg1)
{
    switch (arg1) {
        case 0:
            ui->backButton->setVisible(false);
            ui->nextButton->setVisible(false);
            break;
        case 1:
            ui->backButton->setVisible(true);
            ui->nextButton->setVisible(true);
            break;
    }
}
