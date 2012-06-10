/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004-2005 Takuro Ashie
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

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <string.h>

#include "factory.h"
#include "imengine.h"
#include "preedit.h"
#include "utils.h"
#include <fcitx/candidate.h>

static ConvRule *get_period_rule              (TypingMethod method,
                                               PeriodStyle  period);
static ConvRule *get_comma_rule               (TypingMethod method,
                                               CommaStyle   period);

Preedit::Preedit (AnthyInstance &anthy)
    : m_anthy              (anthy),
      //m_key2kana_tables    (tables),
      m_reading            (anthy),
      m_conversion         (m_anthy, m_reading),
      m_input_mode         (FCITX_ANTHY_MODE_HIRAGANA)
{
}

Preedit::~Preedit ()
{
}


/*
 * getting status
 */
unsigned int
Preedit::get_length (void)
{
    if (is_converting ())
        return m_conversion.get_length ();
    else
        return m_reading.get_length ();

    return 0;
}


/*
 * getting status
 */
unsigned int
Preedit::get_length_by_char (void)
{
    if (is_converting ())
        return m_conversion.get_length_by_char ();
    else
        return m_reading.get_length_by_char ();

    return 0;
}

std::string
Preedit::get_string (void)
{
    if (is_converting ()) {
        return m_conversion.get ();
    } else if (!m_source.empty ()) {
        return m_source;
    } else {
        std::string widestr;
        switch (m_input_mode) {
        case FCITX_ANTHY_MODE_KATAKANA:
            util_convert_to_katakana (widestr, m_reading.get_by_char ());
            return widestr;

        case FCITX_ANTHY_MODE_HALF_KATAKANA:
            util_convert_to_katakana (widestr, m_reading.get_by_char (), true);
            return widestr;

        case FCITX_ANTHY_MODE_LATIN:
            return m_reading.get_raw_by_char ();

        case FCITX_ANTHY_MODE_WIDE_LATIN:
            util_convert_to_wide (widestr, m_reading.get_raw_by_char ());
            return widestr;

        case FCITX_ANTHY_MODE_HIRAGANA:
        default:
            return m_reading.get_by_char ();
        }
    }

    return std::string ();
}

void
Preedit::update_preedit (void)
{
    if (is_converting ())
    {
        m_conversion.update_preedit ();
    } else {
        FcitxMessages* preedit;
        if (m_anthy.support_client_preedit())
            preedit = m_anthy.get_client_preedit();
        else
            preedit = m_anthy.get_preedit();

        std::string s = get_string();
        if (s.length() > 0)
            FcitxMessagesAddMessageAtLast(preedit, MSG_INPUT, "%s", s.c_str());
    }
}

Reading &
Preedit::get_reading (void)
{
    return m_reading;
}

bool
Preedit::is_preediting (void)
{
    if (m_reading.get_length () > 0 ||
        m_conversion.is_converting () ||
        !m_source.empty ())
    {
        return true;
    } else {
        return false;
    }
}

bool
Preedit::is_converting (void)
{
    return m_conversion.is_converting ();
}

bool
Preedit::is_predicting (void)
{
    return m_conversion.is_predicting ();
}

bool
Preedit::is_reconverting (void)
{
    return !m_source.empty ();
}


/*
 * manipulating the preedit string
 */
bool
Preedit::can_process_key_event (const KeyEvent & key)
{
    return m_reading.can_process_key_event (key);
}

bool
Preedit::process_key_event (const KeyEvent & key)
{
    if (!m_reading.can_process_key_event (key))
        return false;

    bool retval = m_reading.process_key_event (key);

    if (m_input_mode == FCITX_ANTHY_MODE_LATIN ||
        m_input_mode == FCITX_ANTHY_MODE_WIDE_LATIN)
    {
        return true;
    }

    // auto convert
    unsigned int len = m_reading.get_length_by_char ();
    if (len > 0) {
        std::string str;
        str = m_reading.get_raw_by_char (len - 1, 1);
        if (is_comma_or_period (str)) {
            if (m_anthy.get_config()->m_behavior_on_period == FCITX_ANTHY_PERIOD_BEHAVIOR_CONVERT &&
                get_length () > 1)
            {
                convert ();
            } else if (m_anthy.get_config()->m_behavior_on_period == FCITX_ANTHY_PERIOD_BEHAVIOR_COMMIT) {
                return true;
            }
        }
    }

    return retval;
}

bool
Preedit::append (const KeyEvent & key,
                 const std::string   & string)
{
    return m_reading.append (key, string);
}

