/*
 * SPDX-FileCopyrightText: 2005 Takuro Ashie
 * SPDX-FileCopyrightText: 2017-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_ANTHY_STYLE_FILE_H_
#define _FCITX5_ANTHY_STYLE_FILE_H_

#include <fcitx-utils/macros.h>
#include <string>
#include <vector>

class StyleLine;
class StyleSection;
class StyleFile;

typedef std::vector<StyleLine> StyleLines;
typedef std::vector<StyleLines> StyleSections;
typedef std::vector<StyleFile> StyleFiles;

enum class StyleLineType {
    UNKNOWN,
    SPACE,
    COMMENT,
    SECTION,
    KEY,
};

class StyleLine {
public:
    StyleLine(StyleFile *style_file, std::string line);
    ~StyleLine();

public:
    StyleLineType type() const;
    std::string line() { return line_; }
    bool get_section(std::string &section) const;
    bool get_key(std::string &key) const;
    bool get_value(std::string &value) const;
    bool get_value_array(std::vector<std::string> &value) const;

private:
    StyleFile *styleFile_;
    std::string line_;
    StyleLineType type_;
};

class StyleFile {
public:
    StyleFile();
    FCITX_INLINE_DEFINE_DEFAULT_DTOR_AND_MOVE_WITHOUT_SPEC(StyleFile);

public:
    bool load(const std::string &filename);

    const std::string &title() const;

    bool getKeyList(std::vector<std::string> &keys, std::string section) const;
    bool getString(std::string &value, std::string section,
                   std::string key) const;
    bool getStringArray(std::vector<std::string> &value, std::string section,
                        std::string key) const;

private:
    void clear();
    void setupDefaultEntries();
    const StyleLines *findSection(const std::string &section) const;

private:
    std::string title_;
    StyleSections sections_;
};

#endif // _FCITX5_ANTHY_STYLE_FILE_H_
