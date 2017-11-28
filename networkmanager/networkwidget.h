#ifndef NETWORKWIDGET_H
#define NETWORKWIDGET_H

#include <QWidget>
#include <QDBusInterface>
#include <QFrame>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QDBusArgument>
#include <QTableWidget>
#include <QFileDialog>
#include <QTreeWidget>
#include <functional>
#include <QDebug>
#include "availablenetworkslist.h"
#include "savednetworkslist.h"
#include <ttoast.h>

namespace Ui {
class NetworkWidget;
}

class NetworkWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit NetworkWidget(QWidget *parent = 0);
        ~NetworkWidget();

    public slots:
        void flightModeChanged(bool flight);
        void stateChanged(uint state);
        bool hasConnection();

    private slots:
        void on_SecurityBackButton_clicked();

        void on_AvailableNetworksList_clicked(const QModelIndex &index);

        void on_SecurityConnectButton_clicked();

        void on_networksManualButton_clicked();

        void on_SecurityType_currentIndexChanged(int index);

        void on_EnterpriseAuthMethod_currentIndexChanged(int index);

        QString selectCertificate();

        void on_EnterpriseTLSUserCertificateSelect_clicked();

        void on_EnterpriseTLSCACertificateSelect_clicked();

        void on_EnterprisePEAPCaCertificateSelect_clicked();

    signals:
        void networkAvailable(bool available);

    private:
        Ui::NetworkWidget *ui;

        QDBusInterface* nmInterface = new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus());
        bool flightMode = false;
};

#endif // NETWORKWIDGET_H
