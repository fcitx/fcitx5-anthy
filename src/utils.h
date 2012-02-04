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

#ifndef __SCIM_ANTHY_UTILS_H__
#define __SCIM_ANTHY_UTILS_H__
#include <fcitx-config/hotkey.h>
#include <string>

void util_split_string        (std::string              &str,
                               std::vector<std::string> &str_list,
                               char                *delim,
                               int                  num);
void util_convert_to_wide (std::string & wide, const std::string & str);
void util_convert_to_half     (std::string              &half,
                               const std::string    &str);
void util_convert_to_katakana (std::string          &kata,
                               const std::string    &hira,
                               bool                 half = false);
bool util_key_is_keypad       (FcitxKeySym sym, unsigned int state);
void util_keypad_to_string    (std::string              &str,
                               FcitxKeySym sym, unsigned int state);
void util_launch_program      (const char          *command);

#endif /* __SCIM_ANTHY_UTILS_H__ */
