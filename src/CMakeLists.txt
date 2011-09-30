include(FindPkgConfig)

PKG_CHECK_MODULES(ANTHY REQUIRED "anthy" )

include_directories (${ANTHY_INCLUDE_DIRS})
include_directories (${FCITX_CONFIG_INCLUDE_DIRS})
include_directories (${FCITX_INCLUDE_DIRS})

set( fcitx_anthy_sources
    eim.cpp
   
    config.cpp
    )

add_definitions( -DLOCALEDIR=\"${CMAKE_INSTALL_PREFIX}/share/locale\" )

fcitx_add_addon(fcitx-anthy ${fcitx_anthy_sources} )
target_link_libraries (fcitx-anthy ${ANTHY_LIBRARIES})
target_link_libraries (fcitx-anthy ${FCITX4_FCITX_CONFIG_LIBRARIES})
target_link_libraries (fcitx-anthy ${FCITX4_FCITX_LIBRARIES})
