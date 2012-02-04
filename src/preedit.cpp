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

#include "preedit.h"
#include "utils.h"
#include "eim.h"

Preedit::Preedit (FcitxAnthy* anthy)
    : m_anthy              (anthy),
      //m_key2kana_tables    (tables),
      m_reading            (anthy),
      m_conversion         (m_anthy, m_reading),
      m_input_mode         (SCIM_ANTHY_MODE_HIRAGANA)
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
        case SCIM_ANTHY_MODE_KATAKANA:
            util_convert_to_katakana (widestr, m_reading.get ());
            return widestr;

        case SCIM_ANTHY_MODE_HALF_KATAKANA:
            util_convert_to_katakana (widestr, m_reading.get (), true);
            return widestr;

        case SCIM_ANTHY_MODE_HIRAGANA:
        default:
            return m_reading.get ();
        }
    }

    return std::string ();
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
Preedit::can_process_key_event (FcitxKeySym sym, unsigned int state)
{
    return m_reading.can_process_key_event (sym, state);
}

bool
Preedit::process_key_event (FcitxKeySym sym, unsigned int state)
{
    if (!m_reading.can_process_key_event (sym, state))
        return false;

    bool retval = m_reading.process_key_event (sym, state);

    // auto convert
    unsigned int len = m_reading.get_length ();
    if (len > 0) {
    }

    return retval;
}

bool
Preedit::append (FcitxKeySym sym, unsigned int state,
                 const std::string   & string)
{
    return m_reading.append (sym, state, string);
}

void
Preedit::erase (bool backward)
{
    if (m_reading.get_length () <= 0)
        return;

    // cancel conversion
    revert ();

    // erase
    TypingMethod method = get_typing_method ();
    bool allow_split
        = method == SCIM_ANTHY_TYPING_METHOD_ROMAJI &&
          m_anthy->config.m_romaji_allow_split;
    if (backward && m_reading.get_caret_pos () == 0)
        return;
    if (!backward && m_reading.get_caret_pos () >= m_reading.get_length ())
        return;
    if (backward)
        m_reading.move_caret (-1, allow_split);
    m_reading.erase (m_reading.get_caret_pos (), 1, allow_split);
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
Preedit::get_candidates (struct _FcitxCandidateWordList *table, int segment_id)
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
        if (get_input_mode () == SCIM_ANTHY_MODE_HALF_KATAKANA) {
            // FIXME! It's ad-hoc
            std::string substr;
            substr = m_reading.get (0, m_reading.get_caret_pos (),
                                    SCIM_ANTHY_STRING_HALF_KATAKANA);
            return substr.length ();
        } else {
            return m_reading.get_caret_pos ();
        }
    }
}

void
Preedit::set_caret_pos (unsigned int pos)
{
    if (is_converting ())
        return;

    m_reading.set_caret_pos (pos);
}

void
Preedit::move_caret (int step)
{
    if (is_converting ())
        return;

    TypingMethod method = get_typing_method ();
    bool allow_split
        = method == SCIM_ANTHY_TYPING_METHOD_ROMAJI &&
          m_anthy->config.m_romaji_allow_split;

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


/*
vi:ts=4:nowrap:ai:expandtab
*/
