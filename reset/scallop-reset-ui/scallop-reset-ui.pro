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

include(../../vars.pri)

unix:!macx {
    CONFIG += link_pkgconfig
    PKGCONFIG += polkit-qt5-1 polkit-qt5-agent-1

    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    DEFINES += SYSTEM_LIBRARY_DIRECTORY=\\\"$$THELIBS_INSTALL_LIB\\\"

    target.path = $$THELIBS_INSTALL_BIN

    INSTALLS += target
}

RESOURCES += \
    resources.qrc
