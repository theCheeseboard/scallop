#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QMouseEvent>
#include <QX11Info>
#include <QDBusInterface>
#include <QCommandLinkButton>
#include <QListWidgetItem>
#include <QCheckBox>
#include "animatedstackedwidget.h"
#include "partitionwindow.h"
#include "erasedrivedialog.h"
#include "installerproc.h"
#include "branding.h"

#include <X11/Xlib.h>
#include <X11/XF86keysym.h>
#undef None
#undef Bool

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    enum languageOrder {
        enUS = 0,
        nlNL,
        viVN,
        daDK,
        esES,
        ruRU,
        svSE,
        ltLT,
        plPL,
        maxLanguage
    };

private slots:

    void on_nextButton_clicked();

    void on_backButton_clicked();

    void on_pages_currentChanged(int arg1);

    void on_driveSkip_clicked();

    void on_drivePartitionManager_clicked();

    void on_driveErase_clicked();

    void on_ExitInstallerDone_clicked();

    void on_RebootInstallerDone_clicked();

    void on_Password_textEdited(const QString &arg1);

    void on_PasswordConfirm_textEdited(const QString &arg1);

    void on_Hostname_textEdited(const QString &arg1);

    void on_installTSOS_clicked();

    void on_startTS_clicked();

    void on_startUtils_clicked();

    void on_OpenTerminal_clicked();

    void on_OpenFileManager_clicked();

    void on_OperPartition_clicked();

    void on_PowerOffButton_clicked();

    void on_RebootButton_clicked();

    void finishedInstallation();

    void on_FullName_textChanged(const QString &arg1);

    void on_UserName_textEdited(const QString &arg1);

    void on_retryErrorButton_clicked();

    void on_skipErrorButton_clicked();

    void on_startOverButton_clicked();

    void on_abortErrorButton_clicked();

    void installerError(QString error, bool canRetry, bool continuable);

    void retranslate();

    void updateLanguageList();

    void on_languageBox_currentRowChanged(int currentRow);

    void on_systemUpdatesCheckbox_toggled(bool checked);

    void on_networkwidget_networkAvailable(bool );

    private:
    Ui::MainWindow *ui;

    InstallerProc* installProcess;
    QProcess* rankProcess = NULL;
    QString mirrorlist;

    int paneBeforeError;

    QString currentLocale = "en_US";

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
};

#endif // MAINWINDOW_H
