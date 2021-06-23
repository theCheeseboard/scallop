QT += gui widgets thelib
TARGET = scallop

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Include the-libs build tools
include(/usr/share/the-libs/pri/gentranslations.pri)

SOURCES += \
    issueswidget.cpp \
    scalloplib.cpp

HEADERS += \
    issueswidget.h \
    scalloplib.h

FORMS += \
    issueswidget.ui

RESOURCES += \
    libscallop_resources.qrc
