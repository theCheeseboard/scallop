#include "networkwidget.h"
#include "ui_networkwidget.h"

extern float getDPIScaling();

NetworkWidget::NetworkWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NetworkWidget)
{
    ui->setupUi(this);

    QDBusConnection::systemBus().connect(nmInterface->service(), nmInterface->path(), nmInterface->interface(), "DeviceAdded", this, SLOT(updateDevices()));
    QDBusConnection::systemBus().connect(nmInterface->service(), nmInterface->path(), nmInterface->interface(), "DeviceRemoved", this, SLOT(updateDevices()));
    QDBusConnection::systemBus().connect(nmInterface->service(), nmInterface->path(), nmInterface->interface(), "StateChanged", this, SLOT(stateChanged(uint)));

    ui->AvailableNetworksList->setItemDelegate(new AvailableNetworksListDelegate());

    //Set first wireless device
    QList<QDBusObjectPath> devices = nmInterface->property("AllDevices").value<QList<QDBusObjectPath>>();
    for (QDBusObjectPath device : devices) {
        QDBusInterface deviceInterface(nmInterface->service(), device.path(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus());
        if (deviceInterface.property("DeviceType").toUInt() == Wifi) {
            ui->AvailableNetworksList->setModel(new AvailableNetworksList(device));
            break;
        }
    }
}

NetworkWidget::~NetworkWidget()
{
    delete ui;
}

void NetworkWidget::stateChanged(uint state) {
    if (state == 70 || state == 60 || state == 50) {
        emit networkAvailable(true);
    } else {
        emit networkAvailable(false);
    }
}

void NetworkWidget::flightModeChanged(bool flight) {
    flightMode = flight;
}

void NetworkWidget::on_SecurityBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void NetworkWidget::on_AvailableNetworksList_clicked(const QModelIndex &index)
{
    //Determine if we need secrets for this network
    QString ssid = index.data(Qt::DisplayRole).toString();
    AvailableNetworksList::AccessPoint ap = index.data(Qt::UserRole).value<AvailableNetworksList::AccessPoint>();

    QDBusInterface settings(nmInterface->service(), "/org/freedesktop/NetworkManager/Settings", "org.freedesktop.NetworkManager.Settings", QDBusConnection::systemBus());
    QList<QDBusObjectPath> connectionSettings = settings.property("Connections").value<QList<QDBusObjectPath>>();
    QList<QDBusObjectPath> availableSettings;

    for (QDBusObjectPath settingsPath : connectionSettings) {
        //QDBusInterface settingsInterface(nmInterface->service(), settingsPath.path(), "org.freedesktop.NetworkManager.Settings.Connection");
        QDBusMessage msg = QDBusMessage::createMethodCall(nmInterface->service(), settingsPath.path(), "org.freedesktop.NetworkManager.Settings.Connection", "GetSettings");
        QDBusMessage msgReply = QDBusConnection::systemBus().call(msg);

        if (msgReply.arguments().count() != 0) {
            QMap<QString, QVariantMap> settings;

            QDBusArgument arg1 = msgReply.arguments().first().value<QDBusArgument>();
            arg1 >> settings;

            for (QString key : settings.keys()) {
                if (key == "802-11-wireless") {
                    QVariantMap wireless = settings.value("802-11-wireless");
                    if (wireless.value("ssid") == ssid) {
                        availableSettings.append(settingsPath);
                    }
                }
            }
        }
    }

    //Try to connect using all matching settings
    if (availableSettings.count() == 0) {
        ui->SecuritySsidEdit->setText(ssid);
        ui->SecuritySsidEdit->setVisible(false);

        switch (ap.security) {
            case NoSecurity:
                ui->SecurityType->setCurrentIndex(0);
                ui->securityDescriptionLabel->setText(tr("Connect to %1?").arg(ssid));
                break;
            case Leap:
            case StaticWep:
                ui->SecurityType->setCurrentIndex(1);
                ui->securityDescriptionLabel->setText(tr("To connect to %1, you'll need to provide a key.").arg(ssid));
                break;
            case DynamicWep:
                ui->SecurityType->setCurrentIndex(2);
                ui->securityDescriptionLabel->setText(tr("To connect to %1, you'll need to provide a key.").arg(ssid));
                break;
            case WpaPsk:
            case Wpa2Psk:
                ui->SecurityType->setCurrentIndex(3);
                ui->securityDescriptionLabel->setText(tr("To connect to %1, you'll need to provide a key.").arg(ssid));
                break;
            case WpaEnterprise:
            case Wpa2Enterprise:
                ui->SecurityType->setCurrentIndex(4);
                ui->securityDescriptionLabel->setText(tr("To connect to %1, you'll need to provide authentication details.").arg(ssid));
                break;
        }
        ui->SecurityType->setVisible(false);

        ui->stackedWidget->setCurrentIndex(1);
    } else {
        tToast* toast = new tToast();
        toast->setTitle(tr("Wi-Fi"));
        toast->setText(tr("Connecting to %1...").arg(ssid));
        connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
        toast->show(this->window());
        ui->stackedWidget->setCurrentIndex(0);

        bool success = false;

        for (QDBusObjectPath settingsPath : availableSettings) {
            //Connect to the network
            QDBusPendingCall pending = nmInterface->asyncCall("ActivateConnection", QVariant::fromValue(settingsPath), QVariant::fromValue(((AvailableNetworksList*) index.model())->devicePath()), QVariant::fromValue(ap.path));

            QEventLoop* loop = new QEventLoop();

            QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(pending);
            connect(watcher, &QDBusPendingCallWatcher::finished, [=] {
                if (pending.isError()) {
                    loop->exit(1);
                } else {
                    loop->exit(0);
                }
                watcher->deleteLater();
            });

            if (loop->exec() == 0) {
                loop->deleteLater();
                success = true;
                break;
            }
            loop->deleteLater();
        }

        if (!success) {

        }
    }
}

QList<QTreeWidgetItem*> getInfoChildren(QVariantMap parent) {
    QList<QTreeWidgetItem*> items;
    for (QString key : parent.keys()) {
        QVariant val = parent.value(key);
        QTreeWidgetItem* item = new QTreeWidgetItem();

        if (val.type() == QVariant::String) {
            item->setText(0, key);
            item->setText(1, val.toString());
        } else {
            item->setText(0, key);
            item->addChildren(getInfoChildren(val.toMap()));
        }
        items.append(item);
    }
    return items;
}

void NetworkWidget::on_SecurityConnectButton_clicked()
{
    QMap<QString, QVariantMap> settings;

    QVariantMap connection;
    connection.insert("type", "802-11-wireless");
    settings.insert("connection", connection);

    QVariantMap wireless;
    wireless.insert("ssid", ui->SecuritySsidEdit->text().toUtf8());
    wireless.insert("mode", "infrastructure");

    if (ui->SecuritySsidEdit->isVisible()) {
        wireless.insert("hidden", true);
    }

    QVariantMap security;
    switch (ui->SecurityType->currentIndex()) {
        case 0: //No security
            security.insert("key-mgmt", "none");
            break;
        case 1: //Static WEP
            security.insert("key-mgmt", "none");
            security.insert("auth-alg", "shared");
            security.insert("wep-key0", ui->securityKey->text());
            break;
        case 2: //Dynamic WEP
            security.insert("key-mgmt", "none");
            security.insert("auth-alg", "shared");
            security.insert("wep-key0", ui->securityKey->text());
            break;
        case 3: //WPA(2)-PSK
            security.insert("key-mgmt", "wpa-psk");
            security.insert("psk", ui->securityKey->text());
            break;
        case 4: { //WPA(2)-Enterprise
            QVariantMap enterpriseSettings;
            security.insert("key-mgmt", "wpa-eap");
            enterpriseSettings.insert("eap", QStringList() << "ttls");

            switch (ui->EnterpriseAuthMethod->currentIndex()) {
                case 0: //TLS
                    enterpriseSettings.insert("eap", QStringList() << "tls");
                    enterpriseSettings.insert("identity", ui->EnterpriseTLSIdentity->text());
                    enterpriseSettings.insert("client-cert", QUrl::fromLocalFile(ui->EnterpriseTLSUserCertificate->text()).toEncoded());
                    enterpriseSettings.insert("ca-cert", QUrl::fromLocalFile(ui->EnterpriseTLSCACertificate->text()).toEncoded());
                    enterpriseSettings.insert("subject-match", ui->EnterpriseTLSSubjectMatch->text());
                    enterpriseSettings.insert("altsubject-matches", ui->EnterpriseTLSAlternateSubjectMatch->text().split(","));
                    enterpriseSettings.insert("private-key", QUrl::fromLocalFile(ui->EnterpriseTLSPrivateKey->text()).toEncoded());
                    enterpriseSettings.insert("private-key-password", ui->EnterpriseTLSPrivateKeyPassword->text());
                    break;
                case 1: //LEAP
                    enterpriseSettings.insert("eap", QStringList() << "leap");
                    enterpriseSettings.insert("identity", ui->EnterpriseLEAPUsername->text());
                    enterpriseSettings.insert("password", ui->EnterpriseLEAPPassword->text());
                    break;
                case 2: { //FAST
                    enterpriseSettings.insert("eap", QStringList() << "fast");
                    enterpriseSettings.insert("anonymous-identity", ui->EnterpriseFASTAnonymousIdentity->text());
                    enterpriseSettings.insert("pac-file", ui->EnterpriseFASTPacFile->text());

                    int provisioning = 0;
                    if (ui->EnterpriseFASTPacProvisioningAnonymous->isChecked()) provisioning++;
                    if (ui->EnterpriseFASTPacProvisioningAuthenticated->isChecked()) provisioning += 2;
                    enterpriseSettings.insert("phase1-fast-provisioning", QString::number(provisioning));

                    if (ui->EnterpriseFASTPhase2Auth->currentIndex() == 0) { //GTC
                        enterpriseSettings.insert("phase2-auth", "gtc");
                    } else if (ui->EnterpriseFASTPhase2Auth->currentIndex() == 1) { //MSCHAPv2
                        enterpriseSettings.insert("phase2-auth", "mschapv2");
                    }

                    enterpriseSettings.insert("identity", ui->EnterpriseFASTUsername->text());
                    enterpriseSettings.insert("password", ui->EnterpriseFASTPassword->text());

                    break;
                }
                case 4: //PEAP
                    enterpriseSettings.insert("eap", QStringList() << "peap");

                    if (ui->EnterprisePEAPVer0->isChecked()) { //Force version 0
                        enterpriseSettings.insert("phase1-peapver", "0");
                    } else if (ui->EnterprisePEAPVer1->isChecked()) { //Force version 1
                        enterpriseSettings.insert("phase1-peapver", "1");
                    }

                    //fall through
                case 3: //TTLS
                    enterpriseSettings.insert("anonymous-identity", ui->EnterprisePEAPAnonymousIdentity->text());
                    enterpriseSettings.insert("client-cert", QUrl::fromLocalFile(ui->EnterprisePEAPCaCertificate->text()).toEncoded());

                    if (ui->EnterprisePEAPPhase2Auth->currentIndex() == 0) { //MSCHAPv2
                        enterpriseSettings.insert("phase2-auth", "mschapv2");
                    } else if (ui->EnterprisePEAPPhase2Auth->currentIndex() == 1) { //MD5
                        enterpriseSettings.insert("phase2-auth", "md5");
                    } else if (ui->EnterprisePEAPPhase2Auth->currentIndex() == 2) { //GTC
                        enterpriseSettings.insert("phase2-auth", "gtc");
                    }

                    enterpriseSettings.insert("identity", ui->EnterprisePEAPUsername->text());
                    enterpriseSettings.insert("password", ui->EnterprisePEAPPassword->text());
                    break;
            }

            settings.insert("802-1x", enterpriseSettings);
            break;
        }
    }
    settings.insert("802-11-wireless", wireless);
    settings.insert("802-11-wireless-security", security);

    QDBusPendingCall pendingCall = nmInterface->asyncCall("AddAndActivateConnection", QVariant::fromValue(settings), QVariant::fromValue(((AvailableNetworksList*) ui->AvailableNetworksList->model())->devicePath()), QVariant::fromValue(QDBusObjectPath("/")));

    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(pendingCall);
    connect(watcher, &QDBusPendingCallWatcher::finished, [=] {
        watcher->deleteLater();
        if (pendingCall.isError()) {
            tToast* toast = new tToast();
            toast->setTitle(tr("Connection Error"));
            toast->setText(pendingCall.error().message());
            toast->setTimeout(10000);
            connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
            toast->show(this->window());
        }
    });

    tToast* toast = new tToast();
    toast->setTitle(tr("Wi-Fi"));
    toast->setText(tr("Connecting to %1...").arg(ui->SecuritySsidEdit->text()));
    connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
    toast->show(this->window());
    ui->stackedWidget->setCurrentIndex(0);
}

void NetworkWidget::on_networksManualButton_clicked()
{
    ui->SecuritySsidEdit->setVisible(true);
    ui->SecurityType->setVisible(true);
    ui->securityDescriptionLabel->setText(tr("Enter the information to connect to a new network"));
    ui->stackedWidget->setCurrentIndex(1);
}

void NetworkWidget::on_SecurityType_currentIndexChanged(int index)
{
    switch (index) {
        case 0: //No security
            ui->SecurityKeysStack->setCurrentIndex(0);
            break;
        case 1: //Static WEP
            ui->SecurityKeysStack->setCurrentIndex(1);
            break;
        case 2: //Dynamic WEP
            ui->SecurityKeysStack->setCurrentIndex(1);
            break;
        case 3: //WPA(2)-PSK
            ui->SecurityKeysStack->setCurrentIndex(1);
            break;
        case 4: //WPA(2) Enterprise
            ui->SecurityKeysStack->setCurrentIndex(2);
            break;
    }
}

void NetworkWidget::on_EnterpriseAuthMethod_currentIndexChanged(int index)
{
    if (index == 4) {
        ui->peapVersionButtons->setVisible(true);
        ui->peapVersionLabel->setVisible(true);
        ui->WpaEnterpriseAuthDetails->setCurrentIndex(3);
    } else {
        ui->peapVersionButtons->setVisible(false);
        ui->peapVersionLabel->setVisible(false);
        ui->WpaEnterpriseAuthDetails->setCurrentIndex(index);
    }
}

QString NetworkWidget::selectCertificate() {
    QFileDialog* dialog = new QFileDialog(this);
    dialog->setNameFilter("Certificates (*.der *.pem *.crt *.cer)");
    if (dialog->exec() == QFileDialog::Accepted) {
        dialog->deleteLater();
        return dialog->selectedFiles().first();
    } else {
        dialog->deleteLater();
        return "";
    }
}
void NetworkWidget::on_EnterpriseTLSUserCertificateSelect_clicked()
{
    ui->EnterpriseTLSUserCertificate->setText(selectCertificate());
}

void NetworkWidget::on_EnterpriseTLSCACertificateSelect_clicked()
{
    ui->EnterpriseTLSCACertificate->setText(selectCertificate());
}

void NetworkWidget::on_EnterprisePEAPCaCertificateSelect_clicked()
{
    ui->EnterprisePEAPCaCertificate->setText(selectCertificate());
}

bool NetworkWidget::hasConnection() {
    int state = nmInterface->property("State").toInt();
    if (state == 70 || state == 60 || state == 50) {
        return true;
    } else {
        return false;
    }
}
