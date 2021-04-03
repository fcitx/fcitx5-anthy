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

#include "nicola.h"
#include "factory.h"
#include "imengine.h"
#include "utils.h"
#include <fcitx-utils/log.h>

void NicolaTimeoutFunc(void* arg)
{
    NicolaConvertor* convertor = (NicolaConvertor*) arg;
    convertor->process_timeout();
}

NicolaConvertor::NicolaConvertor (AnthyInstance &anthy,
                                  Key2KanaTableSet &tables)
    : m_tables            (tables),
      m_anthy             (anthy),
      m_timer_id          (0),
      m_processing_timeout(false)
{
}

NicolaConvertor::~NicolaConvertor ()
{
    FcitxInstanceRemoveTimeoutByFunc(m_anthy.get_owner(), NicolaTimeoutFunc);
}

bool
NicolaConvertor::can_append (const KeyEvent & key,
                             bool             ignore_space)
{
    if (key == m_through_key_event) {
        m_through_key_event = KeyEvent ();
        return false;
    }

    if (m_processing_timeout &&
        m_prev_char_key.empty () && !m_prev_thumb_key.empty())
    {
        emit_key_event (m_prev_thumb_key);
        m_prev_thumb_key = KeyEvent ();
        return false;
    }

    if (key.is_release &&
        (key.sym != m_prev_char_key.sym &&
         key.sym != m_repeat_char_key.sym &&
         key.sym != m_prev_thumb_key.sym &&
         key.sym != m_repeat_thumb_key.sym
        ))
    {
        return false;
    }

    // ignore short cut keys of apllication.
    if ((key.state & FcitxKeyState_Ctrl) ||
        (key.state & FcitxKeyState_Alt) ||
        (key.state & FcitxKeyState_Super))
    {
        return false;
    }

    if (isprint (key.get_ascii_code ()) &&
        (ignore_space || !isspace (key.get_ascii_code ())))
    {
        return true;
    }

    if (is_thumb_key (key))
        return true;

    return false;
}

void
NicolaConvertor::search (const KeyEvent& key,
                         NicolaShiftType shift_type,
                         std::string &result,
                         std::string &raw)
{
    raw = key.get_ascii_code ();

    std::string str1;
    if (get_case_sensitive ())
        str1 = raw;
    else
        str1 = tolower (key.get_ascii_code ());

    std::vector<Key2KanaTable*> &tables = m_tables.get_tables();
    for (unsigned int j = 0; j < tables.size (); j++) {
        if (!tables[j])
            continue;

        Key2KanaRules &rules = tables[j]->get_table ();

        for (unsigned int i = 0; i < rules.size (); i++) {
            std::string str2 = rules[i].get_sequence ();

            for (unsigned int k = 0;
                 !get_case_sensitive () && k < str2.length ();
                 k++)
            {
                str2[k] = tolower (str2[k]);
            }

            if (str1 == str2) {
                switch (shift_type) {
                case FCITX_ANTHY_NICOLA_SHIFT_RIGHT:
                    result = rules[i].get_result (2);
                    break;
                case FCITX_ANTHY_NICOLA_SHIFT_LEFT:
                    result = rules[i].get_result (1);
                    break;
                default:
                    result = rules[i].get_result (0);
                    break;
                }
                break;
            }
        }
    }

    if (result.empty ()) {
        result = raw;
    }
}

bool
NicolaConvertor::handle_voiced_consonant  (std::string & result,
                                           std::string & pending)
{
    VoicedConsonantRule *table = fcitx_anthy_voiced_consonant_table;

    if (result.empty ())
        return false;

    if (m_pending.empty ()) {
        for (unsigned int i = 0; table[i].string; i++) {
            if (result == table[i].string) {
                pending = m_pending = result;
                result = std::string ();
                return false;
            }
        }

    } else if (result == "\xE3\x82\x9B") {
        // voiced consonant
        for (unsigned int i = 0; table[i].string; i++) {
            if (m_pending == table[i].string) {
                result = table[i].voiced;
                m_pending = std::string ();
                return false;
            }
        }
        return true;

    } else if (result == "\xE3\x82\x9C") {
        // half voiced consonant
        for (unsigned int i = 0; table[i].string; i++) {
            if (m_pending == table[i].string) {
                result = table[i].half_voiced;
                m_pending = std::string ();
                return false;
            }
        }
        return true;

    } else {
        m_pending = std::string ();
        for (unsigned int i = 0; table[i].string; i++) {
            if (result == table[i].string) {
                pending = m_pending = result;
                result = std::string ();
                return true;
            }
        }
        return true;
    }

    return false;
}

