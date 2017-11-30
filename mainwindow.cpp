#include "mainwindow.h"
#include "ui_mainwindow.h"

extern Branding branding;
extern float getDPIScaling();
extern QTranslator *qtTranslator, *tsTranslator;

QString calculateSize(quint64 size) {
    QString ret;
    if (size >= 1073741824) {
        ret = QString::number(((float) size / 1024 / 1024 / 1024), 'f', 2).append(" GiB");
    } else if (size >= 1048576) {
        ret = QString::number(((float) size / 1024 / 1024), 'f', 2).append(" MiB");
    } else if (size >= 1024) {
        ret = QString::number(((float) size / 1024), 'f', 2).append(" KiB");
    } else {
        ret = QString::number((float) size, 'f', 2).append(" B");
    }

    return ret;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //this->setFixedSize(this->size());
    ui->iconLabel->setPixmap(QIcon(":/icons/icon.svg").pixmap(32 * getDPIScaling(), 32 * getDPIScaling()));
    ui->tsLogo->setPixmap(QIcon(":/icons/icon.svg").pixmap(32 * getDPIScaling(), 32 * getDPIScaling()));
    ui->osHeader->setText(branding.name);
    ui->osFooter->setText(branding.name);

    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    ui->pages->setCurrentIndex(0);
    ui->backButton->setVisible(false);
    ui->nextButton->setVisible(false);
    ui->rankingMirrorsLabel->setVisible(false);
    ui->PowerOffButton->setProperty("type", "destructive");
    ui->RebootButton->setProperty("type", "destructive");

    //Immediately start ranking mirrors
    QFile::copy("/etc/pacman.d/mirrorlist", QDir::homePath() + "/.mirrors");
    QFile mirrorFile("/etc/pacman.d/mirrorlist");
    QFile tempFile(QDir::homePath() + "/.mirrors");
    mirrorFile.open(QFile::ReadOnly);
    tempFile.open(QFile::WriteOnly);
    while (!mirrorFile.atEnd()) {
        QByteArray line = mirrorFile.readLine();
        /*if (line.count() > 1) {
            line.remove(0, 1);
        }*/
        tempFile.write(line);
    }
    tempFile.close();
    mirrorFile.close();

    rankProcess = new QProcess();
    connect(rankProcess, &QProcess::readyRead, [=] {
        mirrorlist += rankProcess->readAll();
    });
    connect(rankProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), [=] {
        //Check mirrorlist for validity
        QString recreated;
        for (QString line : mirrorlist.split("\n")) {
            if (line.startsWith("#") || line.startsWith(("Server ="))) {
                recreated += line + "\n";
            }
        }

        installProcess->setMirrorlist(recreated);
        ui->rankingMirrorsLabel->setVisible(false);
    });

    QPushButton* closeButton = new QPushButton;
    closeButton->setIcon(QIcon::fromTheme("window-close"));
    closeButton->setParent(this);
    closeButton->setIconSize(QSize(24 * getDPIScaling(), 24 * getDPIScaling()));
    closeButton->setGeometry(this->geometry().right() - 38 * getDPIScaling(), 6 * getDPIScaling(), 32 * getDPIScaling(), 32 * getDPIScaling());
    closeButton->setFlat(true);
    connect(closeButton, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(ui->pages, &AnimatedStackedWidget::currentChanged, [=](int arg1) {
        if (arg1 == 4) {
            closeButton->setVisible(false);
        }
    });

    ui->label_13->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));

    if (QDBusConnection::sessionBus().interface()->registeredServiceNames().value().contains("org.thesuite.theshell")) {
        ui->startTS->setVisible(false);
    }

    installProcess = new InstallerProc(this);

    ui->driveSkip->setVisible(false);

    this->retranslate();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_nextButton_clicked()
{
    if (ui->pages->currentIndex() == 4 && oemMode) {
        ui->pages->setCurrentIndex(6);
        return;
    } else if (ui->pages->currentIndex() == 5) {
        if (ui->Password->text() != ui->PasswordConfirm->text()) {
            QMessageBox::warning(this, tr("Passwords don't match"), tr("The passwords don't match. Try again."));
            return;
        }
    }
    ui->pages->setCurrentIndex(ui->pages->currentIndex() + 1);
}

