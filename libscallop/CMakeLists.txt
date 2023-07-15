project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(libcontemporary REQUIRED)

set(SOURCES
    issueswidget.cpp
    scalloplib.cpp
)

set(HEADERS
    issueswidget.h
    scalloplib.h

    issueswidget.ui
    libscallop_resources.qrc
)

add_library(libscallop STATIC ${SOURCES} ${HEADERS})
cntp_init(libscallop 20)
set_target_properties(libscallop PROPERTIES
        OUTPUT_NAME libscallop
        FRAMEWORK TRUE
        MACOSX_FRAMEWORK_IDENTIFIER com.vicr123.libscallop
        VERSION 1.0.0
        PUBLIC_HEADER "${HEADERS}")

target_link_libraries(libscallop Qt::Widgets libcontemporary)
target_compile_definitions(libscallop PRIVATE LIBSCALLOP_LIBRARY)
