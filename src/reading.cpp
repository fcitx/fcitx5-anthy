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

#include "reading.h"
#include "utils.h"
#include "eim.h"

ReadingSegment::ReadingSegment ()
{
}

ReadingSegment::~ReadingSegment ()
{
}

static const char *
find_romaji (std::string c)
{
    ConvRule *table = scim_anthy_romaji_typing_rule;

    for (unsigned int i = 0; table[i].string; i++) {
        std::string kana = table[i].result;
        if (c == kana)
            return table[i].string;
    }

    return "";
}

static void
to_half (std::string &dest, std::string &src)
{
    WideRule *table = scim_anthy_wide_table;
    
    for (unsigned int i = 0; i < src.size (); i++) {
        bool found = false;
        std::string kana1 = src.substr (i, 1);
        for (unsigned int i = 0; table[i].code; i++) {
            std::string kana2 = table[i].wide;
            if (kana1 == kana2) {
                dest += table[i].code;
                found = true;
                break;
            }
        }
        if (!found)
            dest += kana1;
    }
}

// Only a romaji string can be splited with raw key string.
// Other typing method aren't supported splitting raw key string.
void
ReadingSegment::split (ReadingSegments &segments)
{
    if (kana.length () <= 1)
        segments.push_back (*this);

    std::string half;
    to_half (half, kana);
    bool same_with_raw = half == raw;

    std::string::iterator it;
    for (unsigned int i = 0; i < kana.size (); i++) {
        std::string c = kana.substr (i, 1);
        ReadingSegment seg;
        seg.kana = c;
        if (same_with_raw)
            to_half (seg.raw, c);
        else
            seg.raw = find_romaji (c);
        segments.push_back (seg);
    }
}



Reading::Reading (FcitxAnthy *anthy)
    : m_anthy           (anthy),
      //m_key2kana_tables (tables),
      m_key2kana_normal (anthy, m_key2kana_tables),
      m_kana            (anthy),
      m_key2kana        (&m_key2kana_normal),
      m_segment_pos     (0),
      m_caret_offset    (0)
{
}

Reading::~Reading ()
{
}

bool
Reading::can_process_key_event (FcitxKeySym sym, unsigned int state)
{
    if (m_kana.can_append (sym, state))
        return true;

    return m_key2kana->can_append (sym, state);
}

bool
Reading::process_key_event (FcitxKeySym sym, unsigned int state)
{
    if (!can_process_key_event (sym, state))
        return false;

    if (m_caret_offset != 0) {
        split_segment (m_segment_pos);
        reset_pending ();
    }

    bool was_pending;
    if (m_kana.can_append (sym, state))
        was_pending = m_kana.is_pending ();
    else
        was_pending = m_key2kana->is_pending ();

    std::string raw;
    std::string result, pending;
    bool need_commiting;
    if (m_kana.can_append (sym, state))
        need_commiting = m_kana.append (sym, state, result, pending, raw);
    else
        need_commiting = m_key2kana->append (sym, state, result, pending, raw);

    ReadingSegments::iterator begin = m_segments.begin ();

    // fix previous segment and prepare next segment if needed
    if (!result.empty () || !pending.empty ()) {
        if (!was_pending ||  // previous segment was already fixed
            need_commiting)  // previous segment has been fixed
        {
            ReadingSegment c;
            m_segments.insert (begin + m_segment_pos, c);
            m_segment_pos++;
        }
    }

    // fill segment
    if (result.length() > 0 && pending.length () > 0) {
        m_segments[m_segment_pos - 1].kana = result;

        ReadingSegment c;
        c.raw += raw;
        c.kana = pending;
        m_segments.insert (begin + m_segment_pos, c);
        m_segment_pos++;

    } else if (result.length () > 0) {
        m_segments[m_segment_pos - 1].raw += raw;
        m_segments[m_segment_pos - 1].kana = result;

    } else if (pending.length () > 0) {
        m_segments[m_segment_pos - 1].raw += raw;
        m_segments[m_segment_pos - 1].kana = pending;

    } else {

    }

    return false;
}

