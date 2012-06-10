install(DIRECTORY 48x48 22x22 scalable DESTINATION share/icons/hicolor
         PATTERN ".*" EXCLUDE
         PATTERN "*~" EXCLUDE)


install(FILES 48x48/status/fcitx-anthy.png DESTINATION share/fcitx/imicon RENAME anthy.png)
