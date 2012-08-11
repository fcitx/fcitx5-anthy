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

#ifndef __FCITX_ANTHY_KEY2KANA_H__
#define __FCITX_ANTHY_KEY2KANA_H__

#include "key2kana_base.h"
#include "default_tables.h"
#include "key2kana_table.h"

class AnthyInstance;

class Key2KanaConvertor : public Key2KanaConvertorBase
{
public:
    Key2KanaConvertor                        (AnthyInstance    & anthy,
                                              Key2KanaTableSet & tables);
    virtual ~Key2KanaConvertor               ();

    bool       can_append                    (const KeyEvent   & key,
                                              bool               ignore_space = false);
    bool       append                        (const KeyEvent   & key,
                                              std::string       & result,
                                              std::string       & pending,
                                              std::string           & raw);
    void       clear                         (void);

    bool       is_pending                    (void);
    std::string get_pending                   (void);
    std::string flush_pending                 (void);
    void       reset_pending                 (const std::string & result,
                                              const std::string     & raw);
    void       set_pseudo_ascii_mode         (int                mode)
        { m_pseudo_ascii_mode = mode; }
    bool       is_pseudo_ascii_mode          (void)
        { return m_is_in_pseudo_ascii_mode; }
    bool       process_pseudo_ascii_mode     (const std::string & wstr);
    void       reset_pseudo_ascii_mode       (void);

private:
    bool       append             (const std::string     & str,
                                   std::string       & result,
                                   std::string       & pending);

private:
    AnthyInstance     &m_anthy;
    Key2KanaTableSet  &m_tables;

    // state
    KeyEvent           m_last_key;
    std::string         m_pending;
    Key2KanaRule       m_exact_match;
    int                m_pseudo_ascii_mode;
    bool               m_is_in_pseudo_ascii_mode;
    bool               m_reset_pseudo_ascii_mode;
};

#endif /* __FCITX_ANTHY_KEY2KANA_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
