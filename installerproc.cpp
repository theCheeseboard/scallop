#include "installerproc.h"

extern bool isEFI();
extern Branding branding;

InstallerProc::InstallerProc(QObject *parent) : QThread(parent)
{

}

void InstallerProc::run() {
    qDebug() << "Starting installation on /mnt NOW.";

    waiter = new QEventLoop();

    { //Start unsquashing the filesystem.
        unsquash:
        progressUpdate(tr("Unsquashing filesystem to disk. This could take a while."));
        progressBarUpdate(0, 0);
        QProcess* unsquashProcess = new QProcess;
        connect(unsquashProcess, &QProcess::readyRead, [=] {
            //Read the last segment
            QString str = unsquashProcess->readAll();
            QString percentageStr = str.split(" ").last();
            int percentage = percentageStr.remove("%").toInt();
            emit progressBarUpdate(percentage, 100);
            qDebug() << str;
            qDebug() << percentage;
        });

        if (QFile("/run/archiso/copytoram/airootfs.sfs").exists()) {
            unsquashProcess->start("unsquashfs -f -d /mnt /run/archiso/copytoram/airootfs.sfs");
        } else {
            unsquashProcess->start("unsquashfs -f -d /mnt /run/archiso/bootmnt/arch/x86_64/airootfs.sfs");
        }
        unsquashProcess->waitForFinished(-1);
        sync();

        if (unsquashProcess->exitCode() != 0) {
            emit error(tr("Couldn't copy files over to the disk."), true, true);
            if (waiter->exec() == 0) { //Retry
                goto unsquash;
            }
        }
    }

    progressBarUpdate(0, 0);

    progressUpdate(tr("Configuring system..."));
    if (!QFile::exists("/installer/initramfs")) {
        emit error(tr("No initramfs found."), false, false);
        return;
    }

    QFile("/installer/initramfs").copy("/mnt/boot/vmlinuz-linux");

    { //Modify some files
        fstab:
        progressUpdate(tr("Generating an fstab..."));

        QProcess fstab;
        fstab.start("genfstab -p -U /mnt");
        fstab.waitForFinished();

        if (fstab.exitCode() != 0) {
            emit error(tr("Couldn't generate an fstab."), true, true);
            if (waiter->exec() == 0) { //Retry
                goto fstab;
            }
        }

        QFile fstabFile("/mnt/etc/fstab");
        if (!fstabFile.open(QFile::WriteOnly)) {
            emit error(tr("Couldn't write fstab."), true, true);
            if (waiter->exec() == 0) { //Retry
                goto fstab;
            }
        }
        fstabFile.write(fstab.readAllStandardOutput());
        fstabFile.close();

        progressUpdate(tr("Configuring system..."));
        QFile journalFile("/mnt/etc/systemd/journald.conf");
        if (!journalFile.open(QFile::ReadOnly)) {
            emit error("Couldn't edit journal configuration file.", true, true);
            if (waiter->exec() == 0) { //Retry
                goto fstab;
            }
        }
        QString journalContents(journalFile.readAll());
        journalContents.replace("Storage=volatile", "#Storage=auto");
        journalFile.close();
        if (!journalFile.open(QFile::WriteOnly)) {
            emit error("Couldn't edit journal configuration file.", true, true);
            if (waiter->exec() == 0) { //Retry
                goto fstab;
            }
        }
        journalFile.write(journalContents.toUtf8());
        journalFile.close();

        QFile("/mnt/etc/udev/rules.d/81-dhcpcd.rules").remove();
        QProcess::execute("systemctl disable pacman-init.service choose-mirror.service");
        QFile("/mnt/etc/systemd/system/choose-mirror.service").remove();
        QFile("/mnt/etc/systemd/system/pacman-init.service").remove();
        QFile("/mnt/etc/systemd/system/etc-pacman.d-gnupg.mount").remove();
        QFile("/mnt/etc/systemd/system/getty@tty1.service.d/autologin.conf").remove();
        QFile("/mnt/root/.automated_script.sh").remove();
        QFile("/mnt/root/.zlogin").remove();
        QFile("/mnt/etc/mkinitcpio-archiso.conf").remove();
        QFile("/mnt/etc/initcpio").remove();
    }

    {
        genlocale:
        progressUpdate("Generating locales...");
        QFile locale("/mnt/etc/locale.gen");
        locale.open(QFile::Append);
        locale.write("en_US.UTF-8 UTF-8\n");
        locale.write("en_US ISO-8859-1\n");
        locale.close();

        if (QProcess::execute("locale-gen") != 0) {
            emit error(tr("Couldn't generate locales."), true, true);
            if (waiter->exec() == 0) { //Retry
                goto genlocale;
            }
        }
    }

    {
        progressUpdate(tr("Configuring mkinitcpio..."));

        QFile initcpioFile("/mnt/etc/mkinitcpio.conf");
        initcpioFile.open(QFile::ReadOnly);
        QString initcpio = initcpioFile.readAll();
        initcpioFile.close();
        QString newFile;

        for (QString line : initcpio.split("\n")) {
            if (line.startsWith("HOOKS")) {
                newFile.append("HOOKS=\"base udev plymouth autodetect modconf block filesystems keyboard fsck\"\n");
            } else {
                newFile.append(line + "\n");
            }
        }

        initcpioFile.open(QFile::WriteOnly);
        initcpioFile.write(newFile.toUtf8());
        initcpioFile.close();
    }

    {
        progressUpdate(tr("Generating a new initramfs..."));
        if (QProcess::execute("arch-chroot /mnt mkinitcpio -p linux") != 0) {
            emit error(tr("Couldn't generate an initramfs."), true, true);
            if (waiter->exec() == 0) { //Retry
                goto genlocale;
            }
        }
    }

    {
        progressUpdate(tr("Configuring system..."));
        QFile chfnDefault("/mnt/etc/login.defs");
        chfnDefault.open(QFile::ReadOnly);
        QString chfnDefaults(chfnDefault.readAll());
        chfnDefault.close();

        QStringList chfnDefaultsArray = chfnDefaults.split("\n");
        for (QString line : chfnDefaultsArray) {
            if (line.startsWith("CHFN_RESTRICT")) {
                int index = chfnDefaultsArray.indexOf(line);
                chfnDefaultsArray.removeAt(index);
                chfnDefaultsArray.insert(index, "CHFN_RESTRICT           frwh");
            }
        }

        chfnDefaults = "";
        for (QString line : chfnDefaultsArray) {
            chfnDefaults.append(line + "\n");
        }

        chfnDefault.open(QFile::WriteOnly);
        chfnDefault.write(chfnDefaults.toUtf8());
        chfnDefault.close();
    }

    {
        bootloader:
        progressUpdate(tr("Installing bootloader..."));
        if (isEFI()) {
            if (QProcess::execute("arch-chroot /mnt grub-install --target=x86_64-efi --efi-directory=/boot/ --bootloader-id=tsos_grub") != 0) {
                emit error(tr("Couldn't install the bootloader."), true, true);
                if (waiter->exec() == 0) { //Retry
                    goto bootloader;
                }
            }
        } else {
            if (QProcess::execute("arch-chroot /mnt grub-install --target=i386-pc /dev/" + disk) != 0) {
                emit error(tr("Couldn't install the bootloader."), true, true);
                if (waiter->exec() == 0) { //Retry
                    goto bootloader;
                }
            }
        }

        QFile grubDefault("/mnt/etc/default/grub");
        grubDefault.open(QFile::ReadOnly);
        QString grubDefaults(grubDefault.readAll());
        grubDefault.close();

        QStringList grubDefaultsArray = grubDefaults.split("\n");
        for (QString line : grubDefaultsArray) {
            if (line.startsWith("GRUB_CMDLINE_LINUX_DEFAULT")) {
                int index = grubDefaultsArray.indexOf(line);
                grubDefaultsArray.removeAt(index);
                grubDefaultsArray.insert(index, "GRUB_CMDLINE_LINUX_DEFAULT=\"quiet splash\"");
            } else  if (line.startsWith("#GRUB_DISABLE_LINUX_UUID")) {
                int index = grubDefaultsArray.indexOf(line);
                grubDefaultsArray.removeAt(index);
                grubDefaultsArray.insert(index, "GRUB_DISABLE_LINUX_UUID=true");
            }
        }

        grubDefaults = "";
        for (QString line : grubDefaultsArray) {
            grubDefaults.append(line + "\n");
        }
        grubDefault.open(QFile::WriteOnly);
        grubDefault.write(grubDefaults.toUtf8());
        grubDefault.close();

        if (QProcess::execute("arch-chroot /mnt grub-mkconfig -o /boot/grub/grub.cfg") != 0) {
            emit error(tr("Couldn't create bootloader configuration."), true, true);
            if (waiter->exec() == 0) { //Retry
                goto bootloader;
            }
        }

        QFile grubConfig("/mnt/boot/grub/grub.cfg");
        grubConfig.open(QFile::ReadWrite);
        QString grubConfiguration(grubConfig.readAll());
        grubConfig.close();
        grubConfiguration = grubConfiguration.replace("Arch Linux", branding.name);
        grubConfig.open(QFile::ReadWrite);
        grubConfig.write(grubConfiguration.toUtf8());
        grubConfig.close();
    }

    if (!oemMode) {
        progressUpdate(tr("Waiting for input..."));
        while (!userInformationReady) {
            QThread::sleep(3);
        }

        {
            progressUpdate(tr("Configuring users..."));

            QProcess::execute("useradd -R /mnt -g wheel -m " + userName);
            QProcess::execute("arch-chroot /mnt chfn -f \"" + fullName + "\" " + userName);

            QProcess chpasswd;
            chpasswd.start("chpasswd -R /mnt");
            chpasswd.write(QString("root:" + password + "\n").toUtf8());
            chpasswd.write(QString(userName + ":" + password + "\n").toUtf8());
            chpasswd.closeWriteChannel();
            chpasswd.waitForFinished(-1);

            progressUpdate("Configuring Hostname...");
            QFile hostnameFile("/mnt/etc/hostname");
            hostnameFile.open(QFile::WriteOnly);
            hostnameFile.write(hostname.toUtf8());
            hostnameFile.close();
        }
    }

    {
        progressUpdate(tr("Configuring system..."));

        QFile sudoersConfig("/mnt/etc/sudoers");
        sudoersConfig.open(QFile::ReadOnly);
        QString sudoersConfiguration(sudoersConfig.readAll());
        sudoersConfig.close();
        sudoersConfiguration = sudoersConfiguration.replace("# %wheel ALL=(ALL) ALL", "%wheel ALL=(ALL) ALL");
        sudoersConfig.open(QFile::WriteOnly);
        sudoersConfig.write(sudoersConfiguration.toUtf8());
        sudoersConfig.close();
    }

    {
        progressUpdate(tr("Configuring services..."));
        QProcess::execute("arch-chroot /mnt systemctl enable NetworkManager");
        QProcess::execute("arch-chroot /mnt systemctl enable bluetooth");
        QProcess::execute("arch-chroot /mnt systemctl enable sddm-plymouth");
        if (oemMode) {
            QProcess::execute("arch-chroot /mnt systemctl enable scallop-onboarding");
        }
    }

    {
        progressUpdate(tr("Removing unneeded packages..."));
        //QProcess::execute("arch-chroot /mnt pacman -Rs --noconfirm scallop");
        QFile("/mnt/bin/install_theos").remove();
        QFile("/mnt/etc/scallop-live").remove();
    }

    {
        progressUpdate(tr("Populating pacman keyring..."));
        QProcess::execute("arch-chroot /mnt pacman-key --init");
        QProcess::execute("arch-chroot /mnt pacman-key --populate archlinux");
    }

    {
        progressUpdate(tr("Waiting for mirrorlist rankings to complete...\nDepending on your internet speed, this can take a while. Please wait..."));
        while (this->mirrorlist == "") {
            QThread::sleep(5);
        }

        QFile mirrorlist("/mnt/etc/pacman.d/mirrorlist");
        mirrorlist.open(QFile::ReadWrite);
        mirrorlist.write(this->mirrorlist.toUtf8());
        mirrorlist.close();
    }

    if (doUpdates) {
        QString currentStage = tr("Updating system\nSyncing package repositories...");

        QProcess* pacmanProc = new QProcess();
        connect(pacmanProc, &QProcess::readyRead, [=, &currentStage] {
            QString output = pacmanProc->readAll();
            QStringList lines = output.split("\n");
            lines.removeAll("");
            QString line = lines.last();
            progressUpdate(currentStage + "\n\n" + line);
        });
        pacmanProc->start("arch-chroot /mnt pacman -Sy --noconfirm --noprogressbar --color never");
        pacmanProc->waitForFinished(-1);

        if (pacmanProc->exitCode() == 0) {
            currentStage = tr("Updating system\nDownloading updates...");
            pacmanProc->start("arch-chroot /mnt pacman -Suw --noconfirm --noprogressbar --color never");
            pacmanProc->waitForFinished(-1);

            if (pacmanProc->exitCode() == 0) {
                currentStage = tr("Updating system\nInstalling updates...");
                pacmanProc->start("arch-chroot /mnt pacman -Su --noconfirm --noprogressbar --color never");
                pacmanProc->waitForFinished(-1);
            } else {
                //Updates could not be installed
                qDebug() << "Updates couldn't be installed";
            }
        } else {
            //Updates could not be installed
            qDebug() << "Updates couldn't be installed";
        }
    }

    progressUpdate(tr("Done!"));
    emit finished();
}

void InstallerProc::setDisk(QString disk) {
    this->disk = disk;
}

void InstallerProc::setUserInformation(QString fullName, QString userName, QString password, QString hostname) {
    this->fullName = fullName;
    this->userName = userName;
    this->password = password;
    this->hostname = hostname;
    userInformationReady = true;
}

void InstallerProc::setMirrorlist(QString mirrorlist) {
    this->mirrorlist = mirrorlist;
}

void InstallerProc::cont(bool retry) {
    if (retry) {
        waiter->exit(0);
    } else {
        waiter->exit(1);
    }
}

void InstallerProc::setDoUpdates(bool doUpdates) {
    this->doUpdates = doUpdates;
}

void InstallerProc::setOemMode(bool oemMode) {
    this->oemMode = oemMode;
}
