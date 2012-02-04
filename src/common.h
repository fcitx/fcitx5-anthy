#define RESET_FCITX_HOTKEY(key) \
    do { \
        (key).sym = FcitxKey_None; \
        (key).state = FcitxKeyState_None; \
    } while(0)

#define FCITX_HOTKEY_EMPTY(key) ((key).sym == FcitxKey_None && (key).state == FcitxKeyState_None)