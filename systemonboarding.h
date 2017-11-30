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
#include <QListWidgetItem>
#include <QMessageBox>
#include <QLineEdit>
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

        void on_timezoneList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

        void on_username_textEdited(const QString &arg1);

        void on_fullName_textChanged(const QString &arg1);

        void on_timezoneCityList_currentRowChanged(int currentRow);

        void on_actionReboot_triggered();

        void on_actionPower_Off_triggered();

        void checkUserPage();

        void on_password_textChanged(const QString &arg1);

        void on_passwordConfirm_textChanged(const QString &arg1);

        void finalizeSettings();

        void finishOnboarding();

        void on_hostname_textEdited(const QString &arg1);

    signals:
        void updatesComplete();

    private:
        Ui::SystemOnboarding *ui;

        QJsonObject timezoneData;
        bool updating = false;
};

#endif // SYSTEMONBOARDING_H
