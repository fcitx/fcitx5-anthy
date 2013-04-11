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

#ifndef __FCITX_ANTHY_ACTION_H__
#define __FCITX_ANTHY_ACTION_H__

#include <string>
#include "common.h"

class AnthyInstance;

typedef bool (AnthyInstance::*PMF) (void);

class Action
{

public:

    Action();
    Action  (const std::string &name, FcitxHotkey* hotkey, PMF pmf);
    ~Action ();

public:
    bool perform (AnthyInstance  *performer);
    bool perform (AnthyInstance  *performer,
                  const KeyEvent &key);

    bool match_action_name (const char *name);

    bool operator < (const Action& b) {
        return m_name < b.m_name;
    }

    bool match_key_event (const KeyEvent &key);

    std::string         m_name;
    std::string         m_desc;
    PMF            m_pmf;
    FcitxHotkey*   m_key_bindings;
};

#endif /* __FCITX_ANTHY_ACTION_H__ */
