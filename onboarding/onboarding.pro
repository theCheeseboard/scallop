QT       += core gui thelib
TARGET = scallop-onboarding
SHARE_APP_NAME = scallop/onboarding

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    OnboardingPages/onboardingcompleteoobe.cpp \
    OnboardingPages/onboardinghostname.cpp \
    endsession.cpp \
    main.cpp

HEADERS += \
    OnboardingPages/onboardingcompleteoobe.h \
    OnboardingPages/onboardinghostname.h \
    endsession.h

FORMS += \
    OnboardingPages/onboardingcompleteoobe.ui \
    OnboardingPages/onboardinghostname.ui \
    endsession.ui

unix:!macx {
    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    LIBS += -lthedesk
    INCLUDEPATH += $$[QT_INSTALL_HEADERS]/libthedesk

    target.path = /usr/bin

    lightdmconfig.path = /usr/share/scallop/onboarding/
    lightdmconfig.files = lightdm-configuration.conf

    defaults.files = defaults.conf
    defaults.path = /etc/scallop/onboarding

    polkit.files = 50-scallop-onboarding.rules
    polkit.path = /usr/share/polkit-1/rules.d/

    INSTALLS += target system defaults polkit
}


DISTFILES += \
    50-scallop-onboarding.rules \
    defaults.conf \
    lightdm-configuration.conf \
    scallop-onboarding.desktop

RESOURCES +=
