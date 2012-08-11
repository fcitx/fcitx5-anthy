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

#include <fstream>
#include <fcitx-utils/log.h>

#include "style_file.h"
#include "key2kana_table.h"

const int MAX_LINE_LENGTH = 4096;

static std::string
escape (const std::string &str)
{
    std::string dest = str;

    for (unsigned int i = 0; i < dest.size (); i++) {
        if (dest[i] == '#'  ||                   // for comment
            dest[i] == '\\' ||                   // for backslash itself
            dest[i] == '='  ||                   // for separatort
            dest[i] == '['  || dest[i] == ']' || // for section
            dest[i] == ','  ||                   // for array
            dest[i] == ' '  || dest[i] == '\t')  // for space
        {
            dest.insert (i, "\\");
            i++;
        }
    }

    return dest;
}

static std::string
unescape (const std::string &str)
{
    std::string dest = str;

    for (unsigned int i = 0; i < dest.size (); i++) {
        if (dest[i] == '\\') {
            dest.erase (i, 1);
            if (i < dest.size () && dest[i] == '\\')
                i++;
        }
    }

    return dest;
}

StyleLine::StyleLine (StyleFile *style_file, std::string line)
    : m_style_file (style_file),
      m_line  (line),
      m_type  (FCITX_ANTHY_STYLE_LINE_UNKNOWN)
{
}

StyleLine::StyleLine (StyleFile *style_file, std::string key, std::string value)
    : m_style_file (style_file),
      m_line  (escape (key) + std::string ("=")),
      m_type  (FCITX_ANTHY_STYLE_LINE_KEY)
{
    set_value (value);
}

StyleLine::StyleLine (StyleFile *style_file, std::string key,
                      std::vector<std::string> &value)
    : m_style_file (style_file),
      m_line  (escape (key) + std::string("=")),
      m_type  (FCITX_ANTHY_STYLE_LINE_KEY)
{
    set_value_array (value);
}

StyleLine::~StyleLine ()
{
}

StyleLineType
StyleLine::get_type (void)
{
    if (m_type != FCITX_ANTHY_STYLE_LINE_UNKNOWN)
        return m_type;

    int spos, epos;
    for (spos = 0;
         spos < m_line.length () && isspace (m_line[spos]);
         spos++);
    if (m_line.length() > 0) {
        for (epos = m_line.length () - 1;
             epos >= 0 && isspace (m_line[epos]);
             epos--);
    } else {
        epos = 0;
    }

    if (m_line.length() == 0 || spos >= m_line.length()) {
        m_type = FCITX_ANTHY_STYLE_LINE_SPACE;
        return m_type;

    } else if (m_line[spos] == '#') {
        m_type = FCITX_ANTHY_STYLE_LINE_COMMENT;
        return m_type;

    } else if (m_line[spos] == '[' && m_line[epos] == ']') {
        m_type = FCITX_ANTHY_STYLE_LINE_SECTION;
        return m_type;
    }

    m_type = FCITX_ANTHY_STYLE_LINE_KEY;
    return m_type;
}

bool
StyleLine::get_section (std::string &section)
{
    if (get_type () != FCITX_ANTHY_STYLE_LINE_SECTION)
        return false;

    int spos, epos;
    for (spos = 0;
         spos < m_line.length () && isspace (m_line[spos]);
         spos++);
    for (epos = m_line.length () - 1;
         epos >= 0 && isspace (m_line[epos]);
         epos--);
    spos++;

    if (spos < epos)
        section = m_line.substr (spos, epos - spos);
    else
        section = std::string ();

    return true;
}

bool
StyleLine::get_key (std::string &key)
{
    if (get_type () != FCITX_ANTHY_STYLE_LINE_KEY)
        return false;

    int spos, epos;
    for (spos = 0;
         spos < m_line.length () && isspace (m_line[spos]);
         spos++);

    for (epos = spos;
         epos < m_line.length ();
         epos++)
    {
        if (m_line[epos] == '\\') {
            epos++;
            continue;
        }
        if (m_line[epos] == '=') {
            break;
        }
    }
    for (--epos;
         epos >= spos && isspace (m_line[epos]);
         epos--);
    if (!isspace(m_line[epos]))
        epos++;

    if (spos >= 0 && spos < epos && epos <= m_line.length ()) {
        key = unescape (m_line.substr (spos, epos - spos));
    } else
        key = std::string ();

    return true;
}

static int
get_value_position (std::string &str)
{
    unsigned int spos;
    for (spos = 0;
         spos < str.length ();
         spos++)
    {
        if (str[spos] == '\\') {
            spos++;
            continue;
        }
        if (str[spos] == '=') {
            break;
        }
    }
    if (spos >= str.length ())
        return true;
    else
        spos++;
    for (;
         spos < str.length () && isspace(str[spos]);
         spos++);

    return spos;
}

