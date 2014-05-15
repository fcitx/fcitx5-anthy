/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
 *  Copyright (C) 2004 Takuro Ashie
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

#include "key2kana.h"
#include "factory.h"
#include "imengine.h"
#include "utils.h"

Key2KanaConvertor::Key2KanaConvertor (AnthyInstance    & anthy,
                                      Key2KanaTableSet & tables)
    : m_anthy                   (anthy),
      m_tables                  (tables),
      m_is_in_pseudo_ascii_mode (false)
{
    set_case_sensitive (false);
    set_pseudo_ascii_mode (0);
}

Key2KanaConvertor::~Key2KanaConvertor ()
{
}

bool
Key2KanaConvertor::can_append (const KeyEvent & key,
                               bool             ignore_space)
{
    // ignore key release.
    if (key.is_release)
        return false;

    // ignore short cut keys of apllication.
    if ((key.state & FcitxKeyState_Ctrl) ||
        (key.state & FcitxKeyState_Alt) ||
        (key.state & FcitxKeyState_Super))
    {
        return false;
    }

    if (isprint(key.get_ascii_code ()) &&
        (ignore_space || !isspace(key.get_ascii_code ())))
        return true;

    if (util_key_is_keypad (key))
        return true;

    return false;
}

bool
Key2KanaConvertor::append (const KeyEvent & key,
                           std::string & result,
                           std::string & pending,
                           std::string &raw)
{
    if (!can_append (key))
        return false;

    m_last_key = key;

    util_keypad_to_string (raw, key);

    if (util_key_is_keypad (key)) {
        bool retval = false;
        std::string wide;
        TenKeyType ten_key_type = m_anthy.get_config()->m_ten_key_type;

        // convert key pad string to wide
        if ((ten_key_type == FCITX_ANTHY_TEN_KEY_TYPE_FOLLOWMODE &&
             (m_anthy.get_input_mode () == FCITX_ANTHY_MODE_LATIN ||
              m_anthy.get_input_mode () == FCITX_ANTHY_MODE_HALF_KATAKANA)) ||
            ten_key_type == FCITX_ANTHY_TEN_KEY_TYPE_HALF)
        {
            wide = raw;
        } else {
            util_convert_to_wide (wide, raw);
        }

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

static
void split_string_list(std::vector<std::string>& str, const std::string& s)
{

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

    if (m_pseudo_ascii_mode != 0 && process_pseudo_ascii_mode (widestr)) {
        m_pending += widestr;
        pending = m_pending;
        return false;
    }
    if (!m_case_sensitive) {
        std::string half = matching_str;
        for (unsigned int i = 0; i < half.length (); i++)
            half[i] = tolower (half[i]);
        matching_str = half;
    }

    /* find matched table */
    if ((m_anthy.get_typing_method () == FCITX_ANTHY_TYPING_METHOD_KANA) &&
        (m_last_key.keycode == 132 || m_last_key.keycode == 133) &&
        (m_anthy.get_config()->m_kana_layout_ro_key[0]))
    {
        // Special treatment for Kana "Ro" key.
        // This code is a temporary solution. It doesn't care some minor cases.
        std::vector<std::string> kana_ro_result;
        split_string_list (kana_ro_result,
                                m_anthy.get_config()->m_kana_layout_ro_key);
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
    m_last_key = KeyEvent ();
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
    m_last_key = KeyEvent ();

    for (unsigned int i = 0; i < util_utf8_string_length(raw); i++) {
        std::string res, pend;
        append (util_utf8_string_substr(raw, i, 1), res, pend);
    }
}

bool
Key2KanaConvertor::process_pseudo_ascii_mode (const std::string & wstr)
{
    for (unsigned int i = 0; i < wstr.length (); i++) {
        if ((wstr[i] >= 'A' && wstr[i] <= 'Z') ||
            isspace(wstr[i]))
        {
            m_is_in_pseudo_ascii_mode = true;
        } else if (wstr[i] & 0x80) {
            m_is_in_pseudo_ascii_mode = false;
        }
    }

    return m_is_in_pseudo_ascii_mode;
}

void
Key2KanaConvertor::reset_pseudo_ascii_mode (void)
{
    if (m_is_in_pseudo_ascii_mode)
        m_pending.clear();
    m_is_in_pseudo_ascii_mode = false;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
