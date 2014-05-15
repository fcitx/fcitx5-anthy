/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2012 CSSlayer
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

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
    unsigned int keycode;
    bool is_release;

    KeyEvent() {
        sym = FcitxKey_None;
        state = FcitxKeyState_None;
        is_release = false;
    }

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
