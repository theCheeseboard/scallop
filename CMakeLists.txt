cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)

project(scallop VERSION 1.0.0 LANGUAGES CXX)

set(SCALLOP_ROOTFS_LOCATION "https://packages.vicr123.com/cactus/rootfs/x86_64/latest.squashfs" CACHE STRING "Location to download the root squashfs from, if not installed locally")
set(SCALLOP_PACKAGED_LOCATION "/opt/cactus-recovery-media/rootfs.squashfs" CACHE STRING "Location of the root squashfs file on disk")

add_subdirectory(libscallop)
add_subdirectory(boot-utilities)
add_subdirectory(installer)
add_subdirectory(onboarding)
add_subdirectory(onboarding-service)
add_subdirectory(reset)
