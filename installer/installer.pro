QT       += core gui thelib frisbee
SHARE_APP_NAME = scallop/install-system

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
TARGET = scallop-install-system

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    flowcontroller.cpp \
    install/bootloaderstate.cpp \
    install/diskmanagementstate.cpp \
    install/installmanager.cpp \
    install/mountstate.cpp \
    install/unsquashstate.cpp \
    installerdata.cpp \
    installipcmanager.cpp \
    main.cpp \
    mainwidget.cpp \
    mainwindow.cpp \
    pages/diskpage.cpp \
    pages/finishedpage.cpp \
    pages/issuespage.cpp \
    pages/progresspage.cpp \
    pages/readypage.cpp \
    pages/welcomepage.cpp \
    popovers/eraseconfirmpopover.cpp

HEADERS += \
    flowcontroller.h \
    install/bootloaderstate.h \
    install/diskmanagementstate.h \
    install/installmanager.h \
    install/mountstate.h \
    install/unsquashstate.h \
    installerdata.h \
    installipcmanager.h \
    mainwidget.h \
    mainwindow.h \
    pages/diskpage.h \
    pages/finishedpage.h \
    pages/issuespage.h \
    pages/progresspage.h \
    pages/readypage.h \
    pages/welcomepage.h \
    popovers/eraseconfirmpopover.h

FORMS += \
    mainwidget.ui \
    mainwindow.ui \
    pages/diskpage.ui \
    pages/finishedpage.ui \
    pages/issuespage.ui \
    pages/progresspage.ui \
    pages/readypage.ui \
    pages/welcomepage.ui \
    popovers/eraseconfirmpopover.ui


unix:!macx {
    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    DEFINES += SYSTEM_LIBRARY_DIRECTORY=\\\"$$[QT_INSTALL_LIBS]\\\"

    target.path = /usr/bin

#    icon.path = /usr/share/icons/hicolor/scalable/apps/
#    icon.files = icons/scallop.svg

    INSTALLS += target
}
