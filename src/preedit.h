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

#ifndef __FCITX_ANTHY_PREEDIT_H__
#define __FCITX_ANTHY_PREEDIT_H__

#include <anthy/anthy.h>
#include "reading.h"
#include "conversion.h"

#define FCITX_ANTHY_PSEUDO_ASCII_TRIGGERED_CAPITALIZED			(1 << 0)
#define FCITX_ANTHY_PSEUDO_ASCII_TRIGGERED_COUPLE_OF_CAPITAL		(1 << 1)

class AnthyInstance;

class Preedit
{
public:
    Preedit (AnthyInstance &anthy);
    virtual ~Preedit ();

    // getting status
    unsigned int  get_length             (void);
    unsigned int  get_length_by_char     (void);
    std::string   get_string             (void);
    void          update_preedit         (void);
    Reading      &get_reading            (void);

    bool          is_preediting          (void);
    bool          is_converting          (void);
    bool          is_predicting          (void);
    bool          is_reconverting        (void);

    // for handling the preedit string
    bool          can_process_key_event  (const KeyEvent & key);
    // return true if commiting is needed.
    bool          process_key_event      (const KeyEvent & key);
    bool          append                 (const KeyEvent & key,
                                                  const std::string   & string);
    void          erase                  (bool backward = true);
    void          finish                 (void);

    // for handling the conversion string
    void          convert                (CandidateType type
                                                  = FCITX_ANTHY_CANDIDATE_DEFAULT,
                                                  bool single_segment = false);
    void          convert                (const std::string &source,
                                                  bool single_segment = false);
    void          revert                 (void);
    void          commit                 (int  segment_id = -1,
                                                  bool lean       = true);

    // for prediction
    void          predict                (void);

    // segments of the converted sentence
    int           get_nr_segments        (void);
    std::string    get_segment_string     (int segment_id = -1);
    int           get_selected_segment   (void);
    void          select_segment         (int segment_id);
    int           get_segment_size       (int segment_id = -1);
    void          resize_segment         (int relative_size,
                                                  int segment_id = -1);

    // candidates for a segment
    void          get_candidates         (FcitxCandidateWordList *table,
                                                  int segment_id = -1);
    int           get_selected_candidate (int segment_id = -1);
    void          select_candidate       (int candidate_id,
                                                  int segment_id = -1);

    // for handling the caret
    unsigned int  get_caret_pos          (void);
    void          set_caret_pos_by_char  (unsigned int   pos);
    void          move_caret             (int            len);

    // clear all or part of the string.
    void          clear                  (int segment_id = -1);

    // preferences
    void          set_input_mode         (InputMode      mode);
    InputMode     get_input_mode         (void);
    void          set_typing_method      (TypingMethod   method);
    TypingMethod  get_typing_method      (void);
    void          set_period_style       (PeriodStyle    style);
    PeriodStyle   get_period_style       (void);
    void          set_comma_style        (CommaStyle     style);
    CommaStyle    get_comma_style        (void);
    void          set_bracket_style      (BracketStyle   style);
    BracketStyle  get_bracket_style      (void);
    void          set_slash_style        (SlashStyle     style);
    SlashStyle    get_slash_style        (void);
    void          set_symbol_width       (bool           half);
    bool          get_symbol_width       (void);
    void          set_number_width       (bool           half);
    bool          get_number_width       (void);
    void          set_pseudo_ascii_mode  (int            mode);
    bool          is_pseudo_ascii_mode   (void);
    void          reset_pseudo_ascii_mode(void);

private:
    void                  get_reading_substr     (std::string   & substr,
                                                  unsigned int   start,
                                                  unsigned int   len,
                                                  CandidateType  type);
    bool                  is_comma_or_period     (const std::string & str);

private:
    AnthyInstance    &m_anthy;

    // converter objects
    Reading           m_reading;
    Conversion        m_conversion;

    // mode flags
    InputMode         m_input_mode;

    // source string for reconversion
    std::string        m_source;
};

#endif /* __FCITX_ANTHY_PREEDIT_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
