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

#ifndef __SCIM_ANTHY_CONVERSION_H__
#define __SCIM_ANTHY_CONVERSION_H__

#include <anthy/anthy.h>
#include <fcitx/candidate.h>

#include "reading.h"

struct FcitxAnthy;

typedef enum {
    SCIM_ANTHY_CANDIDATE_DEFAULT       = 0,
    SCIM_ANTHY_CANDIDATE_LATIN         = -1,
    SCIM_ANTHY_CANDIDATE_WIDE_LATIN    = -2,
    SCIM_ANTHY_CANDIDATE_HIRAGANA      = -3,
    SCIM_ANTHY_CANDIDATE_KATAKANA      = -4,
    SCIM_ANTHY_CANDIDATE_HALF_KATAKANA = -5,
    SCIM_ANTHY_CANDIDATE_HALF          = -6,
    SCIM_ANTHY_LAST_SPECIAL_CANDIDATE  = -7,
} CandidateType;

struct FcitxAnthyCandWord
{
    int item;
};

class ConversionSegment
{
public:
    ConversionSegment (std::string   str,
                       int          cand_id,
                       unsigned int reading_len);
    virtual ~ConversionSegment ();

    std::string & get_string         (void);
    int          get_candidate_id   (void);
    unsigned int get_reading_length (void);

    void         set                (std::string   str,
                                     int          cand_id);
    void         set_reading_length (unsigned int len);

private:
    std::string   m_string;
    int          m_cand_id;
    unsigned int m_reading_len;
};
typedef std::vector<ConversionSegment> ConversionSegments;

class Conversion
{
public:
    Conversion (FcitxAnthy *anthy, Reading &reading);
    virtual ~Conversion ();

    // starting and finishing
    void          convert                (std::string    source,
                                          CandidateType ctype,
                                          bool          single_segment);
    void          convert                (CandidateType type
                                          = SCIM_ANTHY_CANDIDATE_DEFAULT,
                                          bool          single_segment = false);
    void          convert                (const std::string &source,
                                          bool          single_segment = false);
    void          predict                (void);
    void          clear                  (int           segment_id = -1);
    void          commit                 (int           segment_id = -1,
                                          bool          learn      = true);

    // getting status
    bool          is_converting          (void);
    bool          is_predicting          (void);

    std::string    get                    (void);
    unsigned int  get_length             (void);

    // segments of the converted sentence
    int           get_nr_segments        (void);
    std::string    get_segment_string     (int           segment_id = -1,
                                          int           candidate_id
                                          = SCIM_ANTHY_LAST_SPECIAL_CANDIDATE);
    int           get_selected_segment   (void);
    void          select_segment         (int           segment_id);
    int           get_segment_size       (int           segment_id = -1);
    void          resize_segment         (int           relative_size,
                                          int           segment_id = -1);
    unsigned int  get_segment_position   (int           segment_id = -1);

    // candidates for a segment or prediction
    void          get_candidates         (struct _FcitxCandidateWordList *table,
                                          int           segment_id = -1);
    int           get_selected_candidate (int           segment_id = -1);
    void          select_candidate       (int           candidate_id,
                                          int           segment_id = -1);

private:
    void          get_reading_substr     (std::string   &string,
                                          int           segment_id,
                                          int           candidate_id,
                                          int           seg_start,
                                          int           seg_len);
    std::string    get_prediction_string  (int           candidate_id);
    void          join_all_segments      (void);

private:
    FcitxAnthy      *m_anthy;

    // convertors
    Reading            &m_reading;
    anthy_context_t     m_anthy_context;

    // status variables
    ConversionSegments  m_segments;
    int                 m_start_id;    // number of commited segments
    int                 m_cur_segment; // relative position from m_start_id
    bool                m_predicting;
};

#endif /* __SCIM_ANTHY_READING_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
