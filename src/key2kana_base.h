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

#ifndef __FCITX_ANTHY_KEY2KANA_BASE_H__
#define __FCITX_ANTHY_KEY2KANA_BASE_H__

#include "common.h"

class Key2KanaConvertorBase
{
public:
    Key2KanaConvertorBase                 ()
        : m_case_sensitive (true)
        {};
    virtual ~Key2KanaConvertorBase        () {};

    virtual bool       can_append         (const KeyEvent   & key,
                                           bool               ignore_space = false) = 0;
    virtual bool       append             (const KeyEvent   & key,
                                           std::string       & result,
                                           std::string       & pending,
                                           std::string           & raw)       = 0;
    virtual bool       append             (const std::string     & raw,
                                           std::string       & result,
                                           std::string       & pending)   = 0;
    virtual void       clear              (void)                         = 0;

    virtual bool       is_pending         (void)                         = 0;
    virtual std::string get_pending        (void)                         = 0;
    virtual std::string flush_pending      (void)                         = 0;
    virtual void       reset_pending      (const std::string & result,
                                           const std::string     & raw)       = 0;

    virtual void       reset_pseudo_ascii_mode       (void)
        {}
    virtual bool       process_pseudo_ascii_mode     (const std::string & wstr)
        { return false; }

    virtual void       set_case_sensitive (bool               sensitive)
        { m_case_sensitive = sensitive; }
    virtual bool       get_case_sensitive (void)
        { return m_case_sensitive; }

protected:
    bool m_case_sensitive;
};

#endif /* __FCITX_ANTHY_KEY2KANA_BASE_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
