cmake_minimum_required(VERSION 3.6)

project(fcitx5-anthy)

find_package(ECM REQUIRED 1.0.0)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(FeatureSummary)
include(GNUInstallDirs)
include(ECMUninstallTarget)

find_package(Fcitx5Core REQUIRED)
find_package(Fcitx5Module REQUIRED COMPONENTS Clipboard)
find_package(Gettext REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Anthy IMPORTED_TARGET "anthy" REQUIRED)

include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")
add_definitions(-DFCITX_GETTEXT_DOMAIN=\"fcitx5-anthy\" -D_GNU_SOURCE)

add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(icon)
add_subdirectory(profile)
