include_directories (${FCITX4_FCITX_INCLUDE_DIRS})
include_directories (${FCITX4_FCITX_CONFIG_INCLUDE_DIRS})
include_directories (${FCITX4_FCITX_UTILS_INCLUDE_DIRS})

set( fcitx_anthy_sources
     action.cpp
     style_file.cpp
     conversion.cpp
     default_tables.cpp
     factory.cpp
     kana.cpp
     key2kana.cpp
     key2kana_table.cpp
     preedit.cpp
     reading.cpp
     utils.cpp
     conversion.cpp
     imengine.cpp
     nicola.cpp
    )

add_definitions( -DLOCALEDIR=\"${CMAKE_INSTALL_PREFIX}/share/locale\" )

fcitx_add_addon_full(anthy DESC SOURCES ${fcitx_anthy_sources}
  IM_CONFIG anthy.conf
  LINK_LIBS ${ANTHY_TARGET})
