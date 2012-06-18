/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
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

#ifndef __FCITX_ANTHY_NICOLA_H__
#define __FCITX_ANTHY_NICOLA_H__

#include <sys/time.h>
#include <stdint.h>

#include "key2kana_base.h"
#include "key2kana_table.h"

class AnthyInstance;

typedef enum {
    FCITX_ANTHY_NICOLA_SHIFT_NONE,
    FCITX_ANTHY_NICOLA_SHIFT_LEFT,
    FCITX_ANTHY_NICOLA_SHIFT_RIGHT,
} NicolaShiftType;

class NicolaConvertor : public Key2KanaConvertorBase
{
public:
    NicolaConvertor                     (AnthyInstance    & anthy,
                                         Key2KanaTableSet & tables);
    virtual ~NicolaConvertor            ();

    bool       can_append               (const KeyEvent&   key,
                                         bool              ignore_space = false);
    bool       append                   (const KeyEvent&   key,
                                         std::string       & result,
                                         std::string       & pending,
                                         std::string           & raw);
    bool       append                   (const std::string     & raw,
                                         std::string       & result,
                                         std::string       & pending);
    void       clear                    (void);

    bool       is_pending               (void);
    std::string get_pending              (void);
    std::string flush_pending            (void);
    void       reset_pending            (const std::string & result,
                                         const std::string     & raw);

public:
    void       process_timeout          (void);

private:
    void       search                   (const KeyEvent&     key,
                                         NicolaShiftType    shift_type,
                                         std::string       & result,
                                         std::string           & raw);
    bool       handle_voiced_consonant  (std::string       & result,
                                         std::string       & pending);
    bool       is_char_key              (const KeyEvent&     key);
    bool       is_thumb_key             (const KeyEvent&     key);
    bool       is_left_thumb_key        (const KeyEvent&     key);
    bool       is_right_thumb_key       (const KeyEvent&     key);
    NicolaShiftType
               get_thumb_key_type       (const KeyEvent&     key);
    bool       emit_key_event           (const KeyEvent&    key);
    void       set_alarm                (int                time_msec);
    bool stop();
    int         thumb_key ( const KeyEvent& key);

private:
    Key2KanaTableSet &m_tables;

    AnthyInstance    &m_anthy;

    // state
    KeyEvent          m_prev_char_key;

    KeyEvent          m_repeat_char_key;

    uint32_t          m_timer_id;
    bool              m_processing_timeout;

    std::string        m_pending;
    KeyEvent m_through_key_event;
    KeyEvent m_repeat_thumb_key;
    KeyEvent m_prev_thumb_key;
};

#endif /* __FCITX_ANTHY_NICOLA_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
