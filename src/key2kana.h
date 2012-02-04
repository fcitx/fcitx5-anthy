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

#ifndef __SCIM_ANTHY_KEY2KANA_H__
#define __SCIM_ANTHY_KEY2KANA_H__

#include "key2kanabase.h"
#include "defaulttables.h"
#include "key2kanatable.h"

struct FcitxAnthy;

class Key2KanaConvertor : public Key2KanaConvertorBase
{
public:
    Key2KanaConvertor                        (FcitxAnthy*        m_anthy,
                                              Key2KanaTableSet & tables);
    virtual ~Key2KanaConvertor               ();

    bool       can_append                    (FcitxKeySym sym, unsigned int state,
                                              bool               ignore_space = false);
    bool       append                        (FcitxKeySym sym, unsigned int state,
                                              std::string       & result,
                                              std::string       & pending,
                                              std::string           & raw);
    void       clear                         (void);

    bool       is_pending                    (void);
    std::string get_pending                   (void);
    std::string flush_pending                 (void);
    void       reset_pending                 (const std::string & result,
                                              const std::string     & raw);

private:
    bool       append             (const std::string     & str,
                                   std::string       & result,
                                   std::string       & pending);

private:
    FcitxAnthy*        m_anthy;
    
    Key2KanaTableSet  &m_tables;

    // state
    FcitxHotkey        m_last_key;
    std::string        m_pending;
    Key2KanaRule       m_exact_match;
};

#endif /* __SCIM_ANTHY_KEY2KANA_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
