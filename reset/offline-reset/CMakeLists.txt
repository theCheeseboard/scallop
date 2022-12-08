project(scallopofflinereset VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core DBus)

set(SOURCES
    main.cpp
    plymouth.cpp
)

set(HEADERS
    plymouth.h
)


add_executable(scallopofflinereset ${SOURCES} ${HEADERS})

set_target_properties(scallopofflinereset PROPERTIES
        CNTP_DATA_SUBDIR "scallop/reset/offline-reset"
        OUTPUT_NAME "scallop-offline-reset"
)

cntp_init(scallopofflinereset 20)
cntp_translate(scallopofflinereset)
cntp_target_name(scallopofflinereset "Scallop Reset Trigger")

target_link_libraries(scallopofflinereset Qt6::Core Qt6::DBus)

install(TARGETS scallopofflinereset
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)

install(DIRECTORY systemd/
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system
)
