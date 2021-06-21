QT -= gui
QT += dbus
TARGET = scallop-offline-reset
SHARE_APP_NAME=scallop/reset/offline-reset

CONFIG += c++11 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        plymouth.cpp

unix:!macx {
    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    DEFINES += SYSTEM_LIBRARY_DIRECTORY=\\\"$$THELIBS_INSTALL_LIB\\\"

    target.path = /usr/lib

    sd.path = /usr/lib/systemd/system
    sd.files = $$files(systemd/**)

    INSTALLS += target sd
}

HEADERS += \
    plymouth.h
