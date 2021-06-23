QT       += core gui thelib frisbee
SHARE_APP_NAME = scallop/install-system

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
TARGET = scallop-install-system

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    diskmodel.cpp \
    flowcontroller.cpp \
    install/bootloaderstate.cpp \
    install/diskmanagementstate.cpp \
    install/fstabstate.cpp \
    install/installmanager.cpp \
    install/mountstate.cpp \
    install/unsquashstate.cpp \
    installerdata.cpp \
    installipcmanager.cpp \
    main.cpp \
    mainwidget.cpp \
    mainwindow.cpp \
    pages/advanceddiskpopover.cpp \
    pages/diskpage.cpp \
    pages/disktypepage.cpp \
    pages/encryptpage.cpp \
    pages/finishedpage.cpp \
    pages/issuespage.cpp \
    pages/progresspage.cpp \
    pages/readypage.cpp \
    pages/welcomepage.cpp \
    popovers/encryptpopover.cpp \
    popovers/eraseconfirmpopover.cpp \
    popovers/mountpointpopover.cpp \
    popovers/splitpopover.cpp \
    probe/probemanager.cpp

HEADERS += \
    diskmodel.h \
    flowcontroller.h \
    install/bootloaderstate.h \
    install/diskmanagementstate.h \
    install/fstabstate.h \
    install/installmanager.h \
    install/mountstate.h \
    install/unsquashstate.h \
    installerdata.h \
    installipcmanager.h \
    mainwidget.h \
    mainwindow.h \
    pages/advanceddiskpopover.h \
    pages/diskpage.h \
    pages/disktypepage.h \
    pages/encryptpage.h \
    pages/finishedpage.h \
    pages/issuespage.h \
    pages/progresspage.h \
    pages/readypage.h \
    pages/welcomepage.h \
    popovers/encryptpopover.h \
    popovers/eraseconfirmpopover.h \
    popovers/mountpointpopover.h \
    popovers/splitpopover.h \
    probe/probemanager.h

FORMS += \
    mainwidget.ui \
    mainwindow.ui \
    pages/advanceddiskpopover.ui \
    pages/diskpage.ui \
    pages/disktypepage.ui \
    pages/encryptpage.ui \
    pages/finishedpage.ui \
    pages/issuespage.ui \
    pages/progresspage.ui \
    pages/readypage.ui \
    pages/welcomepage.ui \
    popovers/encryptpopover.ui \
    popovers/eraseconfirmpopover.ui \
    popovers/mountpointpopover.ui \
    popovers/splitpopover.ui


unix:!macx {
    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    LIBS += -L$$OUT_PWD/../libscallop/ -lscallop
    PRE_TARGETDEPS += $$OUT_PWD/../libscallop/libscallop.a

    INCLUDEPATH += $$PWD/../libscallop
    DEPENDPATH += $$PWD/../libscallop

    DEFINES += SYSTEM_LIBRARY_DIRECTORY=\\\"$$THELIBS_INSTALL_LIB\\\"

    target.path = /usr/bin

    icon.path = /usr/share/icons/hicolor/scalable/apps/
    icon.files = icons/scallop-installer.svg

    sdbootconf.path = /usr/share/scallop/install-system
    sdbootconf.files = systemd-boot-config/

    INSTALLS += target icon sdbootconf
}

RESOURCES += \
    resources.qrc
