QT       += core gui thelib tdesktopenvironment
SHARE_APP_NAME = scallop/boot-utilities
TARGET = scallop-boot-utilities

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    languagepopover.cpp \
    main.cpp \
    mainwindow.cpp \
    powerpopover.cpp

HEADERS += \
    languagepopover.h \
    mainwindow.h \
    powerpopover.h

FORMS += \
    languagepopover.ui \
    mainwindow.ui \
    powerpopover.ui

unix:!macx {
    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    DEFINES += SYSTEM_LIBRARY_DIRECTORY=\\\"$$THELIBS_INSTALL_LIB\\\"

    target.path = /usr/bin

    desktop.files = scallop-boot-utilities.desktop
    desktop.path = /usr/share/xsessions

    INSTALLS += target desktop
}

DISTFILES += \
    scallop-boot-utilities.desktop
