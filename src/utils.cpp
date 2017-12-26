/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie
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

#include <fcitx-utils/utf8.h>
#include <limits>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "default_tables.h"
#include "utils.h"
#include <fcitx-utils/stringutils.h>

std::string util::utf8_string_substr(const std::string &s, size_t start,
                                     size_t len) {
    auto iter = fcitx::utf8::nextNChar(s.begin(), start);
    auto end = fcitx::utf8::nextNChar(iter, len);
    std::string result(iter, end);
    return result;
}

bool util::match_key_event(const fcitx::KeyList &hotkey, const fcitx::Key &_key,
                           fcitx::KeyStates ignore_mask) {
    fcitx::Key key = fcitx::Key(_key.sym(), _key.states() & ~ignore_mask);
    return key.checkKeyList(hotkey);
}

void util::split_string(std::string &str, std::vector<std::string> &str_list,
                        char *delim, int num) {
    std::string::size_type start = 0, end;

    for (int i = 0; (num > 0 && i < num) || start < str.length(); i++) {
        end = str.find(delim, start);
        if ((num > 0 && i == num - 1) || (end == std::string::npos))
            end = str.length();

        if (start < str.length()) {
            str_list.push_back(str.substr(start, end - start));
            start = end + strlen(delim);
        } else {
            str_list.push_back(std::string());
        }
    }
}

std::string util::convert_to_wide(const std::string &str) {
    std::string wide;
    for (unsigned int i = 0; i < str.length(); i++) {
        int c = str[i];
        char cc[2];
        cc[0] = c;
        cc[1] = '\0';
        bool found = false;

        for (unsigned int j = 0; fcitx_anthy_wide_table[j].code; j++) {
            if (fcitx_anthy_wide_table[j].code &&
                *fcitx_anthy_wide_table[j].code == c) {
                wide += fcitx_anthy_wide_table[j].wide;
                found = true;
                break;
            }
        }

        if (!found)
            wide += cc;
    }
    return wide;
}

std::string util::convert_to_half(const std::string &str) {
    std::string half;
    for (unsigned int i = 0; i < fcitx::utf8::length(str); i++) {
        std::string wide = util::utf8_string_substr(str, i, 1);
        bool found = false;

        for (unsigned int j = 0; fcitx_anthy_wide_table[j].code; j++) {
            if (fcitx_anthy_wide_table[j].wide &&
                wide == fcitx_anthy_wide_table[j].wide) {
                half += fcitx_anthy_wide_table[j].code;
                found = true;
                break;
            }
        }

        if (!found)
            half += wide;
    }
    return half;
}

std::string util::convert_to_katakana(const std::string &hira, bool half) {
    std::string kata;
    for (unsigned int i = 0; i < fcitx::utf8::length(hira); i++) {
        std::string tmpwide;
        bool found = false;

        HiraganaKatakanaRule *table = fcitx_anthy_hiragana_katakana_table;

        for (unsigned int j = 0; table[j].hiragana; j++) {
            tmpwide = table[j].hiragana;
            if (util::utf8_string_substr(hira, i, 1) == tmpwide) {
                if (half)
                    kata += table[j].half_katakana;
                else
                    kata += table[j].katakana;
                found = true;
                break;
            }
        }

        if (!found)
            kata += util::utf8_string_substr(hira, i, 1);
    }
    return kata;
}

bool util::key_is_keypad(const fcitx::Key &key) {
    switch (key.sym()) {
    case FcitxKey_KP_Equal:
    case FcitxKey_KP_Multiply:
    case FcitxKey_KP_Add:
    case FcitxKey_KP_Separator:
    case FcitxKey_KP_Subtract:
    case FcitxKey_KP_Decimal:
    case FcitxKey_KP_Divide:
    case FcitxKey_KP_0:
    case FcitxKey_KP_1:
    case FcitxKey_KP_2:
    case FcitxKey_KP_3:
    case FcitxKey_KP_4:
    case FcitxKey_KP_5:
    case FcitxKey_KP_6:
    case FcitxKey_KP_7:
    case FcitxKey_KP_8:
    case FcitxKey_KP_9:
        return true;
    default:
        return false;
    }
}