void MainWindow::on_backButton_clicked()
{
    if (ui->pages->currentIndex() == 9) {
        ui->pages->setCurrentIndex(0);
    } else if (ui->pages->currentIndex() == 2) {
        ui->pages->setCurrentIndex(0);
    } else {
        ui->pages->setCurrentIndex(ui->pages->currentIndex() - 1);
    }
}

void MainWindow::on_pages_currentChanged(int arg1)
{
    ui->nextButton->setText(tr("Next"));
    switch (arg1) {
        case 0: {
            ui->backButton->setVisible(false);
            ui->nextButton->setVisible(false);
            break;
        }

        case 1: {
            ui->backButton->setVisible(true);
            ui->nextButton->setVisible(true);
            ui->backButton->setEnabled(true);
            ui->nextButton->setEnabled(true);
            ui->nextButton->setText(tr("Skip"));
        }

        case 2: {
            ui->backButton->setVisible(true);
            ui->nextButton->setVisible(true);
            ui->backButton->setEnabled(true);
            ui->nextButton->setEnabled(true);

            QDBusInterface powerInterface("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.UPower", QDBusConnection::systemBus());
            if (powerInterface.property("OnBattery").toBool()) {
                ui->acPane->setStyleSheet("background-color: rgb(200, 100, 0)");
                ui->acPaneCheck->setPixmap(QIcon::fromTheme("go-previous").pixmap(16, 16));
                ui->acPaneOK->setText(tr("Hey!"));
            } else {
                ui->acPane->setStyleSheet("background-color: rgb(0, 100, 0)");
                ui->acPaneCheck->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(16, 16));
                ui->acPaneOK->setText(tr("OK!"));
            }

            if (ui->networkwidget->hasConnection()) {
                ui->networkPane->setStyleSheet("background-color: rgb(0, 100, 0)");
                ui->networkPaneCheck->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(16, 16));
                ui->networkPaneOK->setText(tr("OK!"));
                ui->systemUpdatesCheckbox->setEnabled(true);
                ui->systemUpdatesCheckbox->setChecked(true);
            } else {
                ui->networkPane->setStyleSheet("background-color: rgb(200, 100, 0)");
                ui->networkPaneCheck->setPixmap(QIcon::fromTheme("go-previous").pixmap(16, 16));
                ui->networkPaneOK->setText(tr("Hey!"));
                ui->systemUpdatesCheckbox->setEnabled(false);
                ui->systemUpdatesCheckbox->setChecked(false);
            }

            break;
        }

        case 3: {
            //Start ranking mirrorlist
            if (rankProcess->state() == QProcess::Running) {
                rankProcess->terminate();
                rankProcess->waitForFinished();
            }

            mirrorlist = "";
            rankProcess->start("rankmirrors \"" + QDir::homePath() + "/.mirrors\"");
            ui->rankingMirrorsLabel->setVisible(true);

            //Populate the list of drives
            ui->driveBox->clear();
            QProcess lsblk;
            lsblk.start("lsblk -rb");
            lsblk.waitForFinished();

            QByteArray output = lsblk.readAllStandardOutput();
            for (QString part : QString(output).split("\n")) {
                if (part != "") {
                    QStringList parse = part.split(" ");
                    if (parse.first().startsWith("sd") && parse.first().length() == 3) {
                        ui->driveBox->addItem("/dev/" + parse.first() + " (" + calculateSize(parse.at(3).toLongLong()) + ")", parse.first());
                    }
                }
            }

            ui->backButton->setEnabled(true);
            ui->nextButton->setEnabled(false);
            break;
        }

        case 4: {
            ui->backButton->setEnabled(true);
            ui->nextButton->setEnabled(true);
            break;
        }

        case 5: { //Start the installation on /mnt!
            installProcess->setDisk(ui->driveBox->currentData().toString());
            installProcess->setOemMode(this->oemMode);
            connect(installProcess, &InstallerProc::progressUpdate, [=](QString text) {
                ui->statusLabel->setText(text);
            });
            connect(installProcess, &InstallerProc::progressBarUpdate, [=](int value, int maximum) {
                ui->progressBar->setMaximum(maximum);
                ui->progressBar->setValue(value);
            });
            connect(installProcess, SIGNAL(finished()), this, SLOT(finishedInstallation()));
            connect(installProcess, SIGNAL(error(QString,bool,bool)), this, SLOT(installerError(QString,bool,bool)));
            installProcess->start();
            ui->backButton->setEnabled(false);
            ui->nextButton->setEnabled(false);
            break;
        }

        case 6: {
            installProcess->setUserInformation(ui->FullName->text(), ui->UserName->text(), ui->Password->text(), ui->Hostname->text());
            ui->backButton->setVisible(false);
            ui->nextButton->setVisible(false);
            break;
        }

        case 8: {
            ui->backButton->setVisible(false);
            ui->nextButton->setVisible(false);
            break;
        }

        case 9: {
            ui->backButton->setVisible(true);
            ui->nextButton->setVisible(false);
            break;
        }
    }
}