void
Reading::finish (void)
{
    if (!m_key2kana->is_pending ()) return;

    std::string result;
    result = m_key2kana->flush_pending ();
    if (result.length () > 0)
        m_segments[m_segment_pos - 1].kana = result;
}

void
Reading::clear (void)
{
    m_key2kana_normal.clear ();
    m_kana.clear ();
    m_segments.clear ();
    m_segment_pos  = 0;
    m_caret_offset = 0;
}

std::string
Reading::get (unsigned int start, int len, StringType type)
{
    std::string str;
    unsigned int pos = 0, end = len > 0 ? start + len : get_length () - start;
    std::string kana;
    std::string raw;

    if (start >= end)
        return str;
    if (start >= get_length ())
        return str;

    switch (type) {
    case SCIM_ANTHY_STRING_LATIN:
        raw = get_raw (start, len);
        str = raw;
        return str;

    case SCIM_ANTHY_STRING_WIDE_LATIN:
        raw = get_raw (start, len);
        util_convert_to_wide (str, raw);
        return str;

    default:
        break;
    }

    for (unsigned int i = 0; i < m_segments.size (); i++) {
        if (pos >= start || pos + m_segments[i].kana.length () > start) {
            unsigned int startstart = 0, len;

            if (pos >= start)
                startstart = 0;
            else
                startstart = pos - start;

            if (pos + m_segments[i].kana.length () > end)
                len = end - start;
            else
                len = m_segments[i].kana.length ();

            kana += m_segments[i].kana.substr (startstart, len);
        }

        pos += m_segments[i].kana.length ();
        if (pos >= end)
            break;
    }

    switch (type) {
    case SCIM_ANTHY_STRING_HIRAGANA:
        str = kana;
        break;

    case SCIM_ANTHY_STRING_KATAKANA:
        util_convert_to_katakana (str, kana);
        break;

    case SCIM_ANTHY_STRING_HALF_KATAKANA:
        util_convert_to_katakana (str, kana, true);
        break;

    default:
        break;
    }

    return str;
}

std::string
Reading::get_raw (unsigned int start, int len)
{
    std::string str;
    unsigned int pos = 0, end = len > 0 ? start + len : get_length () - start;

    if (start >= end)
        return str;

    for (unsigned int i = 0; i < m_segments.size (); i++) {
        if (pos >= start || pos + m_segments[i].kana.length () > start) {
            // FIXME!
            str += m_segments[i].raw;
        }

        pos += m_segments[i].kana.length ();

        if (pos >= end)
            break;
    }

    return str;
}

void
Reading::split_segment (unsigned int seg_id)
{
    if (seg_id >= m_segments.size ())
        return;

    unsigned int pos = 0;
    for (unsigned int i = 0; i < seg_id && i < m_segments.size (); i++)
        pos += m_segments[i].kana.length ();

    unsigned int caret = get_caret_pos ();
    unsigned int seg_len = m_segments[seg_id].kana.length ();
    bool caret_was_in_the_segment = false;
    if (caret > pos && caret < pos + seg_len)
        caret_was_in_the_segment = true;

    ReadingSegments segments;
    m_segments[seg_id].split (segments);
    m_segments.erase (m_segments.begin () + seg_id);
    for (int j = segments.size () - 1; j >= 0; j--) {
        m_segments.insert (m_segments.begin () + seg_id, segments[j]);
        if (m_segment_pos > seg_id)
            m_segment_pos++;
    }

    if (caret_was_in_the_segment) {
        m_segment_pos += m_caret_offset;
        m_caret_offset = 0;
    }
}

