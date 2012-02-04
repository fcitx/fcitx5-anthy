/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
 *  Copyright (C) 2004 Takuro Ashie
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

#include "key2kana.h"
#include <fcitx/keys.h>
#include "utils.h"
#include "preedit.h"
#include "eim.h"

Key2KanaConvertor::Key2KanaConvertor (FcitxAnthy* anthy, Key2KanaTableSet & tables)
    : m_anthy(anthy),
      m_tables                  (tables)
{
    set_case_sensitive (false);
}

Key2KanaConvertor::~Key2KanaConvertor ()
{
}

bool
Key2KanaConvertor::can_append (FcitxKeySym sym, unsigned int state,
                               bool             ignore_space)
{
    // ignore short cut keys of apllication.
    if (FcitxHotkeyIsHotKeyModifierCombine(sym, state))
    {
        return false;
    }

    if (FcitxHotkeyIsHotKeySimple(sym, state) &&
        (ignore_space || !FcitxHotkeyIsHotKey(sym, state, FCITX_SPACE)))
        return true;

    if (util_key_is_keypad (sym, state))
        return true;

    return false;
}

bool
Key2KanaConvertor::append (FcitxKeySym sym, unsigned int state,
                           std::string & result,
                           std::string & pending,
                           std::string &raw)
{
    if (!can_append (sym, state))
        return false;

    m_last_key.sym = sym;
    m_last_key.state = state;

    util_keypad_to_string (raw, sym, state);

    if (util_key_is_keypad (sym, state)) {
        bool retval = false;
        std::string wide;
        std::string ten_key_type = m_anthy->config.m_ten_key_type;

        util_convert_to_wide (wide, raw);

        // join to previous string
        if (!m_exact_match.is_empty()) {
            if (!m_exact_match.get_result(0).empty() &&
                m_exact_match.get_result(1).empty())
            {
                result = m_exact_match.get_result(0);
            } else {
                retval = true; /* commit prev pending */
            }
            result += wide;
        } else {
            if (m_pending.length () > 0)
                retval = true; /* commit prev pending */
            result = wide;
        }

        m_pending.clear ();
        m_exact_match.clear ();

        return retval;

    } else {
        // the key isn't keypad
        return append (raw, result, pending);
    }
}

bool
Key2KanaConvertor::append (const std::string & str,
                           std::string & result, std::string & pending)
{
    std::string widestr = str;
    std::string matching_str = m_pending + widestr;
    Key2KanaRule exact_match;
    bool has_partial_match = false;
    bool retval = false;

    if (!m_case_sensitive) {
        std::string half = matching_str;
        for (unsigned int i = 0; i < half.length (); i++)
            half[i] = tolower (half[i]);
        matching_str = half;
    }

    /* find matched table */
    if ((m_anthy->typing_method == SCIM_ANTHY_TYPING_METHOD_KANA) &&
        (m_last_key.state & /*SCIM_KEY_QuirkKanaRoMask*/ (1<<14)) &&
        (strlen(m_anthy->config.m_kana_layout_ro_key) > 0))
    {
        // Special treatment for Kana "Ro" key.
        // This code is a temporary solution. It doesn't care some minor cases.
        std::vector<std::string> kana_ro_result;
        
        UT_array* list = fcitx_utils_split_string(m_anthy->config.m_kana_layout_ro_key, ',');
        for (int i = 0; i < utarray_len(list); i ++)
            kana_ro_result.push_back(*(char**) utarray_eltptr(list, i));
        
        fcitx_utils_free_string_list(list);
        Key2KanaRule kana_ro_rule("\\", kana_ro_result);
        result = kana_ro_rule.get_result (0);
        m_pending.clear ();
        m_exact_match.clear ();
        if (matching_str == "\\") {
            return false;
        } else {
            return true;
        }

    } else {
        std::vector<Key2KanaTable*> &tables = m_tables.get_tables();
        for (unsigned int j = 0; j < tables.size(); j++) {
            if (!tables[j])
                continue;

            Key2KanaRules &rules = tables[j]->get_table ();

            for (unsigned int i = 0; i < rules.size(); i++) {
                /* matching */
                std::string seq = rules[i].get_sequence ();
                if (!m_case_sensitive) {
                    for (unsigned int j = 0; j < seq.length (); j++)
                        seq[j] = tolower (seq[j]);
                }
                std::string romaji = seq;
                if (romaji.find (matching_str) == 0) {
                    if (romaji.length () == matching_str.length ()) {
                        /* exact match */
                        exact_match = rules[i];
                    } else {
                        /* partial match */
                        has_partial_match = true;
                    }
                }
            }
        }
    }

    /* return result */
    if (has_partial_match) {
        m_exact_match = exact_match;
        result.clear ();
        m_pending += widestr;
        pending   =  m_pending;

    } else if (!exact_match.is_empty()) {
        if (!exact_match.get_result(1).empty())
            m_exact_match = exact_match;
        else
            m_exact_match.clear ();
        m_pending = exact_match.get_result (1);
        result    = exact_match.get_result (0);
        pending   = m_pending;

    } else {
        if (!m_exact_match.is_empty()) {
            if (!m_exact_match.get_result(0).empty() &&
                m_exact_match.get_result(1).empty())
            {
                result = m_exact_match.get_result(0);
            } else {
                retval = true; /* commit prev pending */
            }
            m_pending.clear ();
            m_exact_match.clear ();

            std::string tmp_result;
            append(str, tmp_result, pending);
            result += tmp_result;

        } else {
            if (m_pending.length () > 0) {
                retval     = true; /* commit prev pending */
                m_pending  = widestr;
                pending    = m_pending;

            } else {
                result     = widestr;
                pending.clear();
                m_pending.clear ();
            }
        }
    }

    return retval;
}

void
Key2KanaConvertor::clear (void)
{
    m_pending.clear ();
    m_exact_match.clear ();
    RESET_FCITX_HOTKEY(m_last_key);
    reset_pseudo_ascii_mode();
}

bool
Key2KanaConvertor::is_pending (void)
{
    if (m_pending.length () > 0)
        return true;
    else
        return false;
}

std::string
Key2KanaConvertor::get_pending (void)
{
    return m_pending;
}

std::string
Key2KanaConvertor::flush_pending (void)
{
    std::string result;
    if (!m_exact_match.is_empty ()) {
        if (!m_exact_match.get_result(0).empty() &&
            m_exact_match.get_result(1).empty())
        {
            result = m_exact_match.get_result(0);
        } else if (!m_exact_match.get_result(1).empty()) {
            result += m_exact_match.get_result(1);
        } else if (m_pending.length () > 0) {
            result += m_pending;
        }
    }
    clear ();
    return result;
}

void
Key2KanaConvertor::reset_pending (const std::string &result, const std::string &raw)
{
    m_last_key.sym = FcitxKey_None;
    m_last_key.state = FcitxKeyState_None;

    for (unsigned int i = 0; i < raw.length (); i++) {
        std::string res, pend;
        append (raw.substr(i, 1), res, pend);
    }
}


/*
vi:ts=4:nowrap:ai:expandtab
*/
