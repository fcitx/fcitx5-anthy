fcitx_add_addon_conf_file(fcitx-anthy.conf)
fcitx_add_configdesc_file(fcitx-anthy.desc)
fcitx_add_inputmethod_conf_file(anthy.conf)

install(FILES anthy.png DESTINATION share/fcitx/imicon RENAME anthy.png)
install(FILES anthy.png DESTINATION share/icons/hicolor/48x48/status RENAME fcitx-anthy.png)