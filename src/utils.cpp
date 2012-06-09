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

#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcitx-utils/utf8.h>

#include "utils.h"
#include "default_tables.h"

size_t
util_utf8_string_length(const std::string& s)
{
    return fcitx_utf8_strlen(s.c_str());
}

std::string
util_utf8_string_substr(const std::string& s, size_t start, size_t len)
{
    char* cs = strdup(s.c_str());
    char* startp = fcitx_utf8_get_nth_char(cs, start);
    char* endp = fcitx_utf8_get_nth_char(startp, len);
    std::string result(startp, endp - startp);
    free(cs);
    return result;
}

bool
util_match_key_event (const FcitxHotkey* hotkey, const KeyEvent &key,
                      uint16_t ignore_mask)
{
    return FcitxHotkeyIsHotKey(key.sym, key.state & ~ignore_mask, hotkey);
}

void
util_split_string (std::string &str, std::vector<std::string> &str_list,
                   char *delim, int num)
{
    std::string::size_type start = 0, end;

    for (int i = 0; (num > 0 && i < num) || start < str.length (); i++) {
        end = str.find (delim, start);
        if ((num > 0 && i == num - 1) || (end == std::string::npos))
            end = str.length ();

        if (start < str.length ()) {
            str_list.push_back (str.substr (start, end - start));
            start = end + strlen (delim);
        } else {
            str_list.push_back (std::string ());
        }
    }
}

void
util_convert_to_wide (std::string & wide, const std::string & str)
{
    for (unsigned int i = 0; i < str.length (); i++) {
        int c = str[i];
        char cc[2]; cc[0] = c; cc[1] = '\0';
        bool found = false;

        for (unsigned int j = 0; fcitx_anthy_wide_table[j].code; j++) {
            if ( fcitx_anthy_wide_table[j].code &&
                *fcitx_anthy_wide_table[j].code == c)
            {
                wide += fcitx_anthy_wide_table[j].wide;
                found = true;
                break;
            }
        }

        if (!found)
            wide += cc;
    }
}

void
util_convert_to_half (std::string & half, const std::string & str)
{
    for (unsigned int i = 0; i < str.length (); i++) {
        std::string wide = str.substr (i, 1);
        bool found = false;

        for (unsigned int j = 0; fcitx_anthy_wide_table[j].code; j++) {
            if (fcitx_anthy_wide_table[j].wide &&
                wide == fcitx_anthy_wide_table[j].wide)
            {
                half += fcitx_anthy_wide_table[j].code;
                found = true;
                break;
            }
        }

        if (!found)
            half += wide;
    }
}

void
util_convert_to_katakana (std::string & kata,
                          const std::string & hira,
                          bool half)
{
    for (unsigned int i = 0; i < util_utf8_string_length(hira); i++) {
        std::string tmpwide;
        bool found = false;

        HiraganaKatakanaRule *table = fcitx_anthy_hiragana_katakana_table;

        for (unsigned int j = 0; table[j].hiragana; j++) {
            tmpwide = table[j].hiragana;
            if (util_utf8_string_substr(hira, i, 1) == tmpwide) {
                if (half)
                    kata += table[j].half_katakana;
                else
                    kata += table[j].katakana;
                found = true;
                break;
            }
        }

        if (!found)
            kata += hira.substr(i, 1);
    }
}
#if 0
void
util_create_attributes (AttributeList &attrs,
                        unsigned int start,
                        unsigned int length,
                        std::string type,
                        unsigned int fg_color,
                        unsigned int bg_color)
{
    if (type == "None") {
        return;
    } else if (type == "Underline") {
        attrs.push_back (Attribute (start, length,
                                    SCIM_ATTR_DECORATE,
                                    SCIM_ATTR_DECORATE_UNDERLINE));
    } else if (type == "Reverse") {
        attrs.push_back (Attribute (start, length,
                                    SCIM_ATTR_DECORATE,
                                    SCIM_ATTR_DECORATE_REVERSE));
    } else if (type == "Highlight") {
        attrs.push_back (Attribute (start, length,
                                    SCIM_ATTR_DECORATE,
                                    SCIM_ATTR_DECORATE_HIGHLIGHT));
    } else {
        if (type == "Color" || type == "FGColor")
            attrs.push_back (Attribute (start, length,
                                        SCIM_ATTR_FOREGROUND,
                                        fg_color));
        if (type == "Color" || type == "BGColor")
            attrs.push_back (Attribute (start, length,
                                        SCIM_ATTR_BACKGROUND,
                                        bg_color));
    }
}
#endif