bool
Reading::append (FcitxKeySym sym, unsigned int state,
                 const std::string   & string)
{
    bool was_pending;
    std::string result, pending;
    bool need_commiting;

    if (!m_kana.can_append (sym, state, true) &&
        !m_key2kana->can_append (sym, state, true))
        return false;

    if (m_caret_offset != 0) {
        split_segment (m_segment_pos);
        reset_pending ();
    }

    if (m_kana.can_append (sym, state))
        was_pending = m_kana.is_pending ();
    else
        was_pending = m_key2kana->is_pending ();

    if (m_kana.can_append (sym, state))
        need_commiting = m_kana.append (string, result, pending);
    else
        need_commiting = m_key2kana->append (string, result, pending);

    ReadingSegments::iterator begin = m_segments.begin ();

    // fix previous segment and prepare next segment if needed
    if (!result.empty () || !pending.empty ()) {
        if (!was_pending ||  // previous segment was already fixed
            need_commiting)  // previous segment has been fixed
        {
            ReadingSegment c;
            m_segments.insert (begin + m_segment_pos, c);
            m_segment_pos++;
        }
    }

    // fill segment
    if (result.length() > 0 && pending.length () > 0) {
        m_segments[m_segment_pos - 1].kana = result;

        ReadingSegment c;
        c.raw += string;
        c.kana = pending;
        m_segments.insert (begin + m_segment_pos, c);
        m_segment_pos++;

    } else if (result.length () > 0) {
        m_segments[m_segment_pos - 1].raw += string;
        m_segments[m_segment_pos - 1].kana = result;

    } else if (pending.length () > 0) {
        m_segments[m_segment_pos - 1].raw += string;
        m_segments[m_segment_pos - 1].kana = pending;

    } else {

    }

    return false;
}

void
Reading::erase (unsigned int start, int len, bool allow_split)
{
    if (m_segments.size () <= 0)
        return;

    if (get_length () < start)
        return;

    if (len < 0)
        len = get_length () - start;

    // erase
    unsigned int pos = 0;
    for (int i = 0; i <= (int) m_segments.size (); i++) {
        if (pos < start) {
            // we have not yet reached start position.

            if (i == (int) m_segments.size ())
                break;

            pos += m_segments[i].kana.length ();

        } else if (pos == start) {
            // we have reached start position.

            if (i == (int) m_segments.size ())
                break;

            if (allow_split &&
                pos + m_segments[i].kana.length () > start + len)
            {
                // we have overshooted the end position!
                // we have to split this segment
                split_segment (i);

            } else {
                // This segment is completely in the rage, erase it!
                len -= m_segments[i].kana.length ();
                m_segments.erase (m_segments.begin () + i);
                if ((int) m_segment_pos > i)
                    m_segment_pos--;
            }

            // retry from the same position
            i--;

        } else {
            // we have overshooted the start position!

            if (allow_split) {
                pos -= m_segments[i - 1].kana.length ();
                split_segment (i - 1);

                // retry from the previous position
                i -= 2;

            } else {
                // we have overshooted the start position, but have not been
                // allowed to split the segment.
                // So remove all string of previous segment.
                len -= pos - start;
                pos -= m_segments[i - 1].kana.length ();
                m_segments.erase (m_segments.begin () + i - 1);
                if ((int) m_segment_pos > i - 1)
                    m_segment_pos--;

                // retry from the previous position
                i -= 2;
            }
        }

        // Now all letters in the range are removed.
        // Exit the loop.
        if (len <= 0)
            break;
    }

    // reset values
    if (m_segments.size () <= 0) {
        clear ();
    } else {
        reset_pending ();
    }
}

void
Reading::reset_pending (void)
{
    if (m_key2kana->is_pending ())
        m_key2kana->clear ();
    if (m_kana.is_pending ())
        m_kana.clear ();

    if (m_segment_pos <= 0)
        return;

    m_key2kana->reset_pending (m_segments[m_segment_pos - 1].kana,
                               m_segments[m_segment_pos - 1].raw);
    m_kana.reset_pending (m_segments[m_segment_pos - 1].kana,
                          m_segments[m_segment_pos - 1].raw);

    // FIXME! this code breaks pending state on normal input mode.
    m_key2kana->reset_pseudo_ascii_mode();
    for (unsigned int i = 0; i < m_segment_pos; i++)
        m_key2kana->process_pseudo_ascii_mode(m_segments[i].kana);
}

