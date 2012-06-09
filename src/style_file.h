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

#ifndef __FCITX_ANTHY_STYLE_FILE_H__
#define __FCITX_ANTHY_STYLE_FILE_H__

#include <vector>
#include <string>

class Key2KanaTable;
class StyleLine;
class StyleSection;
class StyleFile;

typedef std::vector<StyleLine>  StyleLines;
typedef std::vector<StyleLines> StyleSections;
typedef std::vector<StyleFile>  StyleFiles;

typedef enum {
    FCITX_ANTHY_STYLE_LINE_UNKNOWN,
    FCITX_ANTHY_STYLE_LINE_SPACE,
    FCITX_ANTHY_STYLE_LINE_COMMENT,
    FCITX_ANTHY_STYLE_LINE_SECTION,
    FCITX_ANTHY_STYLE_LINE_KEY,
} StyleLineType;

class StyleLine
{
public:
    StyleLine (StyleFile  *style_file,
               std::string      line);
    StyleLine (StyleFile  *style_file,
               std::string      key,
               std::string      value);
    StyleLine (StyleFile  *style_file,
               std::string      key,
               std::vector<std::string> &value);
    ~StyleLine ();

public:
    StyleLineType get_type        (void);
    void          get_line        (std::string     &line) { line = m_line; }
    bool          get_section     (std::string     &section);
    bool          get_key         (std::string     &key);
    bool          get_value       (std::string     &value);
    void          set_value       (std::string      value);
    bool          get_value_array (std::vector<std::string> &value);
    void          set_value_array (std::vector<std::string> &value);

private:
    StyleFile     *m_style_file;
    std::string         m_line;
    StyleLineType  m_type;
};

class StyleFile
{
public:
    StyleFile ();
    ~StyleFile ();

public:
    bool   load                  (const char *filename);
    bool   save                  (const char *filename);

    std::string get_title             (void);
    std::string get_file_name         (void);

    bool   get_section_list      (StyleSections &sections);
    bool   get_entry_list        (StyleLines    &lines,
                                  std::string         section);
    bool   get_key_list          (std::vector<std::string> &keys,
                                  std::string         section);
    bool   get_string            (std::string        &value,
                                  std::string         section,
                                  std::string         key);
    bool   get_string_array      (std::vector<std::string> &value,
                                  std::string         section,
                                  std::string         key);

    void   set_string            (std::string         section,
                                  std::string         key,
                                  std::string         value);
    void   set_string_array      (std::string         section,
                                  std::string         key,
                                  std::vector<std::string> &value);

    void   delete_key            (std::string         section,
                                  std::string         key);
    void   delete_section        (std::string         section);

public: // for getting specific data
    Key2KanaTable *
           get_key2kana_table    (std::string         section);

private:
    void   clear                 (void);
    void   setup_default_entries (void);
    StyleLines *
           find_section          (const std::string  &section);
    StyleLines &
           append_new_section    (const std::string  &section);

private:
    std::string        m_filename;
    std::string        m_format_version;
    std::string        m_title;
    std::string        m_version;

    StyleSections m_sections;
};

#endif /* __FCITX_ANTHY_STYLE_FILE_H__ */
