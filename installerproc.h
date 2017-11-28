#ifndef INSTALLERPROC_H
#define INSTALLERPROC_H

#include <QThread>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QEventLoop>
#include <unistd.h>
#include "branding.h"

class InstallerProc : public QThread
{
    Q_OBJECT
public:
    explicit InstallerProc(QObject *parent = 0);

    void run();
signals:
    void progressUpdate(QString text);
    void progressBarUpdate(int value, int maximum);
    void finished();
    void error(QString error, bool canRetry, bool continuable);

public slots:
    void setDisk(QString disk);
    void setUserInformation(QString fullName, QString userName, QString password, QString hostname);
    void setMirrorlist(QString mirrorlist);
    void setDoUpdates(bool doUpdates);

    void cont(bool retry);

private:
    QString disk;

    bool userInformationReady = false;
    bool doUpdates = true;
    QString fullName, userName, password, hostname, mirrorlist = "";
    QEventLoop* waiter;
};

#endif // INSTALLERPROC_H