unsigned int
Reading::get_length (void)
{
    unsigned int len = 0;
    for (unsigned int i = 0; i < m_segments.size (); i++)
        len += m_segments[i].kana.length();
    return len;
}

unsigned int
Reading::get_caret_pos (void)
{
    unsigned int pos = 0;

    for (unsigned int i = 0;
         i < m_segment_pos && i < m_segments.size ();
         i++)
    {
        pos += m_segments[i].kana.length();
    }

    pos += m_caret_offset;

    return pos;
}

// FIXME! add "allow_split" argument.
void
Reading::set_caret_pos (unsigned int pos)
{
    if (pos == get_caret_pos ())
        return;

    m_key2kana->clear ();
    m_kana.clear ();

    if (pos >= get_length ()) {
        m_segment_pos = m_segments.size ();

    } else if (pos == 0 ||  m_segments.size () <= 0) {
        m_segment_pos = 0;

    } else {
        unsigned int i, tmp_pos = 0;

        for (i = 0; tmp_pos <= pos; i++)
            tmp_pos += m_segments[i].kana.length();

        if (tmp_pos == pos) {
            m_segment_pos = i + 1;
        } else if (tmp_pos < get_caret_pos ()) {
            m_segment_pos = i;
        } else if (tmp_pos > get_caret_pos ()) {
            m_segment_pos = i + 1;
        }
    }

    reset_pending ();
}

void
Reading::move_caret (int step, bool allow_split)
{
    if (step == 0)
        return;

    m_key2kana->clear ();
    m_kana.clear ();

    if (allow_split) {
        unsigned int pos = get_caret_pos ();
        if (step < 0 && pos < (int) abs (step)) {
            // lower limit
            m_segment_pos = 0;

        } else if (step > 0 && pos + step > get_length ()) {
            // upper limit
            m_segment_pos = m_segments.size ();

        } else {
            unsigned int new_pos = pos + step;
            ReadingSegments::iterator it;
            pos = 0;
            m_segment_pos = 0;
            m_caret_offset = 0;
            for (it = m_segments.begin (); pos < new_pos; it++) {
                if (pos + it->kana.length () > new_pos) {
                    m_caret_offset = new_pos - pos;
                    break;
                } else {
                    m_segment_pos++;
                    pos += it->kana.length ();
                }
            }
        }

    } else {
        if (step < 0 && m_segment_pos < (int) abs (step)) {
            // lower limit
            m_segment_pos = 0;

        } else if (step > 0 && m_segment_pos + step > m_segments.size ()) {
            // upper limit
            m_segment_pos = m_segments.size ();

        } else {
            // other
            m_segment_pos += step;
        }
    }

    reset_pending ();
}

void
Reading::set_typing_method (TypingMethod method)
{
    Key2KanaTable *fundamental_table = NULL;
    if (method == SCIM_ANTHY_TYPING_METHOD_KANA) {
        fundamental_table = m_anthy->config.m_custom_kana_table;
        m_key2kana = &m_key2kana_normal;
        m_key2kana_tables.set_typing_method (method, fundamental_table);
        m_key2kana_normal.set_case_sensitive (true);
    } else {
        fundamental_table = m_anthy->config.m_custom_romaji_table;
        m_key2kana = &m_key2kana_normal;
        m_key2kana_tables.set_typing_method (method, fundamental_table);
        m_key2kana_normal.set_case_sensitive (false);
    }
}

TypingMethod
Reading::get_typing_method (void)
{
    return m_key2kana_tables.get_typing_method ();
}

void
Reading::set_symbol_width (bool half)
{
    m_key2kana_tables.set_symbol_width (half);
}

bool
Reading::get_symbol_width (void)
{
    return m_key2kana_tables.symbol_is_half ();
}

void
Reading::set_number_width (bool half)
{
    m_key2kana_tables.set_number_width (half);
}

bool
Reading::get_number_width (void)
{
    return m_key2kana_tables.number_is_half ();
}
