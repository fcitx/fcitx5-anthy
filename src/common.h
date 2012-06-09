#ifndef __FCITX_ANTHY_COMMON_H__
#define __FCITX_ANTHY_COMMON_H__

#include <fcitx-config/hotkey.h>
#include <string>
#include <vector>
#include <libintl.h>

#define FCITX_ANTHY_CONFIG_DICT_ENCODING_DEFAULT               "UTF-8"

class KeyEvent {
public:
    FcitxKeySym sym;
    unsigned int state;
    bool is_release;

    unsigned char get_ascii_code() const {

        if (sym >= FcitxKey_space && sym <= FcitxKey_asciitilde)
            return (char) sym;

        if (sym >= FcitxKey_KP_0 && sym <= FcitxKey_KP_9)
            return (char) (sym - FcitxKey_KP_0 + FcitxKey_0);

        if (sym == FcitxKey_Return)
            return 0x0d;
        if (sym == FcitxKey_Linefeed)
            return 0x0a;
        if (sym == FcitxKey_Tab)
            return 0x09;
        if (sym == FcitxKey_BackSpace)
            return 0x08;
        if (sym == FcitxKey_Escape)
            return 0x1b;

        return 0;
    }

    bool operator == (const KeyEvent& event) const {
        return sym == event.sym && state == event.state;
    }

    bool empty() const {
        return sym == FcitxKey_None;
    }
};

#define _(x) dgettext("fcitx-anthy", (x))

typedef std::vector<KeyEvent> KeyEventList;

#endif