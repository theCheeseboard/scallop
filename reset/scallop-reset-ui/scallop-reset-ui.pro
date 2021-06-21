QT       += core gui thelib network dbus
SHARE_APP_NAME = scallop/reset/ui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    downloadprogress.cpp \
    finalresetpopover.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    downloadprogress.h \
    finalresetpopover.h \
    mainwindow.h

FORMS += \
    downloadprogress.ui \
    finalresetpopover.ui \
    mainwindow.ui

equals(SCALLOP_ROOTFS_LOCATION, "") {
    SCALLOP_ROOTFS_LOCATION="https://packages.vicr123.com/cactus/rootfs/x86_64/latest.squashfs"
}

equals(SCALLOP_PACKAGED_LOCATION, "") {
    SCALLOP_PACKAGED_LOCATION="/opt/cactus-recovery-media/rootfs.squashfs"
}

equals(SCALLOP_VENDOR_WATERMARK_LOCATION, "") {
    SCALLOP_VENDOR_WATERMARK_LOCATION="/usr/share/plymouth/themes/bgrt-cactus/watermark.png"
}

unix:!macx {
    CONFIG += link_pkgconfig
    PKGCONFIG += polkit-qt5-1 polkit-qt5-agent-1

    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    DEFINES += SYSTEM_LIBRARY_DIRECTORY=\\\"$$THELIBS_INSTALL_LIB\\\"
    DEFINES += SCALLOP_ROOTFS_LOCATION=\\\"$$SCALLOP_ROOTFS_LOCATION\\\"
    DEFINES += SCALLOP_PACKAGED_LOCATION=\\\"$$SCALLOP_PACKAGED_LOCATION\\\"
    DEFINES += SCALLOP_VENDOR_WATERMARK_LOCATION=\\\"$$SCALLOP_VENDOR_WATERMARK_LOCATION\\\"

    target.path = $$THELIBS_INSTALL_BIN

    INSTALLS += target
}