void MainWindow::mousePressEvent(QMouseEvent *mevent) {
    if (mevent->button() == Qt::LeftButton) {
        XUngrabPointer(QX11Info::display(), QX11Info::appTime());

        XEvent event;

        event.xclient.type = ClientMessage;
        event.xclient.message_type = XInternAtom(QX11Info::display(), "_NET_WM_MOVERESIZE", False);
        event.xclient.window = this->winId();
        event.xclient.format = 32;
        event.xclient.data.l[0] = mevent->globalX();
        event.xclient.data.l[1] = mevent->globalY();
        event.xclient.data.l[2] = 8;
        event.xclient.data.l[3] = Button1;
        event.xclient.data.l[4] = 0;

        XSendEvent(QX11Info::display(), DefaultRootWindow(QX11Info::display()), False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *mevent) {
    if (mevent->button() == Qt::LeftButton) {
        XEvent event;

        event.xclient.type = ClientMessage;
        event.xclient.serial = 0;
        event.xclient.send_event = True;
        event.xclient.message_type = XInternAtom(QX11Info::display(), "_NET_WM_MOVERESIZE", False);
        event.xclient.window = this->winId();
        event.xclient.format = 32;
        event.xclient.data.l[0] = 0;
        event.xclient.data.l[1] = 0;
        event.xclient.data.l[2] = 11;
        event.xclient.data.l[3] = Button1;
        event.xclient.data.l[4] = 2;

        XSendEvent(QX11Info::display(), DefaultRootWindow(QX11Info::display()), False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
    }
}

void MainWindow::on_driveSkip_clicked()
{
    ui->pages->setCurrentIndex(ui->pages->currentIndex() + 1);
}

void MainWindow::on_drivePartitionManager_clicked()
{
    PartitionWindow* win = new PartitionWindow(ui->driveBox->currentData().toString());
    if (win->exec() == QDialog::Accepted) {
        ui->pages->setCurrentIndex(ui->pages->currentIndex() + 1);
    }
}

void MainWindow::on_driveErase_clicked()
{
    EraseDriveDialog* dialog = new EraseDriveDialog(ui->driveBox->currentData().toString(), this);
    if (dialog->exec() == QDialog::Accepted) {
        ui->pages->setCurrentIndex(ui->pages->currentIndex() + 1);
    }
}

void MainWindow::on_ExitInstallerDone_clicked()
{
    this->close();
}

void MainWindow::on_RebootInstallerDone_clicked()
{
    QList<QVariant> arguments;
    arguments.append(true);

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Reboot");
    message.setArguments(arguments);
    QDBusConnection::systemBus().send(message);
}

void MainWindow::on_Password_textEdited(const QString &arg1)
{
    if (ui->PasswordConfirm->text() != ui->Password->text()) {
        ui->nextButton->setEnabled(false);
    } else {
        ui->nextButton->setEnabled(true);
    }
}

void MainWindow::on_PasswordConfirm_textEdited(const QString &arg1)
{
    if (ui->PasswordConfirm->text() != ui->Password->text()) {
        ui->nextButton->setEnabled(false);
    } else {
        ui->nextButton->setEnabled(true);
    }
}

void MainWindow::on_Hostname_textEdited(const QString &arg1)
{
    if (arg1 == "" || arg1.contains(" ")) {
        ui->nextButton->setEnabled(false);
    } else {
        ui->nextButton->setEnabled(true);
    }
}

void MainWindow::on_installTSOS_clicked()
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        //Install in OEM Mode
        tToast* toast = new tToast();
        toast->setTitle("OEM Mode");
        toast->setText("Installing theShell OS in OEM Mode");
        connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
        toast->show(this);
        oemMode = true;
    } else {
        oemMode = false;
    }

    //Check internet connection
    if (ui->networkwidget->hasConnection()) {
        ui->pages->setCurrentIndex(2);
    } else {
        ui->pages->setCurrentIndex(1);
    }
}

void MainWindow::on_startTS_clicked()
{
    this->hide();
    QProcess::execute("theshell");
    this->show();
}

void MainWindow::on_startUtils_clicked()
{
    ui->pages->setCurrentIndex(9);
}


void MainWindow::on_OpenTerminal_clicked()
{
    QProcess::startDetached("theterminal");
}

void MainWindow::on_OpenFileManager_clicked()
{
    QProcess::startDetached("thefile");
}

void MainWindow::on_OperPartition_clicked()
{
    QProcess::startDetached("partitionmanager");
}

void MainWindow::on_PowerOffButton_clicked()
{
    QList<QVariant> arguments;
    arguments.append(true);

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "PowerOff");
    message.setArguments(arguments);
    QDBusConnection::systemBus().send(message);
}

