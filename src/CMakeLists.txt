set( fcitx_anthy_sources
     action.cpp
     style_file.cpp
     conversion.cpp
     default_tables.cpp
     engine.cpp
     kana.cpp
     key2kana.cpp
     key2kana_base.cpp
     key2kana_table.cpp
     preedit.cpp
     reading.cpp
     utils.cpp
     conversion.cpp
     state.cpp
     nicola.cpp
    )

add_library(anthy MODULE ${fcitx_anthy_sources})
target_link_libraries(anthy Fcitx5::Core Fcitx5::Config PkgConfig::Anthy Fcitx5::Module::Clipboard)
set_target_properties(anthy PROPERTIES PREFIX "")
install(TARGETS anthy DESTINATION "${CMAKE_INSTALL_LIBDIR}/fcitx5")
fcitx5_translate_desktop_file(anthy.conf.in anthy.conf)
fcitx5_translate_desktop_file(anthy-addon.conf.in anthy-addon.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/anthy.conf" DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/anthy-addon.conf" RENAME anthy.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")
