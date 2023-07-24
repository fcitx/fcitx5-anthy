add_library(stylefile STATIC style_file.cpp)
set_target_properties(stylefile PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(stylefile Fcitx5::Utils Fcitx5::Config)
target_include_directories(stylefile PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

set( fcitx_anthy_sources
     action.cpp
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
target_link_libraries(anthy Fcitx5::Core Fcitx5::Config ${ANTHY_TARGET} Fcitx5::Module::Clipboard stylefile)
install(TARGETS anthy DESTINATION "${CMAKE_INSTALL_LIBDIR}/fcitx5")
fcitx5_translate_desktop_file(anthy.conf.in anthy.conf)
configure_file(anthy-addon.conf.in.in anthy-addon.conf.in)
fcitx5_translate_desktop_file(${CMAKE_CURRENT_BINARY_DIR}/anthy-addon.conf.in anthy-addon.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/anthy.conf" DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod" COMPONENT config)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/anthy-addon.conf" RENAME anthy.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon" COMPONENT config)