void MainWindow::on_RebootButton_clicked()
{
    qDebug() << "Reboot";
    QList<QVariant> arguments;
    arguments.append(true);

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Reboot");
    message.setArguments(arguments);
    QDBusConnection::systemBus().send(message);
}

void MainWindow::finishedInstallation() {
    ui->pages->setCurrentIndex(7);
}

void MainWindow::on_FullName_textChanged(const QString &arg1)
{
    if (arg1.contains(" ")) {
        ui->UserName->setText(arg1.left(arg1.indexOf(" ")).toLower());
    } else {
        ui->UserName->setText(arg1.toLower());
    }
}

void MainWindow::on_UserName_textEdited(const QString &arg1)
{
    ui->UserName->setText(arg1.toLower());
}

void MainWindow::on_retryErrorButton_clicked()
{
    QMetaObject::invokeMethod(installProcess, "cont", Q_ARG(bool, true));
    ui->pages->setCurrentIndex(paneBeforeError);
}

void MainWindow::on_skipErrorButton_clicked()
{
    QMetaObject::invokeMethod(installProcess, "cont", Q_ARG(bool, false));
    ui->pages->setCurrentIndex(paneBeforeError);
}
void MainWindow::on_startOverButton_clicked()
{
    this->hide();
    QProcess::execute("install_theos");
    QApplication::exit();
}

void MainWindow::on_abortErrorButton_clicked()
{
    this->close();
}

void MainWindow::installerError(QString error, bool canRetry, bool continuable) {
    //ui->retryErrorButton->setVisible(canRetry);
    //ui->skipErrorButton->setVisible(continuable);
    ui->retryErrorButton->setVisible(false);
    ui->skipErrorButton->setVisible(false);
    ui->errorDetails->setText(error);

    paneBeforeError = ui->pages->currentIndex();
    ui->pages->setCurrentIndex(8);
}

void MainWindow::retranslate() {
    ui->retranslateUi(this);
    this->setWindowTitle(tr("%1 installer").arg(branding.name));
    ui->InstallTitleLabel->setText(tr("Install %1").arg(branding.name));
    ui->InstallExplainLabel->setText(tr("Click \"Next\" to start installing %1").arg(branding.name));
    ui->InstallingTitleLabel->setText(tr("Installing %1...").arg(branding.name));
    ui->InstallingExplainLabel->setText(tr("Thanks for being patient. It'll take a while to install %1. Even if the bar is not moving, the installation is still running.").arg(branding.name));
    ui->SuccessExplainLabel->setText(tr("%1 was successfully installed. What would you like to do now?").arg(branding.name));
    ui->welcomeLabel->setText(tr("Welcome to %1! What would you like to do now?").arg(branding.name));
    ui->installTSOS->setText(tr("Install %1 onto your disk").arg(branding.name));
    ui->installTSOS->setDescription(tr("Install %1 onto your disk either by itself or next to another operating system.").arg(branding.name));
    ui->startTS->setText(tr("Try %1").arg(branding.name));
    ui->driveExplainLabel->setText(tr("How do you want to install %1?").arg(branding.name));
    ui->RebootInstallerDone->setDescription(tr("Stop running %1 from the live media and start using it from the hard disk.").arg(branding.name));
    ui->ExitInstallerDone->setDescription(tr("Exit the installer, but continue using %1 from the live media. Until you reboot, no data will be saved to the disk.").arg(branding.name));
    ui->startOverButton->setDescription(tr("Start installation of %1 again").arg(branding.name));
    ui->abortErrorButton->setDescription(tr("Quit installing %1 altogether. This may leave you with an unbootable system").arg(branding.name));

    updateLanguageList();
}

