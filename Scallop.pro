#-------------------------------------------------
#
# Project created by QtCreator 2017-03-10T17:01:23
#
#-------------------------------------------------

QT       += core gui thelib x11extras dbus
CONFIG   += c++14
LIBS     += -lX11 -lparted -lparted-fs-resize

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = scallop
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    animatedstackedwidget.cpp \
    partitionwindow.cpp \
    partitionframe.cpp \
    driveinfo.cpp \
    doingpartitioning.cpp \
    erasedrivedialog.cpp \
    installerproc.cpp \
    systemonboarding.cpp \
    reset.cpp \
    networkmanager/availablenetworkslist.cpp \
    networkmanager/networkwidget.cpp \
    networkmanager/savednetworkslist.cpp

HEADERS  += mainwindow.h \
    animatedstackedwidget.h \
    partitionwindow.h \
    partitionframe.h \
    driveinfo.h \
    doingpartitioning.h \
    erasedrivedialog.h \
    installerproc.h \
    branding.h \
    systemonboarding.h \
    reset.h \
    networkmanager/availablenetworkslist.h \
    networkmanager/networkwidget.h \
    networkmanager/savednetworkslist.h

FORMS    += mainwindow.ui \
    partitionwindow.ui \
    doingpartitioning.ui \
    erasedrivedialog.ui \
    systemonboarding.ui \
    reset.ui \
    networkmanager/networkwidget.ui

RESOURCES += \
    resources.qrc

TRANSLATIONS += translations/vi_VN.ts \
    translations/nl_NL.ts \
    translations/sv_SE.ts \
    translations/da_DK.ts \
    translations/lt_LT.ts

DISTFILES += \
    scallop-onboarding.service \
    scallop-reset.service

unix {
    target.path = /usr/bin/

    translations.files = translations/*
    translations.path = /usr/share/scallop/translations/

    systemd.files = scallop-onboarding.service scallop-reset.service
    systemd.path = /usr/lib/systemd/system/

    INSTALLS += target translations systemd
}