bool
NicolaConvertor::is_char_key (const KeyEvent& key)
{
    if (!is_thumb_key (key) && isprint (key.get_ascii_code ()))
        return true;
    else
        return false;
}

bool
NicolaConvertor::is_thumb_key (const KeyEvent& key)
{
    if (is_left_thumb_key (key) || is_right_thumb_key (key))
        return true;

    return false;
}

bool
NicolaConvertor::is_left_thumb_key (const KeyEvent& key)
{
    return util_match_key_event (m_anthy.get_config()->m_left_thumb_keys,
                                 key,
                                 0xFFFF);
}

bool
NicolaConvertor::is_right_thumb_key (const KeyEvent& key)
{
    return util_match_key_event (m_anthy.get_config()->m_right_thumb_keys,
                                 key,
                                 0xFFFF);
}

NicolaShiftType
NicolaConvertor::get_thumb_key_type (const KeyEvent& key)
{
    if (is_left_thumb_key (key))
        return FCITX_ANTHY_NICOLA_SHIFT_LEFT;
    else if (is_right_thumb_key (key))
        return FCITX_ANTHY_NICOLA_SHIFT_RIGHT;
    else
        return FCITX_ANTHY_NICOLA_SHIFT_NONE;
}

bool
NicolaConvertor::emit_key_event (const KeyEvent & key)
{
    m_through_key_event = key;

    //m_anthy.forward_key_event (key);
    return m_anthy.process_key_event (key);
}

void
NicolaConvertor::process_timeout (void)
{
    m_processing_timeout = true;
    if (!m_prev_char_key.empty ())
        m_anthy.process_key_event (m_prev_char_key);
    else if (!m_prev_thumb_key.empty())
        m_anthy.process_key_event (m_prev_thumb_key);
    m_processing_timeout = false;
}

void
NicolaConvertor::set_alarm (int time_msec)
{
    if (time_msec < 5)
        time_msec = 5;
    if (time_msec > 1000)
        time_msec = 1000;

    FcitxInstanceAddTimeout (m_anthy.get_owner(),
                             time_msec,
                             NicolaTimeoutFunc,
                             (void *) this);
}

bool
NicolaConvertor::stop()
{
    return FcitxInstanceRemoveTimeoutByFunc(m_anthy.get_owner(), NicolaTimeoutFunc);
}

int
NicolaConvertor::thumb_key(const KeyEvent& key)
{
    return is_left_thumb_key(key) ? 1 : 2;
}