void MainWindow::updateLanguageList() {
    //Clear the box
    ui->languageBox->clear();

    //Block signals
    ui->languageBox->blockSignals(true);

    //For the time being, we'll just have hardcoded locales. This should change soon (hopefully)
    for (int i = 0; i < maxLanguage; i++) {
        switch (i) {
            case enUS:
                ui->languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-us"), tr("English") + " (English)"));
                break;
            case viVN:
                ui->languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-vn"), tr("Vietnamese") + " (Tiếng Việt)"));
                break;
            case daDK:
                ui->languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-dk"), tr("Danish") + " (Dansk)"));
                break;
            case nlNL:
                ui->languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-nl"), tr("Dutch") + " (Nederlands)"));
                break;
            case esES:
                ui->languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-es"), tr("Spanish") + " (Español)"));
                break;
            case ruRU:
                ui->languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-ru"), tr("Russian") + " (русский)"));
                break;
            case svSE:
                ui->languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-se"), tr("Swedish") + " (Svenska)"));
                break;
            case ltLT:
                ui->languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-lt"), tr("Lithuanian") + " (Lietuviškai)"));
                break;
            case plPL:
                ui->languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-pl"), tr("Polish") + " (Polski)"));
                break;
        }
    }

    if (currentLocale == "en_US") {
        ui->languageBox->setCurrentRow(enUS);
    } else if (currentLocale == "vi_VN") {
        ui->languageBox->setCurrentRow(viVN);
    } else if (currentLocale == "da_DK") {
        ui->languageBox->setCurrentRow(daDK);
    } else if (currentLocale == "nl_NL") {
        ui->languageBox->setCurrentRow(nlNL);
    } else if (currentLocale == "ru_RU") {
        ui->languageBox->setCurrentRow(ruRU);
    } else if (currentLocale == "sv_SE") {
        ui->languageBox->setCurrentRow(svSE);
    } else if (currentLocale == "lt_LT") {
        ui->languageBox->setCurrentRow(ltLT);
    }

    //Unblock signals
    ui->languageBox->blockSignals(false);
}

void MainWindow::on_languageBox_currentRowChanged(int currentRow)
{

    switch (currentRow) {
        case enUS:
            currentLocale = "en_US";
            break;
        case viVN:
            currentLocale = "vi_VN";
            break;
        case daDK:
            currentLocale =  "da_DK";
            break;
        case nlNL:
            currentLocale =  "nl_NL";
            break;
        case esES:
            currentLocale = "es_ES";
            break;
        case ruRU:
            currentLocale = "ru_RU";
            break;
        case svSE:
            currentLocale = "sv_SE";
            break;
        case ltLT:
            currentLocale = "lt_LT";
            break;
    }

    qputenv("LANG", currentLocale.toUtf8());

    QLocale defaultLocale(currentLocale);
    QLocale::setDefault(defaultLocale);

    if (defaultLocale.language() == QLocale::Arabic || defaultLocale.language() == QLocale::Hebrew) {
        //Reverse the layout direction
        QApplication::setLayoutDirection(Qt::RightToLeft);
    } else {
        //Set normal layout direction
        QApplication::setLayoutDirection(Qt::LeftToRight);
    }

    qtTranslator->load("qt_" + defaultLocale.name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    QApplication::installTranslator(qtTranslator);

    tsTranslator->load(defaultLocale.name(), "/usr/share/scallop/translations");
    QApplication::installTranslator(tsTranslator);

    retranslate();
}

void MainWindow::on_systemUpdatesCheckbox_toggled(bool checked)
{
    installProcess->setDoUpdates(checked);
}

void MainWindow::on_networkwidget_networkAvailable(bool available)
{
    if (available && ui->pages->currentIndex() == 1) {
        ui->pages->setCurrentIndex(2);
    }
}
