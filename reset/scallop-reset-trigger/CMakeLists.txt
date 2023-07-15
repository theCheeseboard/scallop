project(scallopresettrigger VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus)
find_package(PolkitQt6-1 REQUIRED)
include(GNUInstallDirs)

set(SOURCES
    main.cpp
)

set(HEADERS

)


add_executable(scallopresettrigger ${SOURCES} ${HEADERS})

set_target_properties(scallopresettrigger PROPERTIES
        CNTP_DATA_SUBDIR "scallop/reset/trigger"
        OUTPUT_NAME "scallop-reset-trigger"
)

cntp_init(scallopresettrigger 20)
cntp_translate(scallopresettrigger)
cntp_target_name(scallopresettrigger "Scallop Reset Trigger")

target_link_libraries(scallopresettrigger Qt::Core Qt::DBus PolkitQt6-1::Core PolkitQt6-1::Agent)
target_include_directories(scallopresettrigger PUBLIC ../libscallop/)
target_compile_definitions(scallopresettrigger
    PRIVATE SCALLOP_PACKAGED_LOCATION="${SCALLOP_PACKAGED_LOCATION}"
)

install(TARGETS scallopresettrigger
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)

install(FILES com.vicr123.scallop.reset.policy
    DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/actions
)
