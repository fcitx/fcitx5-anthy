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
#include "kana.h"
#include "default_tables.h"
#include "engine.h"
#include "state.h"
#include "utils.h"

static bool has_voiced_consonant(std::string str) {
    VoicedConsonantRule *table = fcitx_anthy_voiced_consonant_table;

    for (unsigned int i = 0; table[i].string; i++) {
        if (!strcmp(str.c_str(), table[i].string) && table[i].voiced &&
            *table[i].voiced) {
            return true;
        }
    }

    return false;
}

static bool has_half_voiced_consonant(std::string str) {
    VoicedConsonantRule *table = fcitx_anthy_voiced_consonant_table;

    for (unsigned int i = 0; table[i].string; i++) {
        if (!strcmp(str.c_str(), table[i].string) && table[i].half_voiced &&
            *table[i].half_voiced) {
            return true;
        }
    }

    return false;
}

std::string to_voiced_consonant(std::string str) {
    VoicedConsonantRule *table = fcitx_anthy_voiced_consonant_table;

    for (unsigned int i = 0; table[i].string; i++) {
        if (!strcmp(str.c_str(), table[i].string))
            return std::string(table[i].voiced);
    }

    return str;
}

std::string to_half_voiced_consonant(std::string str) {
    VoicedConsonantRule *table = fcitx_anthy_voiced_consonant_table;

    for (unsigned int i = 0; table[i].string; i++) {
        if (!strcmp(str.c_str(), table[i].string))
            return std::string(table[i].half_voiced);
    }

    return str;
}

KanaConvertor::KanaConvertor(AnthyState &anthy)
    : Key2KanaConvertorBase(anthy) {}

KanaConvertor::~KanaConvertor() {}

bool KanaConvertor::canAppend(const fcitx::KeyEvent &key, bool) {
    // ignore key release.
    if (key.isRelease())
        return false;

    auto states = key.rawKey().states();
    // ignore short cut keys of application.
    if ((states & fcitx::KeyState::Ctrl) || (states & fcitx::KeyState::Alt) ||
        (states & fcitx::KeyState::Super)) {
        return false;
    }

    auto sym = key.rawKey().sym();
    if (sym == FcitxKey_overline ||
        (sym >= FcitxKey_kana_fullstop && sym <= FcitxKey_semivoicedsound)) {
        return true;
    }

#if 0
    if (key.code == SCIM_KEY_KP_Equal ||
        (key.code >= SCIM_KEY_KP_Multiply &&
         key.code <= SCIM_KEY_KP_9))
    {
        return true;
    }
#endif

    return false;
}

bool KanaConvertor::append(const fcitx::KeyEvent &key, std::string &result,
                           std::string &pending, std::string &raw) {
    KeyCodeToCharRule *table = fcitx_anthy_keypad_table;

    auto sym = key.rawKey().sym();
    // handle keypad code
    if (sym == FcitxKey_KP_Equal ||
        (sym >= FcitxKey_KP_Multiply && sym <= FcitxKey_KP_9)) {
        TenKeyType ten_key_type = *config().m_general->m_ten_key_type;

        for (unsigned int i = 0; table[i].code; i++) {
            if (table[i].code == sym) {
                if (ten_key_type == TenKeyType::WIDE)
                    result = util::convert_to_wide(table[i].kana);
                else
                    result = table[i].kana;
                raw = table[i].kana;

                return false;
            }
        }
    }

    table = fcitx_anthy_kana_table;

    // handle voiced sound
    if (sym == FcitxKey_voicedsound && !pending_.empty() &&
        has_voiced_consonant(pending_)) {
        result = to_voiced_consonant(pending_);
        raw = util::get_ascii_code(key);
        pending_ = std::string();
        return false;
    }

    // handle semi voiced sound
    if (sym == FcitxKey_semivoicedsound && !pending_.empty() &&
        has_half_voiced_consonant(pending_)) {
        result = to_half_voiced_consonant(pending_);
        raw = util::get_ascii_code(key);
        pending_ = std::string();
        return false;
    }

    // kana key code
    for (unsigned int i = 0; table[i].code; i++) {
        if (table[i].code == sym) {
            bool retval = pending_.empty() ? false : true;

            if (has_voiced_consonant(table[i].kana)) {
                result = std::string();
                pending = table[i].kana;
                pending_ = table[i].kana;
            } else {
                result = table[i].kana;
                pending_ = std::string();
            }
            raw = util::get_ascii_code(key);

            return retval;
        }
    }

    std::string s;
    s += util::get_ascii_code(key);
    raw = s;

    return append(raw, result, pending);
}

bool KanaConvertor::append(const std::string &str, std::string &result,
                           std::string &) {
    result = str;
    pending_ = std::string();

    return false;
}

void KanaConvertor::clear() { pending_ = std::string(); }

bool KanaConvertor::isPending() const { return !pending_.empty(); }

std::string KanaConvertor::pending() const { return pending_; }

std::string KanaConvertor::flushPending() { return std::string(); }

void KanaConvertor::resetPending(const std::string &result,
                                 const std::string &) {
    pending_ = std::string();
    if (has_voiced_consonant(result))
        pending_ = result;
}
