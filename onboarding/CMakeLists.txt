project(scalloponboarding VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(libcontemporary REQUIRED)
find_package(libthedesk REQUIRED)
find_package(libtdesktopenvironment REQUIRED)
find_package(KF5PulseAudioQt REQUIRED)

set(SOURCES
    OnboardingPages/onboardingcompleteoobe.cpp
    OnboardingPages/onboardinghostname.cpp
    endsession.cpp
    main.cpp

    OnboardingPages/onboardingcompleteoobe.ui
    OnboardingPages/onboardinghostname.ui
    endsession.ui
)

set(HEADERS
    OnboardingPages/onboardingcompleteoobe.h
    OnboardingPages/onboardinghostname.h
    endsession.h
)


add_executable(scalloponboarding ${SOURCES} ${HEADERS})

set_target_properties(scalloponboarding PROPERTIES
        CNTP_DATA_SUBDIR "scallop/onboarding"
        OUTPUT_NAME "scallop-onboarding"
)

cntp_init(scalloponboarding 20)
cntp_translate(scalloponboarding)
cntp_target_name(scalloponboarding "Scallop Onboarding")
cntp_defaults_file(scalloponboarding
    DEFAULTS_FILE scalloponboarding.conf)

target_link_libraries(scalloponboarding Qt6::Widgets KF5::PulseAudioQt libcontemporary libtdesktopenvironment libthedesk)

install(TARGETS scalloponboarding
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)

install(FILES lightdm-configuration.conf
    DESTINATION ${CMAKE_INSTALL_DATADIR}/scallop/onboarding
)

install(FILES 50-scallop-onboarding.rules
    DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/rules.d
)
