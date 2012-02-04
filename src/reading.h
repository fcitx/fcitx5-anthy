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

#ifndef __SCIM_ANTHY_READING_H__
#define __SCIM_ANTHY_READING_H__

#include "key2kanatable.h"
#include "key2kana.h"
#include "kana.h"

typedef enum {
    SCIM_ANTHY_STRING_LATIN,
    SCIM_ANTHY_STRING_WIDE_LATIN,
    SCIM_ANTHY_STRING_HIRAGANA,
    SCIM_ANTHY_STRING_KATAKANA,
    SCIM_ANTHY_STRING_HALF_KATAKANA,
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

    const std::string& get     (void) { return kana; }
    const std::string& get_raw (void) { return raw; }

    void split (ReadingSegments &segments);

private:
    std::string     raw;
    std::string     kana;
};

class Reading
{
public:
    Reading (FcitxAnthy* anthy);
    virtual ~Reading ();

    bool         can_process_key_event (FcitxKeySym sym, unsigned int state);
    bool         process_key_event     (FcitxKeySym sym, unsigned int state);
    void         finish                (void);
    void         clear                 (void);

    std::string   get                   (unsigned int     start  = 0,
                                        int              length = -1,
                                        StringType       type
                                        = SCIM_ANTHY_STRING_HIRAGANA);
    std::string        get_raw               (unsigned int     start  = 0,
                                        int              length = -1);
    bool         append                (FcitxKeySym sym, unsigned int state,
                                        const std::string& string);
    void         erase                 (unsigned int     start  = 0,
                                        int              length = -1,
                                        bool             allow_split = false);

    unsigned int get_length            (void);
    unsigned int get_caret_pos         (void);
    void         set_caret_pos         (unsigned int     pos);
    void         move_caret            (int              step,
                                        bool             allow_split = false);

    void         set_typing_method     (TypingMethod     method);
    TypingMethod get_typing_method     (void);
    void         set_symbol_width      (bool             half);
    bool         get_symbol_width      (void);
    void         set_number_width      (bool             half);
    bool         get_number_width      (void);

private:
    void         reset_pending         (void);
    void         split_segment         (unsigned int     seg_id);

private:
    FcitxAnthy* m_anthy;

    // tables
    Key2KanaTableSet       m_key2kana_tables;
    Key2KanaTableSet       m_nicola_tables;

    // convertors
    Key2KanaConvertor      m_key2kana_normal;
    KanaConvertor          m_kana;
    Key2KanaConvertorBase *m_key2kana;

    // state
    ReadingSegments        m_segments;
    unsigned int           m_segment_pos;
    unsigned int           m_caret_offset;
};

#endif /* __SCIM_ANTHY_READING_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