std::string util::keypad_to_string(const fcitx::KeyEvent &key) {
    char raw[2];

    switch (key.rawKey().sym()) {
    case FcitxKey_KP_Equal:
        raw[0] = '=';
        break;

    case FcitxKey_KP_Multiply:
        raw[0] = '*';
        break;

    case FcitxKey_KP_Add:
        raw[0] = '+';
        break;

    case FcitxKey_KP_Separator:
        raw[0] = ',';
        break;

    case FcitxKey_KP_Subtract:
        raw[0] = '-';
        break;

    case FcitxKey_KP_Decimal:
        raw[0] = '.';
        break;

    case FcitxKey_KP_Divide:
        raw[0] = '/';
        break;

    case FcitxKey_KP_0:
    case FcitxKey_KP_1:
    case FcitxKey_KP_2:
    case FcitxKey_KP_3:
    case FcitxKey_KP_4:
    case FcitxKey_KP_5:
    case FcitxKey_KP_6:
    case FcitxKey_KP_7:
    case FcitxKey_KP_8:
    case FcitxKey_KP_9:
        raw[0] = '0' + key.rawKey().sym() - FcitxKey_KP_0;
        break;

    default:
        raw[0] = util::get_ascii_code(key);
        break;
    }

    raw[1] = '\0';
    return raw;
}

void util::launch_program(std::string command) {
    if (command.empty())
        return;

    /* split string */
    auto array = fcitx::stringutils::split(command, FCITX_WHITESPACE);

    if (array.size() <= 0)
        return;

    fcitx::startProcess(array);
}

bool util::surrounding_get_safe_delta(uint from, uint to, int32_t *delta) {
    const int64_t kInt32AbsMax =
        llabs(static_cast<int64_t>(std::numeric_limits<int32_t>::max()));
    const int64_t kInt32AbsMin =
        llabs(static_cast<int64_t>(std::numeric_limits<int32_t>::min()));
    const int64_t kInt32SafeAbsMax = std::min(kInt32AbsMax, kInt32AbsMin);

    const int64_t diff = static_cast<int64_t>(from) - static_cast<int64_t>(to);
    if (llabs(diff) > kInt32SafeAbsMax) {
        return false;
    }

    *delta = static_cast<int32_t>(diff);
    return true;
}

// Returns true if |surrounding_text| contains |selected_text|
// from |cursor_pos| to |*anchor_pos|.
// Otherwise returns false.
static bool search_anchor_pos_forward(const std::string &surrounding_text,
                                      const std::string &selected_text,
                                      size_t selected_chars_len,
                                      uint cursor_pos, uint *anchor_pos) {

    size_t len = fcitx::utf8::length(surrounding_text);
    if (len < cursor_pos) {
        return false;
    }

    size_t offset =
        fcitx::utf8::ncharByteLength(surrounding_text.begin(), cursor_pos);

    if (surrounding_text.compare(offset, selected_text.size(), selected_text) !=
        0) {
        return false;
    }
    *anchor_pos = cursor_pos + selected_chars_len;
    return true;
}

// Returns true if |surrounding_text| contains |selected_text|
// from |*anchor_pos| to |cursor_pos|.
// Otherwise returns false.
bool search_anchor_pos_backward(const std::string &surrounding_text,
                                const std::string &selected_text,
                                size_t selected_chars_len, uint cursor_pos,
                                uint *anchor_pos) {
    if (cursor_pos < selected_chars_len) {
        return false;
    }

    // Skip |iter| to (potential) anchor pos.
    const uint skip_count = cursor_pos - selected_chars_len;
    if (skip_count > cursor_pos) {
        return false;
    }
    size_t offset =
        fcitx::utf8::ncharByteLength(surrounding_text.begin(), skip_count);

    if (surrounding_text.compare(offset, selected_text.size(), selected_text) !=
        0) {
        return false;
    }
    *anchor_pos = skip_count;
    return true;
}

bool util::surrounding_get_anchor_pos_from_selection(
    const std::string &surrounding_text, const std::string &selected_text,
    uint cursor_pos, uint *anchor_pos) {
    if (surrounding_text.empty()) {
        return false;
    }

    if (selected_text.empty()) {
        return false;
    }

    const size_t selected_chars_len = fcitx::utf8::length(selected_text);

    if (search_anchor_pos_forward(surrounding_text, selected_text,
                                  selected_chars_len, cursor_pos, anchor_pos)) {
        return true;
    }

    return search_anchor_pos_backward(surrounding_text, selected_text,
                                      selected_chars_len, cursor_pos,
                                      anchor_pos);
}

const fcitx::KeyList &util::selection_keys() {
    const static fcitx::KeyList selectionKeys{
        fcitx::Key(FcitxKey_1), fcitx::Key(FcitxKey_2), fcitx::Key(FcitxKey_3),
        fcitx::Key(FcitxKey_4), fcitx::Key(FcitxKey_5), fcitx::Key(FcitxKey_6),
        fcitx::Key(FcitxKey_7), fcitx::Key(FcitxKey_8), fcitx::Key(FcitxKey_9),
        fcitx::Key(FcitxKey_0)};
    return selectionKeys;
}