bool
StyleLine::get_value (std::string &value)
{
    if (get_type () != FCITX_ANTHY_STYLE_LINE_KEY)
        return false;

    unsigned int spos = get_value_position (m_line);
    unsigned int epos = m_line.length ();

    value = unescape (m_line.substr (spos, epos - spos));

    return true;
}

void
StyleLine::set_value (std::string value)
{
    std::string key;
    get_key (key);
    m_line = escape (key) + std::string ("=") + escape (value);
}

bool
StyleLine::get_value_array (std::vector<std::string> &value)
{
    if (get_type () != FCITX_ANTHY_STYLE_LINE_KEY)
        return false;

    unsigned int spos = get_value_position (m_line);
    unsigned int epos = m_line.length ();

    unsigned int head_of_element = spos;
    for (unsigned int i = spos; i <= epos; i++) {
        if (i < epos && m_line[i] == '\\') {
            i++;
            continue;
        }

        if (i == epos || m_line[i] == ',') {
            std::string str;
            if (head_of_element == epos)
                str = std::string ();
            else
                str = unescape (m_line.substr (head_of_element,
                                               i - head_of_element));
            value.push_back (str);
            head_of_element = i + 1;
        }
    }

    return true;
}

void
StyleLine::set_value_array (std::vector<std::string> &value)
{
    std::string key;
    get_key (key);

    m_line = escape (key) + std::string ("=");
    for (unsigned int i = 0; i < value.size (); i++) {
        if (i != 0)
            m_line += ",";
        m_line += escape (value[i]);
    }
}


StyleFile::StyleFile ()
{
    setup_default_entries ();
}

StyleFile::~StyleFile ()
{
}

bool
StyleFile::load (const char *filename)
{
    clear ();
    setup_default_entries ();
    m_filename = filename;

    std::ifstream in_file (filename);
    if (!in_file)
        return false;

    clear ();

    m_sections.push_back (StyleLines ());
    StyleLines *section = &m_sections[0];
    unsigned int section_id = 0;

    char buf[MAX_LINE_LENGTH];
    do {
        in_file.getline (buf, MAX_LINE_LENGTH);
        if (in_file.eof ())
            break;

        std::string dest = buf;
        StyleLine line (this, dest);
        StyleLineType type = line.get_type ();

        if (type == FCITX_ANTHY_STYLE_LINE_SECTION) {
            m_sections.push_back (StyleLines ());
            section = &m_sections.back();
            section_id++;
        }

        section->push_back (line);

        if (section_id == 0) {
            std::string key;
            line.get_key (key);
            if (key == "FormatVersion") {
                line.get_value (m_format_version);

            } else if (key == "Title") {
                line.get_value (m_title);

            } else if (key == "Version") {
                line.get_value (m_version);
            }
        }
    } while (!in_file.eof ());

    in_file.close ();

    m_filename = filename;

    return true;
}

bool
StyleFile::save (const char *filename)
{
    std::ofstream out_file (filename);
    if (!out_file)
        return false;

    StyleSections::iterator it;
    for (it = m_sections.begin (); it != m_sections.end (); it++) {
        StyleLines::iterator lit;
        for (lit = it->begin (); lit != it->end (); lit++) {
            std::string line, dest;
            lit->get_line (line);
            dest = line;
            out_file << dest.c_str () << std::endl;
        }
    }

    out_file.close ();

    m_filename = filename;

    return true;
}

void
StyleFile::clear (void)
{
    m_filename       = std::string ();
    m_format_version = std::string ();
    m_title          = std::string ();
    m_version        = std::string ();
    m_sections.clear ();
}

std::string
StyleFile::get_title (void)
{
    return m_title;
}

std::string
StyleFile::get_file_name (void)
{
    return m_filename;
}

bool
StyleFile::get_string (std::string &value, std::string section, std::string key)
{
    StyleSections::iterator it;
    for (it = m_sections.begin (); it != m_sections.end (); it++) {
        if (it->size () <= 0)
            continue;

        std::string s, k;
        (*it)[0].get_section (s);

        if (s != section)
            continue;

        StyleLines::iterator lit;
        for (lit = it->begin (); lit != it->end (); lit++) {
            lit->get_key (k);
            if (k == key) {
                lit->get_value (value);
                return true;
            }
        }
    }

    return false;
}

bool
StyleFile::get_string_array (std::vector<std::string> &value,
                             std::string section, std::string key)
{
    StyleLines *lines = find_section (section);
    if (!lines)
        return false;

    // find entry
    StyleLines::iterator lit;
    for (lit = lines->begin (); lit != lines->end (); lit++) {
        std::string k;
        lit->get_key (k);
        if (k == key) {
            lit->get_value_array (value);
            return true;
        }
    }

    return false;
}

