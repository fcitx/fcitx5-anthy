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

    unsigned char get_ascii_code() const { return sym & 0xff; }

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