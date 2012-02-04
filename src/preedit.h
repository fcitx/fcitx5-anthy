/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
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

#ifndef __SCIM_ANTHY_PREEDIT_H__
#define __SCIM_ANTHY_PREEDIT_H__

#include <anthy/anthy.h>
#include <fcitx-config/hotkey.h>
#include "reading.h"
#include "conversion.h"

#define SCIM_ANTHY_PSEUDO_ASCII_TRIGGERED_CAPITALIZED			(1 << 0)
#define SCIM_ANTHY_PSEUDO_ASCII_TRIGGERED_COUPLE_OF_CAPITAL		(1 << 1)

typedef enum {
    SCIM_ANTHY_MODE_HIRAGANA,
    SCIM_ANTHY_MODE_KATAKANA,
    SCIM_ANTHY_MODE_HALF_KATAKANA
} InputMode;

class Preedit
{
public:
    Preedit (FcitxAnthy* anthy);
    virtual ~Preedit ();

    // getting status
    virtual unsigned int  get_length             (void);
    virtual std::string   get_string             (void);
    virtual Reading      &get_reading            (void);

    virtual bool          is_preediting          (void);
    virtual bool          is_converting          (void);
    virtual bool          is_predicting          (void);
    virtual bool          is_reconverting        (void);

    // for handling the preedit string
    virtual bool          can_process_key_event  (FcitxKeySym sym, unsigned int state);
    // return true if commiting is needed.
    virtual bool          process_key_event      (FcitxKeySym sym, unsigned int state);
    virtual bool          append                 (FcitxKeySym sym, unsigned int state,
                                                  const std::string   & string);
    virtual void          erase                  (bool backward = true);
    virtual void          finish                 (void);

    // for handling the conversion string
    virtual void          convert                (CandidateType type
                                                  = SCIM_ANTHY_CANDIDATE_DEFAULT,
                                                  bool single_segment = false);
    virtual void          convert                (const std::string &source,
                                                  bool single_segment = false);
    virtual void          revert                 (void);
    virtual void          commit                 (int  segment_id = -1,
                                                  bool lean       = true);

    // for prediction
    virtual void          predict                (void);

    // segments of the converted sentence
    virtual int           get_nr_segments        (void);
    virtual std::string    get_segment_string     (int segment_id = -1);
    virtual int           get_selected_segment   (void);
    virtual void          select_segment         (int segment_id);
    virtual int           get_segment_size       (int segment_id = -1);
    virtual void          resize_segment         (int relative_size,
                                                  int segment_id = -1);

    // candidates for a segment
    virtual void          get_candidates         (struct _FcitxCandidateWordList *table,
                                                  int segment_id = -1);
    virtual int           get_selected_candidate (int segment_id = -1);
    virtual void          select_candidate       (int candidate_id,
                                                  int segment_id = -1);

    // for handling the caret
    virtual unsigned int  get_caret_pos          (void);
    virtual void          set_caret_pos          (unsigned int   pos);
    virtual void          move_caret             (int            len);

    // clear all or part of the string.
    virtual void          clear                  (int segment_id = -1);

    // preferences
    virtual void          set_input_mode         (InputMode      mode);
    virtual InputMode     get_input_mode         (void);
    virtual void          set_typing_method      (TypingMethod   method);
    virtual TypingMethod  get_typing_method      (void);
    virtual void          set_symbol_width       (bool           half);
    virtual bool          get_symbol_width       (void);
    virtual void          set_number_width       (bool           half);
    virtual bool          get_number_width       (void);

private:
    void                  get_reading_substr     (std::string   & substr,
                                                  unsigned int   start,
                                                  unsigned int   len,
                                                  CandidateType  type);
    bool                  is_comma_or_period     (const std::string & str);

private:
    FcitxAnthy* m_anthy;

    // converter objects
    Reading           m_reading;
    Conversion        m_conversion;

    // mode flags
    InputMode         m_input_mode;

    // source string for reconversion
    std::string        m_source;
};

#endif /* __SCIM_ANTHY_PREEDIT_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
