//
// Copyright (C) 2005 Takuro Ashie
// Copyright (C) 2017~2017 by CSSlayer
// wengxt@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _FCITX5_ANTHY_UTILS_H_
#define _FCITX5_ANTHY_UTILS_H_

#include <fcitx-utils/charutils.h>
#include <fcitx/event.h>
#include <string>

namespace util {

std::string utf8_string_substr(const std::string &s, size_t start, size_t len);

bool match_key_event(const fcitx::KeyList &list, const fcitx::Key &key,
                     fcitx::KeyStates ignore_mask = fcitx::KeyStates());
void split_string(std::string &str, std::vector<std::string> &str_list,
                  char *delim, int num);
std::string convert_to_wide(const std::string &str);
std::string convert_to_half(const std::string &str);
std::string convert_to_katakana(const std::string &hira, bool half = false);

bool key_is_keypad(const fcitx::Key &key);
std::string keypad_to_string(const fcitx::KeyEvent &key);
void launch_program(std::string command);

bool surrounding_get_safe_delta(uint from, uint to, int32_t *delta);

bool surrounding_get_anchor_pos_from_selection(
    const std::string &surrounding_text, const std::string &selected_text,
    uint cursor_pos, uint *anchor_pos);

inline char get_ascii_code(const fcitx::Key &key) {
    auto chr = fcitx::Key::keySymToUnicode(key.sym());
    if (fcitx::charutils::isprint(chr)) {
        return chr;
    }
    return 0;
}

inline char get_ascii_code(const fcitx::KeyEvent &event) {
    return get_ascii_code(event.rawKey());
}

const fcitx::KeyList &selection_keys();
} // namespace util

#endif // _FCITX5_ANTHY_UTILS_H_