void
Preedit::erase (bool backward)
{
    if (m_reading.get_length_by_char () <= 0)
        return;

    // cancel conversion
    revert ();

    // erase
    TypingMethod method = get_typing_method ();
    bool allow_split
        = method == FCITX_ANTHY_TYPING_METHOD_ROMAJI &&
          m_anthy.get_config()->m_romaji_allow_split;
    if (backward && m_reading.get_caret_pos_by_char () == 0)
        return;
    if (!backward && m_reading.get_caret_pos_by_char () >= m_reading.get_length_by_char ())
        return;
    if (backward)
        m_reading.move_caret (-1, allow_split);
    m_reading.erase (m_reading.get_caret_pos_by_char (), 1, allow_split);
}

void
Preedit::finish (void)
{
    m_reading.finish ();
}


/*
 * manipulating conversion string
 */
void
Preedit::convert (CandidateType type, bool single_segment)
{
    if (m_source.empty ())
        m_conversion.convert (type, single_segment);
    else
        m_conversion.convert (m_source, single_segment);
}

void
Preedit::convert (const std::string &source, bool single_segment)
{
    m_conversion.convert (source, single_segment);
    m_source = source;
}

void
Preedit::revert (void)
{
    m_conversion.clear ();
}

void
Preedit::commit (int segment_id, bool learn)
{
    if (m_conversion.is_converting ())
        m_conversion.commit (segment_id, learn);
    if (!m_conversion.is_converting ())
        clear ();
}

int
Preedit::get_nr_segments (void)
{
    return m_conversion.get_nr_segments ();
}

std::string
Preedit::get_segment_string (int segment_id)
{
    return m_conversion.get_segment_string (segment_id);
}

int
Preedit::get_selected_segment (void)
{
    return m_conversion.get_selected_segment ();
}

void
Preedit::select_segment (int segment_id)
{
    m_conversion.select_segment (segment_id);
}

int
Preedit::get_segment_size (int segment_id)
{
    return m_conversion.get_segment_size (segment_id);
}

void
Preedit::resize_segment (int relative_size, int segment_id)
{
    m_conversion.resize_segment (relative_size, segment_id);
}


/*
 * candidates for a segment
 */
void
Preedit::get_candidates (FcitxCandidateWordList *table, int segment_id)
{
    m_conversion.get_candidates (table, segment_id);
}

int
Preedit::get_selected_candidate (int segment_id)
{
    return m_conversion.get_selected_candidate (segment_id);
}

void
Preedit::select_candidate (int candidate_id, int segment_id)
{
    m_conversion.select_candidate (candidate_id, segment_id);
}

/*
 * manipulating the caret
 */
unsigned int
Preedit::get_caret_pos (void)
{
    if (is_converting ()) {
        return m_conversion.get_segment_position ();
    } else {
        if (get_input_mode () == FCITX_ANTHY_MODE_HALF_KATAKANA) {
            // FIXME! It's ad-hoc
            std::string substr;
            substr = m_reading.get_by_char (0, m_reading.get_caret_pos_by_char (),
                                    FCITX_ANTHY_STRING_HALF_KATAKANA);
            return substr.length ();
        } else {
            return m_reading.get_caret_pos ();
        }
    }
}

void
Preedit::set_caret_pos_by_char (unsigned int pos)
{
    if (is_converting ())
        return;

    m_reading.set_caret_pos_by_char (pos);
}

void
Preedit::move_caret (int step)
{
    if (is_converting ())
        return;

    TypingMethod method = get_typing_method ();
    bool allow_split
        = method == FCITX_ANTHY_TYPING_METHOD_ROMAJI &&
          m_anthy.get_config()->m_romaji_allow_split;

    m_reading.move_caret (step, allow_split);
}

void
Preedit::predict (void)
{
    m_conversion.predict ();
}


/*
 * clear all string
 */
void
Preedit::clear (int segment_id)
{
    // FIXME! We should add implementation not only for conversion string but
    // also for reading string.

    if (!is_converting ()) {
        m_reading.clear ();
        m_conversion.clear ();
        m_source = std::string ();
        return;
    }

    m_conversion.clear (segment_id);
    if (m_conversion.get_nr_segments () <= 0) {
        m_reading.clear ();
        m_source = std::string ();
    }
}


/*
 * preference
 */
void
Preedit::set_input_mode (InputMode mode)
{
    m_input_mode = mode;
}

InputMode
Preedit::get_input_mode (void)
{
    return m_input_mode;
}

void
Preedit::set_typing_method (TypingMethod method)
{
    m_reading.set_typing_method (method);
}

TypingMethod
Preedit::get_typing_method (void)
{
    return m_reading.get_typing_method ();
}

void
Preedit::set_period_style (PeriodStyle style)
{
    m_reading.set_period_style (style);
}

