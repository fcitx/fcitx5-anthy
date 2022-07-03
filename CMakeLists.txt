cmake_minimum_required(VERSION 3.6)

project(fcitx5-anthy VERSION 5.0.12)

find_package(ECM 1.0.0 REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(FeatureSummary)
include(GNUInstallDirs)
include(ECMUninstallTarget)

find_package(Fcitx5Core 5.0.6 REQUIRED)
find_package(Fcitx5Module REQUIRED COMPONENTS Clipboard)
find_package(Gettext REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(AnthyUnicode IMPORTED_TARGET "anthy-unicode")

if (AnthyUnicode_FOUND)
    set(ANTHY_TARGET PkgConfig::AnthyUnicode)
else()
    pkg_check_modules(Anthy IMPORTED_TARGET "anthy" REQUIRED)
    set(ANTHY_TARGET PkgConfig::Anthy)
endif()

include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")
add_definitions(-DFCITX_GETTEXT_DOMAIN=\"fcitx5-anthy\" -D_GNU_SOURCE)
fcitx5_add_i18n_definition()

add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(icon)
add_subdirectory(profile)

fcitx5_translate_desktop_file(org.fcitx.Fcitx5.Addon.Anthy.metainfo.xml.in
    org.fcitx.Fcitx5.Addon.Anthy.metainfo.xml XML)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/org.fcitx.Fcitx5.Addon.Anthy.metainfo.xml" DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
