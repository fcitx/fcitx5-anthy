include_directories (${ANTHY_INCLUDE_DIR})
include_directories (${FCITX4_FCITX_INCLUDE_DIRS})
include_directories (${FCITX4_FCITX_CONFIG_INCLUDE_DIRS})
include_directories (${FCITX4_FCITX_UTILS_INCLUDE_DIRS})

set( fcitx_anthy_sources
     config.cpp
     conversion.cpp
     defaulttables.cpp
     eim.cpp
     kana.cpp
     key2kana.cpp
     key2kanatable.cpp
     preedit.cpp
     reading.cpp
     utils.cpp
    )

add_definitions( -DLOCALEDIR=\"${CMAKE_INSTALL_PREFIX}/share/locale\" )

fcitx_add_addon(fcitx-anthy ${fcitx_anthy_sources} )
target_link_libraries (fcitx-anthy ${ANTHY_LIBRARIES})
