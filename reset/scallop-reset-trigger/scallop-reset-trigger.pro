QT -= gui
QT += dbus
SHARE_APP_NAME=scallop/reset/trigger

CONFIG += c++11 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp

equals(SCALLOP_PACKAGED_LOCATION, "") {
    SCALLOP_PACKAGED_LOCATION="/opt/cactus-recovery-media/rootfs.squashfs"
}

unix:!macx {
    CONFIG += link_pkgconfig
    PKGCONFIG += polkit-qt5-1 polkit-qt5-agent-1

    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    DEFINES += SYSTEM_LIBRARY_DIRECTORY=\\\"$$THELIBS_INSTALL_LIB\\\"
    DEFINES += SCALLOP_PACKAGED_LOCATION=\\\"$$SCALLOP_PACKAGED_LOCATION\\\"

    target.path = $$THELIBS_INSTALL_BIN

    polkit.path = $$THELIBS_INSTALL_PREFIX/share/polkit-1/actions
    polkit.files = com.vicr123.scallop.reset.policy

    setuid.path = /
    setuid.extra += chmod u+s $(INSTALL_ROOT)/$$THELIBS_INSTALL_BIN/$$TARGET

    INSTALLS += target setuid polkit
}

DISTFILES += \
    com.vicr123.scallop.reset.policy