PeriodStyle
Preedit::get_period_style (void)
{
    return m_reading.get_period_style ();
}

void
Preedit::set_comma_style (CommaStyle style)
{
    m_reading.set_comma_style (style);
}

CommaStyle
Preedit::get_comma_style (void)
{
    return m_reading.get_comma_style ();
}

void
Preedit::set_bracket_style (BracketStyle style)
{
    m_reading.set_bracket_style (style);
}

BracketStyle
Preedit::get_bracket_style (void)
{
    return m_reading.get_bracket_style ();
}

void
Preedit::set_slash_style (SlashStyle style)
{
    m_reading.set_slash_style (style);
}

SlashStyle
Preedit::get_slash_style (void)
{
    return m_reading.get_slash_style ();
}

void
Preedit::set_symbol_width (bool half)
{
    m_reading.set_symbol_width (half);
}

bool
Preedit::get_symbol_width (void)
{
    return m_reading.get_symbol_width ();
}

void
Preedit::set_number_width (bool half)
{
    m_reading.set_number_width (half);
}

bool
Preedit::get_number_width (void)
{
    return m_reading.get_number_width ();
}

void
Preedit::set_pseudo_ascii_mode (int mode)
{
    m_reading.set_pseudo_ascii_mode (mode);
}

bool
Preedit::is_pseudo_ascii_mode (void)
{
    return m_reading.is_pseudo_ascii_mode ();
}

void
Preedit::reset_pseudo_ascii_mode (void)
{
    m_reading.reset_pseudo_ascii_mode ();
}

bool
Preedit::is_comma_or_period (const std::string & str)
{
    TypingMethod typing = get_typing_method ();
    PeriodStyle  period = get_period_style ();
    CommaStyle   comma  = get_comma_style ();

    ConvRule *period_rule = get_period_rule (typing, period);
    ConvRule *comma_rule  = get_comma_rule  (typing, comma);

    for (unsigned int i = 0; period_rule && period_rule[i].string; i++) {
        if (period_rule[i].string &&
            !strcmp (period_rule[i].string, str.c_str ()))
        {
            return true;
        }
    }
    for (unsigned int i = 0; comma_rule && comma_rule[i].string; i++) {
        if (comma_rule[i].string &&
            !strcmp (comma_rule[i].string, str.c_str ()))
        {
            return true;
        }
    }

    return false;
}


/*
 * utilities
 */
static ConvRule *
get_period_rule (TypingMethod method, PeriodStyle period)
{
    switch (method) {
    case FCITX_ANTHY_TYPING_METHOD_KANA:
        switch (period) {
        case FCITX_ANTHY_PERIOD_WIDE:
            return fcitx_anthy_kana_wide_period_rule;
        case FCITX_ANTHY_PERIOD_HALF:
            return fcitx_anthy_kana_half_period_rule;
        case FCITX_ANTHY_PERIOD_JAPANESE:
        default:
            return fcitx_anthy_kana_ja_period_rule;
        };
        break;

    case FCITX_ANTHY_TYPING_METHOD_ROMAJI:
    default:
        switch (period) {
        case FCITX_ANTHY_PERIOD_WIDE:
            return fcitx_anthy_romaji_wide_period_rule;
        case FCITX_ANTHY_PERIOD_HALF:
            return fcitx_anthy_romaji_half_period_rule;
        case FCITX_ANTHY_PERIOD_JAPANESE:
        default:
            return fcitx_anthy_romaji_ja_period_rule;
        };
        break;
    };

    return NULL;
}

static ConvRule *
get_comma_rule (TypingMethod method, CommaStyle period)
{
    switch (method) {
    case FCITX_ANTHY_TYPING_METHOD_KANA:
        switch (period) {
        case FCITX_ANTHY_PERIOD_WIDE:
            return fcitx_anthy_kana_wide_comma_rule;
        case FCITX_ANTHY_PERIOD_HALF:
            return fcitx_anthy_kana_half_comma_rule;
        case FCITX_ANTHY_PERIOD_JAPANESE:
        default:
            return fcitx_anthy_kana_ja_comma_rule;
        };
        break;

    case FCITX_ANTHY_TYPING_METHOD_ROMAJI:
    default:
        switch (period) {
        case FCITX_ANTHY_PERIOD_WIDE:
            return fcitx_anthy_romaji_wide_comma_rule;
        case FCITX_ANTHY_PERIOD_HALF:
            return fcitx_anthy_romaji_half_comma_rule;
        case FCITX_ANTHY_PERIOD_JAPANESE:
        default:
            return fcitx_anthy_romaji_ja_comma_rule;
        };
        break;
    };

    return NULL;
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