void
StyleFile::set_string (std::string section, std::string key, std::string value)
{
    StyleLines *lines = find_section (section);
    if (lines) {
        // find entry
        StyleLines::iterator lit, last = lines->begin () + 1;
        for (lit = last; lit != lines->end (); lit++) {
            StyleLineType type = lit->get_type ();
            if (type != FCITX_ANTHY_STYLE_LINE_SPACE)
                last = lit + 1;

            std::string k;
            lit->get_key (k);

            // replace existing entry
            if (k.length () > 0 && k == key) {
                lit->set_value (value);
                return;
            }
        }

        // append new entry if no mathced entry exists.
        lines->insert (last, StyleLine (this, key, value));

    } else {
        StyleLines &newsec = append_new_section (section);

        // append new entry
        newsec.push_back (StyleLine (this, key, value));
    }
}

void
StyleFile::set_string_array (std::string section, std::string key,
                             std::vector<std::string> &value)
{
    StyleLines *lines = find_section (section);
    if (lines) {
        // find entry
        StyleLines::iterator lit, last = lines->begin () + 1;
        for (lit = last; lit != lines->end (); lit++) {
            StyleLineType type = lit->get_type ();
            if (type != FCITX_ANTHY_STYLE_LINE_SPACE)
                last = lit;

            std::string k;
            lit->get_key (k);

            // replace existing entry
            if (k.length () > 0 && k == key) {
                lit->set_value_array (value);
                return;
            }
        }

        // append new entry if no mathced entry exists.
        lines->insert (last + 1, StyleLine (this, key, value));

    } else {
        StyleLines &newsec = append_new_section (section);

        // append new entry
        newsec.push_back (StyleLine (this, key, value));
    }
}

bool
StyleFile::get_section_list (StyleSections &sections)
{
    sections = m_sections;
    return true;
}

bool
StyleFile::get_entry_list (StyleLines &lines, std::string section)
{
    StyleSections::iterator it;
    for (it = m_sections.begin (); it != m_sections.end (); it++) {
        if (it->size () <= 0)
            continue;

        std::string s;
        (*it)[0].get_section (s);
        if (s == section) {
            lines = (*it);
            return true;
        }
    }

    return false;
}

bool
StyleFile::get_key_list (std::vector<std::string> &keys, std::string section)
{
    StyleLines *lines = find_section (section);
    if (!lines)
        return false;

    StyleLines::iterator lit;
    for (lit = lines->begin (); lit != lines->end (); lit++) {
        if (lit->get_type () != FCITX_ANTHY_STYLE_LINE_KEY)
            continue;

        std::string key;
        lit->get_key (key);
        keys.push_back (key);
    }
    return true;
}

void
StyleFile::delete_key (std::string section, std::string key)
{
    StyleLines *lines = find_section (section);
    if (!lines)
        return;

    // find entry
    StyleLines::iterator lit;
    for (lit = lines->begin (); lit != lines->end (); lit++) {
        std::string k;
        lit->get_key (k);
        if (k == key) {
            lines->erase (lit);
            return;
        }
    }
}

void
StyleFile::delete_section (std::string section)
{
    StyleSections::iterator it;
    for (it = m_sections.begin (); it != m_sections.end (); it++) {
        if (it->size () <= 0)
            continue;

        StyleLines::iterator lit;
        std::string s;
        (*it)[0].get_section (s);
        if (s == section) {
            m_sections.erase (it);
            return;
        }
    }
}

Key2KanaTable *
StyleFile::get_key2kana_table (std::string section)
{
    Key2KanaTable *table = NULL;

    std::vector<std::string> keys;
    bool success = get_key_list (keys, section);
    if (success) {
        table = new Key2KanaTable (get_title ());
        std::vector<std::string>::iterator it;
        for (it = keys.begin (); it != keys.end (); it++) {
            std::vector<std::string> array;
            get_string_array (array, section, *it);
            table->append_rule (*it, array);
        }
    }

    return table;
}

void
StyleFile::setup_default_entries (void)
{
    m_title    = "User defined";
    m_sections.push_back (StyleLines ());

    m_sections.push_back (StyleLines ());
    StyleLines &newsec = m_sections.back ();
    std::string str = std::string ("Title") + std::string ("=") + escape (m_title);
    newsec.push_back (StyleLine (this, str.c_str ()));
}

StyleLines *
StyleFile::find_section (const std::string  &section)
{
    // find section
    StyleSections::iterator it;
    for (it = m_sections.begin (); it != m_sections.end (); it++) {
        if (it->size () <= 0)
            continue;

        std::string s;
        (*it)[0].get_section (s);

        if (s == section)
            return &(*it);
    }

    return NULL;
}

StyleLines &
StyleFile::append_new_section (const std::string &section)
{
    // append space before new section
    if (!m_sections.empty()) {
        StyleLines &sec = m_sections.back ();
        if (sec.empty() ||
            sec.back().get_type() != FCITX_ANTHY_STYLE_LINE_SPACE)
        {
            sec.push_back (StyleLine (this, ""));
        }
    }

    //
    // append new section
    //
    m_sections.push_back (StyleLines ());
    StyleLines &newsec = m_sections.back ();

    // new section entry
    std::string str = std::string ("[") + std::string (section) + std::string ("]");
    newsec.push_back (StyleLine (this, str.c_str ()));

    return newsec;
}
