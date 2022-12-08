project(scallopinstaller VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia Svg Network StateMachine)
find_package(libcontemporary REQUIRED)
find_package(libthefrisbee REQUIRED)
find_package(KF5PulseAudioQt REQUIRED)
find_package(KF5NetworkManagerQt REQUIRED)

set(SOURCES
    cactus-install-animation/cactusanimationstage.cpp
    cactus-install-animation/cactusinstallanimationwindow.cpp
    cactus-install-animation/parallaxobject.cpp
    cactus-install-animation/sequencer.cpp
    cactus-install-animation/sequencer/animationelement.cpp
    cactus-install-animation/sequencer/functionelement.cpp
    cactus-install-animation/sequencer/loopelement.cpp
    cactus-install-animation/sequencer/oneshotelement.cpp
    cactus-install-animation/sequencer/parallelelement.cpp
    cactus-install-animation/sequencer/pauseelement.cpp
    cactus-install-animation/sequencer/randomelement.cpp
    cactus-install-animation/sequencer/sequencerelement.cpp
    cactus-install-animation/sequencer/sequentialelement.cpp
    cactus-install-animation/sequencer/soundelement.cpp
    cactus-install-animation/sequencer/textboxelement.cpp
    cactus-install-animation/stages/animationstage1.cpp
    cactus-install-animation/stages/animationstage2.cpp
    cactus-install-animation/stages/animationstage3.cpp
    cactus-install-animation/stages/animationstage4.cpp
    cactus-install-animation/stages/animationstage5.cpp
    cactus-install-animation/stages/animationstage6.cpp
    cactus-install-animation/stages/animationstage7.cpp
    cactus-install-animation/stages/animationstage8.cpp
    cactus-install-animation/textbox.cpp
    cactus-install-animation/zoomsvgrenderer.cpp
    diskmodel.cpp
    flowcontroller.cpp
    install/bootloaderstate.cpp
    install/diskmanagementstate.cpp
    install/fstabstate.cpp
    install/installmanager.cpp
    install/mountstate.cpp
    install/unsquashstate.cpp
    installerdata.cpp
    installipcmanager.cpp
    main.cpp
    mainwidget.cpp
    mainwindow.cpp
    pages/advanceddiskpopover.cpp
    pages/diskpage.cpp
    pages/disktypepage.cpp
    pages/encryptpage.cpp
    pages/finishedpage.cpp
    pages/issuespage.cpp
    pages/networkpage.cpp
    pages/progresspage.cpp
    pages/readypage.cpp
    pages/welcomepage.cpp
    popovers/encryptpopover.cpp
    popovers/eraseconfirmpopover.cpp
    popovers/mountpointpopover.cpp
    popovers/splitpopover.cpp
    probe/probemanager.cpp

    cactus-install-animation/cactusinstallanimationwindow.ui
    mainwidget.ui
    mainwindow.ui
    pages/advanceddiskpopover.ui
    pages/diskpage.ui
    pages/disktypepage.ui
    pages/encryptpage.ui
    pages/finishedpage.ui
    pages/issuespage.ui
    pages/networkpage.ui
    pages/progresspage.ui
    pages/readypage.ui
    pages/welcomepage.ui
    popovers/encryptpopover.ui
    popovers/eraseconfirmpopover.ui
    popovers/mountpointpopover.ui
    popovers/splitpopover.ui

    cactus-animation-resources.qrc
    resources.qrc
)

set(HEADERS
    cactus-install-animation/cactusanimationstage.h
    cactus-install-animation/cactusinstallanimationwindow.h
    cactus-install-animation/parallaxobject.h
    cactus-install-animation/sequencer.h
    cactus-install-animation/sequencer/animationelement.h
    cactus-install-animation/sequencer/functionelement.h
    cactus-install-animation/sequencer/loopelement.h
    cactus-install-animation/sequencer/oneshotelement.h
    cactus-install-animation/sequencer/parallelelement.h
    cactus-install-animation/sequencer/pauseelement.h
    cactus-install-animation/sequencer/randomelement.h
    cactus-install-animation/sequencer/sequencerelement.h
    cactus-install-animation/sequencer/sequentialelement.h
    cactus-install-animation/sequencer/soundelement.h
    cactus-install-animation/sequencer/textboxelement.h
    cactus-install-animation/stages/animationstages.h
    cactus-install-animation/textbox.h
    cactus-install-animation/zoomsvgrenderer.h
    diskmodel.h
    flowcontroller.h
    install/bootloaderstate.h
    install/diskmanagementstate.h
    install/fstabstate.h
    install/installmanager.h
    install/mountstate.h
    install/unsquashstate.h
    installerdata.h
    installipcmanager.h
    mainwidget.h
    mainwindow.h
    pages/advanceddiskpopover.h
    pages/diskpage.h
    pages/disktypepage.h
    pages/encryptpage.h
    pages/finishedpage.h
    pages/issuespage.h
    pages/networkpage.h
    pages/progresspage.h
    pages/readypage.h
    pages/welcomepage.h
    popovers/encryptpopover.h
    popovers/eraseconfirmpopover.h
    popovers/mountpointpopover.h
    popovers/splitpopover.h
    probe/probemanager.h
)


add_executable(scallopinstaller ${SOURCES} ${HEADERS})

set_target_properties(scallopinstaller PROPERTIES
        CNTP_DATA_SUBDIR "scallop/install-system"
        OUTPUT_NAME "scallop-install-system"
)

cntp_init(scallopinstaller 20)
cntp_translate(scallopinstaller)
cntp_target_name(scallopinstaller "Scallop Installer")

target_link_libraries(scallopinstaller Qt6::Widgets Qt6::Multimedia Qt6::Svg Qt6::Network Qt6::StateMachine KF5::PulseAudioQt KF5::NetworkManagerQt libcontemporary libthefrisbee libscallop)
target_include_directories(scallopinstaller PUBLIC ../libscallop/)
target_compile_definitions(scallopinstaller
    PRIVATE SCALLOP_ROOTFS_LOCATION="${SCALLOP_ROOTFS_LOCATION}"
    PRIVATE SCALLOP_PACKAGED_LOCATION="${SCALLOP_PACKAGED_LOCATION}"
)

install(TARGETS scallopinstaller
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)

install(DIRECTORY systemd-boot-config
    DESTINATION ${CMAKE_INSTALL_DATADIR}/scallop/install-system
)
