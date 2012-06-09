/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>

#include "kana.h"
#include "factory.h"
#include "imengine.h"
#include "default_tables.h"
#include "utils.h"

static bool
has_voiced_consonant (std::string str)
{
    VoicedConsonantRule *table = fcitx_anthy_voiced_consonant_table;

    for (unsigned int i = 0; table[i].string; i++) {
        if (!strcmp (str.c_str (), table[i].string) &&
            table[i].voiced && *table[i].voiced)
        {
            return true;
        }
    }

    return false;
}

static bool
has_half_voiced_consonant (std::string str)
{
    VoicedConsonantRule *table = fcitx_anthy_voiced_consonant_table;

    for (unsigned int i = 0; table[i].string; i++) {
        if (!strcmp (str.c_str (), table[i].string) &&
            table[i].half_voiced && *table[i].half_voiced)
        {
            return true;
        }
    }

    return false;
}

std::string
to_voiced_consonant (std::string str)
{
    VoicedConsonantRule *table = fcitx_anthy_voiced_consonant_table;

    for (unsigned int i = 0; table[i].string; i++) {
        if (!strcmp (str.c_str (), table[i].string))
            return std::string (table[i].voiced);
    }

    return str;
}

std::string
to_half_voiced_consonant (std::string str)
{
    VoicedConsonantRule *table = fcitx_anthy_voiced_consonant_table;

    for (unsigned int i = 0; table[i].string; i++) {
        if (!strcmp (str.c_str (), table[i].string))
            return std::string (table[i].half_voiced);
    }

    return str;
}

KanaConvertor::KanaConvertor (AnthyInstance &anthy)
    : m_anthy (anthy)
{
}

KanaConvertor::~KanaConvertor ()
{
}

bool
KanaConvertor::can_append (const KeyEvent & key,
                           bool             ignore_space)
{
    // ignore key release.
    if (key.is_release)
        return false;

    // ignore short cut keys of apllication.
    if (key.state & FcitxKeyState_Ctrl ||
        key.state & FcitxKeyState_Alt)
    {
        return false;
    }

    if (key.sym == FcitxKey_overline ||
        (key.sym >= FcitxKey_kana_fullstop &&
         key.sym <= FcitxKey_semivoicedsound))
    {
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

bool
KanaConvertor::append (const KeyEvent & key,
                       std::string & result,
                       std::string & pending,
                       std::string &raw)
{
    KeyCodeToCharRule *table = fcitx_anthy_keypad_table;

    // handle keypad code
    if (key.sym == FcitxKey_KP_Equal ||
        (key.sym >= FcitxKey_KP_Multiply &&
         key.sym <= FcitxKey_KP_9))
    {
        TenKeyType ten_key_type = m_anthy.get_config()->m_ten_key_type;

        for (unsigned int i = 0; table[i].code; i++) {
            if (table[i].code == key.sym) {
                if (ten_key_type == FCITX_ANTHY_TEN_KEY_TYPE_WIDE)
                    util_convert_to_wide (result, table[i].kana);
                else
                    result = table[i].kana;
                raw = table[i].kana;

                return false;
            }
        }
    }

    table = fcitx_anthy_kana_table;

    // handle voiced sound
    if (key.sym == FcitxKey_voicedsound &&
        !m_pending.empty () && has_voiced_consonant (m_pending))
    {
        result = to_voiced_consonant (m_pending);
        raw    = key.get_ascii_code ();
        m_pending = std::string ();
        return false;
    }

    // handle semi voiced sound
    if (key.sym == FcitxKey_semivoicedsound &&
        !m_pending.empty () && has_half_voiced_consonant (m_pending))
    {
        result = to_half_voiced_consonant (m_pending);
        raw    = key.get_ascii_code ();
        m_pending = std::string ();
        return false;
    }

    // kana key code
    for (unsigned int i = 0; table[i].code; i++) {
        if (table[i].code == key.sym) {
            bool retval = m_pending.empty () ? false : true;

            if (has_voiced_consonant (table[i].kana)) {
                result = std::string ();
                pending = table[i].kana;
                m_pending = table[i].kana;
            } else {
                result = table[i].kana;
                m_pending = std::string ();
            }
            raw = key.get_ascii_code ();

            return retval;
        }
    }

    std::string s;
    s += key.get_ascii_code ();
    raw    = s;

    return append (raw, result, pending);
}

bool
KanaConvertor::append (const std::string & str,
                       std::string   & result,
                       std::string   & pending)
{
    result = str;
    m_pending = std::string ();

    return false;
}

void
KanaConvertor::clear (void)
{
    m_pending = std::string ();
}

bool
KanaConvertor::is_pending (void)
{
    return !m_pending.empty ();
}

std::string
KanaConvertor::get_pending (void)
{
    return std::string (m_pending);
}

std::string
KanaConvertor::flush_pending (void)
{
    return std::string ();
}

void
KanaConvertor::reset_pending (const std::string &result, const std::string &raw)
{
    m_pending = std::string ();
    if (has_voiced_consonant (result))
        m_pending = result;
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
