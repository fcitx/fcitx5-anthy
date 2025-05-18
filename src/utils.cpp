/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  SPDX-FileCopyrightText: 2005 Takuro Ashie
 *  SPDX-FileCopyrightText: 2012 CSSlayer <wengxt@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "utils.h"
#include "default_tables.h"
#include "fcitx-utils/keysym.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fcitx-utils/key.h>
#include <fcitx-utils/macros.h>
#include <fcitx-utils/misc.h>
#include <fcitx-utils/stringutils.h>
#include <fcitx-utils/utf8.h>
#include <fcitx/event.h>
#include <limits>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

std::string util::utf8_string_substr(const std::string &s, size_t start,
                                     size_t len) {
    auto iter = fcitx::utf8::nextNChar(s.begin(), start);
    auto end = fcitx::utf8::nextNChar(iter, len);
    std::string result(iter, end);
    return result;
}

bool util::match_key_event(const fcitx::KeyList &list, const fcitx::Key &key,
                           fcitx::KeyStates ignore_mask) {
    fcitx::Key normKey = fcitx::Key(key.sym(), key.states() & ~ignore_mask);
    return normKey.checkKeyList(list);
}

std::string util::convert_to_wide(const std::string &str) {
    std::string wide;
    for (const auto c : str) {
        char cc[2];
        cc[0] = c;
        cc[1] = '\0';
        bool found = false;

        for (const auto &item : fcitx_anthy_wide_table) {
            if (item.code == cc) {
                wide += item.wide;
                found = true;
                break;
            }
        }

        if (!found) {
            wide += cc;
        }
    }
    return wide;
}

std::string util::convert_to_half(const std::string &str) {
    std::string half;
    for (unsigned int i = 0; i < fcitx::utf8::length(str); i++) {
        std::string wide = util::utf8_string_substr(str, i, 1);
        bool found = false;

        for (const auto &item : fcitx_anthy_wide_table) {
            if (wide == item.wide) {
                half += item.code;
                found = true;
                break;
            }
        }

        if (!found) {
            half += wide;
        }
    }
    return half;
}

std::string util::convert_to_katakana(const std::string &hira, bool half) {
    std::string kata;
    for (auto chr : fcitx::utf8::MakeUTF8StringViewRange(hira)) {
        bool found = false;

        for (const auto &item : fcitx_anthy_hiragana_katakana_table) {
            if (chr == item.hiragana) {
                if (half) {
                    kata += item.half_katakana;
                } else {
                    kata += item.katakana;
                }
                found = true;
                break;
            }
        }

        if (!found) {
            kata += chr;
        }
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

void util::launch_program(std::string_view command) {
    if (command.empty()) {
        return;
    }

    /* split string */
    auto array = fcitx::stringutils::split(command, FCITX_WHITESPACE);

    if (array.size() <= 0) {
        return;
    }

    fcitx::startProcess(array);
}

bool util::surrounding_get_safe_delta(uint from, uint to, int32_t *delta) {
    const int64_t kInt32AbsMax =
        std::llabs(static_cast<int64_t>(std::numeric_limits<int32_t>::max()));
    const int64_t kInt32AbsMin =
        std::llabs(static_cast<int64_t>(std::numeric_limits<int32_t>::min()));
    const int64_t kInt32SafeAbsMax = std::min(kInt32AbsMax, kInt32AbsMin);

    const int64_t diff = static_cast<int64_t>(from) - static_cast<int64_t>(to);
    if (std::llabs(diff) > kInt32SafeAbsMax) {
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
