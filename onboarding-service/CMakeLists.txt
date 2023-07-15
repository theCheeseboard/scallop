project(scalloponboardingservice VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus)
cntp_find_pkgconfig(X11 x11 REQUIRED)

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

target_link_libraries(scalloponboardingservice Qt::Core Qt::DBus PkgConfig::X11)

install(TARGETS scalloponboardingservice
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)
