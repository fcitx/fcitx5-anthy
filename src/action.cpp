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

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include "action.h"
#include "utils.h"

Action::Action () :
    m_name(""),
    m_pmf(NULL),
    m_key_bindings(NULL)
{
}


Action::Action (const std::string &name, FcitxHotkey* hotkey, PMF pmf)
    : m_name (name),
      m_pmf  (pmf),
      m_key_bindings(hotkey)
{
}

Action::~Action (void)
{
}

bool
Action::perform (AnthyInstance *performer)
{
    if (m_pmf)
        return (performer->*m_pmf) ();

    return false;
}

bool
Action::perform (AnthyInstance *performer, const KeyEvent &key)
{
    if (!m_pmf)
        return false;

    if (match_key_event (key)) {
        if (m_pmf)
            return (performer->*m_pmf) ();
    }

    return false;
}

bool
Action::match_key_event (const KeyEvent &key)
{
    return util_match_key_event (m_key_bindings, key, FcitxKeyState_CapsLock);
}

bool
Action::match_action_name (const char *name)
{
    return (m_name.compare (name) == 0);
}
