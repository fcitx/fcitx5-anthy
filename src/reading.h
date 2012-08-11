/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie
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

#ifndef __FCITX_ANTHY_READING_H__
#define __FCITX_ANTHY_READING_H__

#include "key2kana.h"
#include "kana.h"
#include "nicola.h"
#include "factory.h"

class AnthyInstance;

typedef enum {
    FCITX_ANTHY_STRING_LATIN,
    FCITX_ANTHY_STRING_WIDE_LATIN,
    FCITX_ANTHY_STRING_HIRAGANA,
    FCITX_ANTHY_STRING_KATAKANA,
    FCITX_ANTHY_STRING_HALF_KATAKANA,
} StringType;

class Reading;
class ReadingSegment;
typedef std::vector<ReadingSegment> ReadingSegments;

class ReadingSegment
{
    friend class Reading;

public:
    ReadingSegment (void);
    virtual ~ReadingSegment ();

    const std::string & get     (void) { return kana; }
    const std::string     & get_raw (void) { return raw; }

    void split (ReadingSegments &segments);

private:
    std::string     raw;
    std::string kana;
};

class Reading
{
public:
    Reading (AnthyInstance &anthy);
    virtual ~Reading ();

    bool         can_process_key_event (const KeyEvent & key);
    bool         process_key_event     (const KeyEvent & key);
    void         finish                (void);
    void         clear                 (void);

    std::string   get_by_char          (unsigned int     start  = 0,
                                        int              length = -1,
                                        StringType       type
                                        = FCITX_ANTHY_STRING_HIRAGANA);
    std::string       get_raw_by_char               (unsigned int     start  = 0,
                                        int              length = -1);
    bool         append                (const KeyEvent & key,
                                        const std::string   & string);
    void         erase                 (unsigned int     start  = 0,
                                        int              length = -1,
                                        bool             allow_split = false);

    unsigned int get_length            (void);
    unsigned int get_length_by_char    (void);
    unsigned int get_caret_pos         (void);
    unsigned int get_caret_pos_by_char (void);
    void         set_caret_pos_by_char (unsigned int     pos);
    void         move_caret            (int              step,
                                        bool             allow_split = false);

    void         set_typing_method     (TypingMethod     method);
    TypingMethod get_typing_method     (void);
    void         set_period_style      (PeriodStyle      style);
    PeriodStyle  get_period_style      (void);
    void         set_comma_style       (CommaStyle       style);
    CommaStyle   get_comma_style       (void);
    void         set_bracket_style     (BracketStyle     style);
    BracketStyle get_bracket_style     (void);
    void         set_slash_style       (SlashStyle       style);
    SlashStyle   get_slash_style       (void);
    void         set_symbol_width      (bool             half);
    bool         get_symbol_width      (void);
    void         set_number_width      (bool             half);
    bool         get_number_width      (void);
    void         set_pseudo_ascii_mode (int              mode);
    bool         is_pseudo_ascii_mode  (void);
    void         reset_pseudo_ascii_mode (void);

private:
    void         reset_pending         (void);
    void         split_segment         (unsigned int     seg_id);

private:
    AnthyInstance         &m_anthy;

    // tables
    Key2KanaTableSet       m_key2kana_tables;
    Key2KanaTableSet       m_nicola_tables;

    // convertors
    Key2KanaConvertor      m_key2kana_normal;
    KanaConvertor          m_kana;
    NicolaConvertor        m_nicola;
    Key2KanaConvertorBase *m_key2kana;

    // state
    ReadingSegments        m_segments;
    unsigned int           m_segment_pos;
    unsigned int           m_caret_offset;
};

#endif /* __FCITX_ANTHY_READING_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
