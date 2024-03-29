QT       += core gui thelib frisbee svg multimedia network
SHARE_APP_NAME = scallop/install-system

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
TARGET = scallop-install-system

LIBS += -lKF5PulseAudioQt
INCLUDEPATH += /usr/include/KF5/KF5PulseAudioQt/PulseAudioQt

CONFIG += link_pkgconfig
PKGCONFIG += libnm

qtHaveModule(NetworkManagerQt) {
    QT += NetworkManagerQt
} else {
    INCLUDEPATH += /usr/include/KF5/NetworkManagerQt/
    LIBS += -lKF5NetworkManagerQt
}

include(../vars.pri)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cactus-install-animation/cactusanimationstage.cpp \
    cactus-install-animation/cactusinstallanimationwindow.cpp \
    cactus-install-animation/parallaxobject.cpp \
    cactus-install-animation/sequencer.cpp \
    cactus-install-animation/sequencer/animationelement.cpp \
    cactus-install-animation/sequencer/functionelement.cpp \
    cactus-install-animation/sequencer/loopelement.cpp \
    cactus-install-animation/sequencer/oneshotelement.cpp \
    cactus-install-animation/sequencer/parallelelement.cpp \
    cactus-install-animation/sequencer/pauseelement.cpp \
    cactus-install-animation/sequencer/randomelement.cpp \
    cactus-install-animation/sequencer/sequencerelement.cpp \
    cactus-install-animation/sequencer/sequentialelement.cpp \
    cactus-install-animation/sequencer/soundelement.cpp \
    cactus-install-animation/sequencer/textboxelement.cpp \
    cactus-install-animation/stages/animationstage1.cpp \
    cactus-install-animation/stages/animationstage2.cpp \
    cactus-install-animation/stages/animationstage3.cpp \
    cactus-install-animation/stages/animationstage4.cpp \
    cactus-install-animation/stages/animationstage5.cpp \
    cactus-install-animation/stages/animationstage6.cpp \
    cactus-install-animation/stages/animationstage7.cpp \
    cactus-install-animation/stages/animationstage8.cpp \
    cactus-install-animation/textbox.cpp \
    cactus-install-animation/zoomsvgrenderer.cpp \
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
    pages/networkpage.cpp \
    pages/progresspage.cpp \
    pages/readypage.cpp \
    pages/welcomepage.cpp \
    popovers/encryptpopover.cpp \
    popovers/eraseconfirmpopover.cpp \
    popovers/mountpointpopover.cpp \
    popovers/splitpopover.cpp \
    probe/probemanager.cpp

HEADERS += \
    cactus-install-animation/cactusanimationstage.h \
    cactus-install-animation/cactusinstallanimationwindow.h \
    cactus-install-animation/parallaxobject.h \
    cactus-install-animation/sequencer.h \
    cactus-install-animation/sequencer/animationelement.h \
    cactus-install-animation/sequencer/functionelement.h \
    cactus-install-animation/sequencer/loopelement.h \
    cactus-install-animation/sequencer/oneshotelement.h \
    cactus-install-animation/sequencer/parallelelement.h \
    cactus-install-animation/sequencer/pauseelement.h \
    cactus-install-animation/sequencer/randomelement.h \
    cactus-install-animation/sequencer/sequencerelement.h \
    cactus-install-animation/sequencer/sequentialelement.h \
    cactus-install-animation/sequencer/soundelement.h \
    cactus-install-animation/sequencer/textboxelement.h \
    cactus-install-animation/stages/animationstages.h \
    cactus-install-animation/textbox.h \
    cactus-install-animation/zoomsvgrenderer.h \
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
    pages/networkpage.h \
    pages/progresspage.h \
    pages/readypage.h \
    pages/welcomepage.h \
    popovers/encryptpopover.h \
    popovers/eraseconfirmpopover.h \
    popovers/mountpointpopover.h \
    popovers/splitpopover.h \
    probe/probemanager.h

FORMS += \
    cactus-install-animation/cactusinstallanimationwindow.ui \
    mainwidget.ui \
    mainwindow.ui \
    pages/advanceddiskpopover.ui \
    pages/diskpage.ui \
    pages/disktypepage.ui \
    pages/encryptpage.ui \
    pages/finishedpage.ui \
    pages/issuespage.ui \
    pages/networkpage.ui \
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
    cactus-animation-resources.qrc \
    resources.qrc
