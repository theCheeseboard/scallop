project(scallopbootutil VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(libcontemporary REQUIRED)
find_package(libtdesktopenvironment REQUIRED)

set(SOURCES
    languagepopover.cpp
    main.cpp
    mainwindow.cpp
    powerpopover.cpp

    languagepopover.ui
    mainwindow.ui
    powerpopover.ui
)

set(HEADERS
    languagepopover.h
    mainwindow.h
    powerpopover.h
)


add_executable(scallopbootutil ${SOURCES} ${HEADERS})

set_target_properties(scallopbootutil PROPERTIES
        CNTP_DATA_SUBDIR "scallop/boot-utilities"
        OUTPUT_NAME "scallop-boot-utilities"
)

cntp_init(scallopbootutil 20)
cntp_translate(scallopbootutil)
cntp_target_name(scallopbootutil "Scallop Installer")

target_link_libraries(scallopbootutil Qt6::Widgets libcontemporary libtdesktopenvironment libscallop)
target_include_directories(scallopbootutil PUBLIC ../libscallop/)
target_compile_definitions(scallopbootutil
    PRIVATE SCALLOP_ROOTFS_LOCATION="${SCALLOP_ROOTFS_LOCATION}"
    PRIVATE SCALLOP_PACKAGED_LOCATION="${SCALLOP_PACKAGED_LOCATION}"
)

install(TARGETS scallopbootutil
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)

install(FILES scallop-boot-utilities.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/xsessions
)
