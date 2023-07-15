project(scallopresetui VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network DBus)
find_package(libcontemporary REQUIRED)
find_package(PolkitQt6-1 REQUIRED)

set(SOURCES
    downloadprogress.cpp
    finalresetpopover.cpp
    main.cpp
    mainwindow.cpp

    downloadprogress.ui
    finalresetpopover.ui
    mainwindow.ui

    resources.qrc
)

set(HEADERS
    downloadprogress.h
    finalresetpopover.h
    mainwindow.h
)


add_executable(scallopresetui ${SOURCES} ${HEADERS})

set_target_properties(scallopresetui PROPERTIES
        CNTP_DATA_SUBDIR "scallop/reset/ui"
        OUTPUT_NAME "scallop-reset-ui"
)

cntp_init(scallopresetui 20)
cntp_translate(scallopresetui)
cntp_target_name(scallopresetui "Scallop Reset")

target_link_libraries(scallopresetui Qt::Widgets Qt::Network Qt::DBus PolkitQt6-1::Core PolkitQt6-1::Agent libcontemporary)
target_compile_definitions(scallopresetui
    PRIVATE SCALLOP_ROOTFS_LOCATION="${SCALLOP_ROOTFS_LOCATION}"
    PRIVATE SCALLOP_VENDOR_WATERMARK_LOCATION="${SCALLOP_VENDOR_WATERMARK_LOCATION}"
    PRIVATE SCALLOP_PACKAGED_LOCATION="${SCALLOP_PACKAGED_LOCATION}"
)

install(TARGETS scallopresetui
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)