bool
util_key_is_keypad (const KeyEvent &key)
{
    switch (key.sym) {
    case FcitxKey_KP_Equal:
    case FcitxKey_KP_Multiply:
    case FcitxKey_KP_Add:
    case FcitxKey_KP_Separator:
    case FcitxKey_KP_Subtract:
    case FcitxKey_KP_Decimal:
    case FcitxKey_KP_Divide:
    case FcitxKey_KP_0:
    case FcitxKey_KP_1:
    case FcitxKey_KP_2:
    case FcitxKey_KP_3:
    case FcitxKey_KP_4:
    case FcitxKey_KP_5:
    case FcitxKey_KP_6:
    case FcitxKey_KP_7:
    case FcitxKey_KP_8:
    case FcitxKey_KP_9:
        return true;
    default:
        return false;
    }
}

void
util_keypad_to_string (std::string &str, const KeyEvent &key)
{
    char raw[2];

    switch (key.sym) {
    case FcitxKey_KP_Equal:
        raw[0] = '=';
        break;

    case FcitxKey_KP_Multiply:
        raw[0] = '*';
        break;

    case FcitxKey_KP_Add:
        raw[0] = '+';
        break;

    case FcitxKey_KP_Separator:
        raw[0] = ',';
        break;

    case FcitxKey_KP_Subtract:
        raw[0] = '-';
        break;

    case FcitxKey_KP_Decimal:
        raw[0] = '.';
        break;

    case FcitxKey_KP_Divide:
        raw[0] = '/';
        break;

    case FcitxKey_KP_0:
    case FcitxKey_KP_1:
    case FcitxKey_KP_2:
    case FcitxKey_KP_3:
    case FcitxKey_KP_4:
    case FcitxKey_KP_5:
    case FcitxKey_KP_6:
    case FcitxKey_KP_7:
    case FcitxKey_KP_8:
    case FcitxKey_KP_9:
        raw[0] = '0' + key.sym - FcitxKey_KP_0;
        break;

    default:
        if (isprint (key.get_ascii_code()))
            raw[0] = key.get_ascii_code();
        else
            raw[0] = '\0';
        break;
    }

    raw[1] = '\0';
    str = raw;
}

void
util_launch_program (const char *command)
{
    if (!command) return;

    /* split string */
    unsigned int len = strlen (command);
    char tmp[len + 1];
    strncpy (tmp, command, len);
    tmp[len] = '\0';

    char *str = tmp;
    std::vector<char *> array;

    for (unsigned int i = 0; i < len + 1; i++) {
        if (!tmp[i] || isspace (tmp[i])) {
            if (*str) {
                tmp[i] = '\0';
                array.push_back (str);
            }
            str = tmp + i + 1;
        }
    }

    if (array.size () <= 0) return;
    array.push_back (NULL);

    char *args[array.size()];
    for (unsigned int i = 0; i < array.size (); i++)
        args[i] = array[i];


    /* exec command */
	pid_t child_pid;

	child_pid = fork();
	if (child_pid < 0) {
		perror("fork");
	} else if (child_pid == 0) {		 /* child process  */
		pid_t grandchild_pid;

		grandchild_pid = fork();
		if (grandchild_pid < 0) {
			perror("fork");
			_exit(1);
		} else if (grandchild_pid == 0) { /* grandchild process  */
			execvp(args[0], args);
			perror("execvp");
			_exit(1);
		} else {
			_exit(0);
		}
	} else {                              /* parent process */
		int status;
		waitpid(child_pid, &status, 0);
	}
}
