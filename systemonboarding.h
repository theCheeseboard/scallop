#ifndef SYSTEMONBOARDING_H
#define SYSTEMONBOARDING_H

#include <QMainWindow>
#include <QLabel>
#include "animatedstackedwidget.h"
#include "tpropertyanimation.h"
#include <QTimer>
#include <QMenuBar>
#include <QPushButton>
#include <QJsonObject>
#include <QFile>
#include <QJsonArray>
#include <QProcess>
#include <QProgressBar>
#include "branding.h"

namespace Ui {
class SystemOnboarding;
}

class SystemOnboarding : public QMainWindow
{
    Q_OBJECT

public:
    explicit SystemOnboarding(QWidget *parent = 0);
    ~SystemOnboarding();

    void showFullScreen();

private slots:
    void on_nextButton_clicked();

    void on_backButton_clicked();

    void on_pages_currentChanged(int arg1);

    void on_networkwidget_networkAvailable(bool );

    void on_updateLater_clicked();

    void on_updateNow_clicked();

    private:
    Ui::SystemOnboarding *ui;

    QJsonObject timezoneData;
};

#endif // SYSTEMONBOARDING_H
