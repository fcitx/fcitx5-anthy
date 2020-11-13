cmake_minimum_required(VERSION 3.6)

project(fcitx-anthy)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(Fcitx 4.2.9.2 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(AnthyUnicode IMPORTED_TARGET "anthy-unicode")

if (AnthyUnicode_FOUND)
    set(ANTHY_TARGET PkgConfig::AnthyUnicode)
else()
    pkg_check_modules(Anthy IMPORTED_TARGET "anthy" REQUIRED)
    set(ANTHY_TARGET PkgConfig::Anthy)
endif()

_fcitx_add_uninstall_target()

set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fvisibility=hidden ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fvisibility=hidden ${CMAKE_CXX_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_MODULE_LINKER_FLAGS}")

if(NOT DEFINED LIB_INSTALL_DIR)
  set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()

set(HAVE_CONFIG_H)

set(libdir ${LIB_INSTALL_DIR})

add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(icon)
add_subdirectory(profile)
