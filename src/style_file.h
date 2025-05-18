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
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class StyleLine;
class StyleSection;
class StyleFile;

using StyleLines = std::vector<StyleLine>;
using StyleSections = std::vector<StyleLines>;
using StyleFiles = std::vector<StyleFile>;

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

    StyleLineType type() const;
    std::string line() { return line_; }
    std::string get_section() const;
    std::string get_key() const;
    std::string get_value() const;
    std::vector<std::string> get_value_array() const;

private:
    StyleFile *styleFile_;
    std::string line_;
    StyleLineType type_;
};

class StyleFile {
public:
    StyleFile();
    FCITX_INLINE_DEFINE_DEFAULT_DTOR_AND_MOVE_WITHOUT_SPEC(StyleFile);

    bool load(const std::string &filename);

    const std::string &title() const;

    std::optional<std::vector<std::string>>
    getKeyList(std::string_view section) const;
    std::optional<std::string> getString(std::string_view section,
                                         std::string_view key) const;
    std::optional<std::vector<std::string>>
    getStringArray(std::string_view section, std::string_view key) const;

private:
    void clear();
    void setupDefaultEntries();
    const StyleLines *findSection(std::string_view section) const;

    std::string title_;
    StyleSections sections_;
};

#endif // _FCITX5_ANTHY_STYLE_FILE_H_
