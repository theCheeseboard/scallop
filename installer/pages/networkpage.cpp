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
#include "networkpage.h"
#include "ui_networkpage.h"

#include <QFile>
#include "flowcontroller.h"
#include "installerdata.h"

#include <tmessagebox.h>
#include <tinputdialog.h>

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/Utils>

struct NetworkPagePrivate {
    NetworkManager::WirelessDevice::Ptr wlDev;
};

NetworkPage::NetworkPage(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::NetworkPage) {
    ui->setupUi(this);

    d = new NetworkPagePrivate();

    ui->titleLabel->setBackButtonShown(true);
    ui->descriptionLabel->setText(tr("An Internet connection is required to install %1. Connect a network cable or select a wireless network to continue.").arg(InstallerData::systemName()));

    FlowController::instance()->setSkipPage(this, [ = ] {
        if (QFile::exists(SCALLOP_PACKAGED_LOCATION)) return true;
        return false;
    });

    ui->connectedLabel->setVisible(false);
    ui->nextButton->setEnabled(false);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::connectivityChanged, this, [ = ](NetworkManager::Connectivity connectivity) {
        ui->nextButton->setEnabled(connectivity == NetworkManager::Connectivity::Full);
        ui->connectedLabel->setVisible(ui->nextButton->isEnabled());
    });
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(NetworkManager::checkConnectivity());
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [ = ] {
        ui->nextButton->setEnabled(watcher->reply().arguments().first().toInt() == NetworkManager::Connectivity::Full);
        ui->connectedLabel->setVisible(ui->nextButton->isEnabled());
        watcher->deleteLater();
    });

    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, &NetworkPage::updateWirelessDevice);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceRemoved, this, &NetworkPage::updateWirelessDevice);

    updateWirelessDevice();
}

NetworkPage::~NetworkPage() {
    delete d;
    delete ui;
}

void NetworkPage::on_nextButton_clicked() {
    FlowController::instance()->nextPage();
}

void NetworkPage::on_titleLabel_backButtonClicked() {
    FlowController::instance()->previousPage();
}

void NetworkPage::updateWirelessDevice() {
    d->wlDev.clear();

    for (NetworkManager::Device::Ptr device : NetworkManager::networkInterfaces()) {
        if (device->type() == NetworkManager::Device::Wifi) {
            d->wlDev = device.staticCast<NetworkManager::WirelessDevice>();
            d->wlDev->requestScan();

            connect(d->wlDev.data(), &NetworkManager::WirelessDevice::accessPointAppeared, this, &NetworkPage::updateAPs);
            connect(d->wlDev.data(), &NetworkManager::WirelessDevice::accessPointDisappeared, this, &NetworkPage::updateAPs);
            break;
        }
    }

    updateAPs();
}

void NetworkPage::updateAPs() {
    ui->networkList->clear();
    if (d->wlDev) {
        QStringList seenSsids;

        for (QString apName : d->wlDev->accessPoints()) {
            NetworkManager::AccessPoint::Ptr ap = d->wlDev->findAccessPoint(apName);
            if (seenSsids.contains(ap->ssid())) continue;

            QListWidgetItem* apItem = new QListWidgetItem();
            apItem->setText(ap->ssid());
            apItem->setData(Qt::UserRole, apName);
            ui->networkList->addItem(apItem);
            seenSsids.append(ap->ssid());
        }
    }
}

void NetworkPage::on_networkList_itemActivated(QListWidgetItem* item) {
    NetworkManager::AccessPoint::Ptr ap = d->wlDev->findAccessPoint(item->data(Qt::UserRole).toString());

    NetworkManager::WirelessSecurityType securityType = NetworkManager::findBestWirelessSecurity(d->wlDev->wirelessCapabilities(), true, false, ap->capabilities(), ap->wpaFlags(), ap->rsnFlags());

    QString key;
    switch (securityType) {
        case NetworkManager::NoneSecurity:
            //Nothing needs to be done here
            break;
        case NetworkManager::StaticWep:
        case NetworkManager::DynamicWep:
        case NetworkManager::WpaPsk:
        case NetworkManager::Wpa2Psk:
        case NetworkManager::SAE: {
            bool ok;
            key = tInputDialog::getText(this, tr("Security Key"), tr("Please input the security key for the network %1.").arg(QLocale().quoteString(ap->ssid())), QLineEdit::Password, "", &ok);
            if (!ok) return;
        }
        break;
        case NetworkManager::WpaEap:
        case NetworkManager::Wpa2Eap:
        case NetworkManager::Leap:
        case NetworkManager::UnknownSecurity:
        default: {
            tMessageBox* box = new tMessageBox(this);
            box->setWindowTitle(tr("Unsupported Security Settings"));
            box->setText(tr("To connect to this network, you'll need to use a terminal."));
            connect(box, &tMessageBox::finished, this, [ = ] {
                box->deleteLater();
            });
            box->open();
            break;
        }
    }


    NetworkManager::ConnectionSettings settings(NetworkManager::ConnectionSettings::Wireless);
    NetworkManager::WirelessSetting::Ptr wirelessSettings = settings.setting(NetworkManager::Setting::Wireless).staticCast<NetworkManager::WirelessSetting>();
    NetworkManager::WirelessSecuritySetting::Ptr securitySettings = settings.setting(NetworkManager::Setting::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();

    settings.setUuid(NetworkManager::ConnectionSettings::createNewUuid());

    wirelessSettings->setSsid(ap->ssid().toUtf8());

    switch (securityType) {
        case NetworkManager::NoneSecurity:
            //Don't need to set anything special here
            break;
        case NetworkManager::StaticWep:
        case NetworkManager::DynamicWep:
            securitySettings->setKeyMgmt(NetworkManager::WirelessSecuritySetting::Wep);
            securitySettings->setAuthAlg(NetworkManager::WirelessSecuritySetting::Shared);
            securitySettings->setWepKey0(key);
            securitySettings->setInitialized(true);
            break;
        case NetworkManager::WpaPsk:
        case NetworkManager::Wpa2Psk:
            securitySettings->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaPsk);
            securitySettings->setPsk(key);
            securitySettings->setInitialized(true);
            break;
        case NetworkManager::SAE:
            securitySettings->setKeyMgmt(NetworkManager::WirelessSecuritySetting::SAE);
            securitySettings->setPsk(key);
            securitySettings->setInitialized(true);
            break;
        case NetworkManager::WpaEap:
        case NetworkManager::Wpa2Eap:
        case NetworkManager::Leap:
        case NetworkManager::UnknownSecurity:
        default:
            return;
    }

    wirelessSettings->setInitialized(true);


    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(NetworkManager::addAndActivateConnection(settings.toMap(), d->wlDev->uni(), ""));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [ = ] {
        if (watcher->isError()) {
            tMessageBox* box = new tMessageBox(this);
            box->setWindowTitle(tr("Could not connect"));
            box->setText(tr("Could not connect to the network."));
            box->setInformativeText(tr("Ensure that the network security key is correct and that the device is not too far away from the access point."));
            connect(box, &tMessageBox::finished, this, [ = ] {
                box->deleteLater();
            });
            box->open();
        }
    });
}
