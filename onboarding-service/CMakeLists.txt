project(scalloponboardingservice VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core DBus)

set(SOURCES
    main.cpp
)

set(HEADERS

)


add_executable(scalloponboardingservice ${SOURCES} ${HEADERS})

set_target_properties(scalloponboardingservice PROPERTIES
        OUTPUT_NAME "scallop-onboarding-service"
)

cntp_init(scalloponboardingservice 20)

target_link_libraries(scalloponboardingservice Qt6::Core Qt6::DBus)

install(TARGETS scalloponboardingservice
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)
