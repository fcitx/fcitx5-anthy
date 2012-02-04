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

#ifndef __SCIM_ANTHY_KANA_H__
#define __SCIM_ANTHY_KANA_H__

#include "key2kanabase.h"
#include "defaulttables.h"
#include "key2kanatable.h"


struct FcitxAnthy;

class KanaConvertor : public Key2KanaConvertorBase
{
public:
               KanaConvertor      (FcitxAnthy*    anthy);
    virtual   ~KanaConvertor      ();

    bool       can_append         (FcitxKeySym sym, unsigned int state,
                                   bool               ignore_space = false);
    bool       append             (FcitxKeySym sym, unsigned int state,
                                   std::string       & result,
                                   std::string       & pending,
                                   std::string           & raw);
    bool       append             (const std::string     & raw,
                                   std::string       & result,
                                   std::string       & pending);
    void       clear              (void);

    bool       is_pending         (void);
    std::string get_pending        (void);
    std::string flush_pending      (void);
    void       reset_pending      (const std::string & result,
                                   const std::string     & raw);

private:
    FcitxAnthy* m_anthy;

    // state
    std::string         m_pending;
};

#endif /* __SCIM_ANTHY_KANA_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