bool
NicolaConvertor::append (const KeyEvent & key,
                         std::string & result,
                         std::string & pending,
                         std::string &raw)
{
    if (m_processing_timeout) {
        search (m_prev_char_key,
                get_thumb_key_type (m_prev_thumb_key),
                result, raw);
        if (m_prev_thumb_key.empty ()) {
            m_prev_char_key  = KeyEvent ();
            m_prev_thumb_key = KeyEvent ();
        } else {
            m_repeat_char_key  = m_prev_char_key;
            m_repeat_thumb_key = m_prev_thumb_key;
        }
        return handle_voiced_consonant (result, pending);
    }

    if (!key.is_release && util_key_is_keypad (key)) {
        util_keypad_to_string (raw, key);

        // convert key pad string to wide
        std::string wide;
        TenKeyType ten_key_type = m_anthy.get_config()->m_ten_key_type;
        if ((ten_key_type == FCITX_ANTHY_TEN_KEY_TYPE_FOLLOWMODE &&
             (m_anthy.get_input_mode () == FCITX_ANTHY_MODE_LATIN ||
              m_anthy.get_input_mode () == FCITX_ANTHY_MODE_HALF_KATAKANA)) ||
            ten_key_type == FCITX_ANTHY_TEN_KEY_TYPE_HALF)
        {
            wide = raw;
        } else {
            util_convert_to_wide (wide, raw);
        }

        result = wide;

        m_prev_char_key = m_repeat_char_key  = KeyEvent ();
        m_prev_thumb_key = m_repeat_thumb_key = KeyEvent ();

        return handle_voiced_consonant (result, pending);
    }

    if (key.is_release) {
        if (key == m_prev_char_key) {
            if (stop()) {
                search (m_prev_char_key, get_thumb_key_type(m_prev_thumb_key), result, raw);
            }
            m_prev_char_key = KeyEvent();
        }
        else if (get_thumb_key_type(key) == get_thumb_key_type(m_prev_thumb_key)) {
            if (stop())
                emit_key_event(m_prev_thumb_key);
            m_prev_thumb_key = KeyEvent();
        }
        if (is_thumb_key(key))
            m_repeat_thumb_key = KeyEvent();
        else if (key == m_repeat_char_key) {
            m_repeat_char_key = KeyEvent();
        }
    }
    else if (is_thumb_key(key)) {
        if (!m_repeat_thumb_key.empty() && !(key == m_repeat_thumb_key)) {
            stop();
            m_prev_char_key = m_prev_thumb_key = m_repeat_char_key = m_repeat_thumb_key = KeyEvent();
        }
        if (!m_prev_thumb_key.empty()) {
            stop();
            emit_key_event(m_prev_thumb_key);
            m_prev_thumb_key = key;
            set_alarm(m_anthy.get_config()->m_nicola_time);
        }
        else if (!m_prev_char_key.empty()) {
            stop();
            m_repeat_char_key = m_prev_char_key;
            m_repeat_thumb_key = key;
            search (m_prev_char_key, get_thumb_key_type(m_repeat_thumb_key), result, raw);
        }
        else {
            if (get_thumb_key_type(m_repeat_thumb_key) == get_thumb_key_type(key)) {
                if (!m_repeat_char_key.empty()) {
                    search (m_repeat_char_key, get_thumb_key_type(m_repeat_thumb_key), result, raw);
                }
            }
            else {
                m_prev_thumb_key = key;
                set_alarm(m_anthy.get_config()->m_nicola_time);
            }
        }
    }
    else if (is_char_key (key)) {
        if (!m_repeat_char_key.empty() && !(key == m_repeat_char_key)) {
            stop();
            m_prev_char_key = m_prev_thumb_key = m_repeat_char_key = m_repeat_thumb_key = KeyEvent();
        }
        if (!m_prev_char_key.empty()) {
            stop();
            search (m_prev_char_key, get_thumb_key_type(m_prev_thumb_key), result, raw);
            set_alarm(m_anthy.get_config()->m_nicola_time);
            m_prev_char_key = key;
        }
        else if (is_thumb_key(m_prev_thumb_key)) {
            stop();
            m_repeat_char_key = key;
            m_repeat_thumb_key = m_prev_thumb_key;
            search (key, get_thumb_key_type(m_prev_thumb_key), result, raw);
        }
        else {
            if (key == m_repeat_char_key) {
                if (!m_repeat_thumb_key.empty()) {
                    search (m_repeat_char_key, get_thumb_key_type(m_repeat_thumb_key), result, raw);
                }
            }
            else {
                set_alarm(m_anthy.get_config()->m_nicola_time);
                m_prev_char_key = key;
            }
        }
    }
    else {
        if (!m_prev_char_key.empty()) {
            stop();
            search (m_prev_char_key, get_thumb_key_type(m_prev_thumb_key), result, raw);
        }
        else if (!m_prev_thumb_key.empty()) {
            stop();
            emit_key_event(m_prev_thumb_key);
        }
        if (emit_key_event(key))
            return true;
    }

    FcitxLog(DEBUG, "prev:%s %d %d %d", __func__, m_prev_char_key.sym, m_prev_char_key.state, m_prev_char_key.is_release);

    return handle_voiced_consonant (result, pending);
}

bool
NicolaConvertor::append (const std::string & str,
                         std::string   & result,
                         std::string   & pending)
{
    result = str;
    m_pending = std::string ();

    return false;
}

void
NicolaConvertor::clear (void)
{
    m_pending = std::string ();
    m_prev_char_key = KeyEvent();
    m_prev_thumb_key = KeyEvent();
    m_repeat_char_key = KeyEvent();
    m_repeat_thumb_key = KeyEvent();
}

bool
NicolaConvertor::is_pending (void)
{
    return !m_pending.empty ();
}

std::string
NicolaConvertor::get_pending (void)
{
    return m_pending;
}

std::string
NicolaConvertor::flush_pending (void)
{
    return std::string ();
}

void
NicolaConvertor::reset_pending (const std::string & result, const std::string & raw)
{
    VoicedConsonantRule *table = fcitx_anthy_voiced_consonant_table;

    m_pending = std::string ();

    for (unsigned int i = 0; table[i].string; i++) {
        if (result == table[i].string) {
            m_pending = result;
            return;
        }
    }
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
