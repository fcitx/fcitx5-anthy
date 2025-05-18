/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  SPDX-FileCopyrightText: 2005 Takuro Ashie
 *  SPDX-FileCopyrightText: 2012 CSSlayer <wengxt@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "style_file.h"
#include <cstddef>
#include <fcitx-utils/charutils.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/macros.h>
#include <fcitx-utils/stringutils.h>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
const int MAX_LINE_LENGTH = 4096;

std::string escape(const std::string &str) {
    std::string dest = str;

    for (unsigned int i = 0; i < dest.size(); i++) {
        if (dest[i] == '#' ||                   // for comment
            dest[i] == '\\' ||                  // for backslash itself
            dest[i] == '=' ||                   // for separatort
            dest[i] == '[' || dest[i] == ']' || // for section
            dest[i] == ',' ||                   // for array
            dest[i] == ' ' || dest[i] == '\t')  // for space
        {
            dest.insert(i, "\\");
            i++;
        }
    }

    return dest;
}

std::string unescape(const std::string &str) {
    std::string dest = str;

    for (unsigned int i = 0; i < dest.size(); i++) {
        if (dest[i] == '\\') {
            dest.erase(i, 1);
            if (i < dest.size() && dest[i] == '\\') {
                i++;
            }
        }
    }

    return dest;
}
} // namespace

StyleLine::StyleLine(StyleFile *style_file, std::string line)
    : styleFile_(style_file), line_(std::move(line)),
      type_(StyleLineType::UNKNOWN) {

    auto trimmed = fcitx::stringutils::trimView(line_);
    if (trimmed.length() == 0) {
        type_ = StyleLineType::SPACE;
    } else if (trimmed[0] == '#') {
        type_ = StyleLineType::COMMENT;
    } else if (trimmed.front() == '[' && trimmed.back() == ']') {
        type_ = StyleLineType::SECTION;
    } else {
        type_ = StyleLineType::KEY;
    }
}

StyleLine::~StyleLine() {}

StyleLineType StyleLine::type() const { return type_; }

std::string StyleLine::get_section() const {
    std::string section;
    if (type() != StyleLineType::SECTION) {
        return section;
    }

    auto result = fcitx::stringutils::trim(line_);
    // remove [ and ]
    result.pop_back();
    result = result.substr(1);
    return result;
}

std::string StyleLine::get_key() const {
    std::string key;
    if (type() != StyleLineType::KEY) {
        return key;
    }

    // skip space ahead.
    auto spos = line_.find_first_not_of(FCITX_WHITESPACE);
    if (spos == std::string::npos) {
        return key;
    }
    size_t epos = spos;

    for (epos = spos; epos < line_.length(); epos++) {
        if (line_[epos] == '\\') {
            epos++;
            continue;
        }
        if (line_[epos] == '=') {
            break;
        }
    }
    for (--epos; epos >= spos && fcitx::charutils::isspace(line_[epos]);
         epos--) {
        ;
    }
    if (!fcitx::charutils::isspace(line_[epos])) {
        epos++;
    }

    if (spos < epos && epos <= line_.length()) {
        key = unescape(line_.substr(spos, epos - spos));
    } else {
        key = std::string();
    }

    return key;
}

static int get_value_position(std::string_view str) {
    unsigned int spos;
    for (spos = 0; spos < str.length(); spos++) {
        if (str[spos] == '\\') {
            spos++;
            continue;
        }
        if (str[spos] == '=') {
            break;
        }
    }
    if (spos >= str.length()) {
        return true;
    }
    spos++;
    for (; spos < str.length() && fcitx::charutils::isspace(str[spos]);
         spos++) {
    }

    return spos;
}

std::string StyleLine::get_value() const {
    std::string value;
    if (type() != StyleLineType::KEY) {
        return value;
    }

    unsigned int spos = get_value_position(line_);
    unsigned int epos = line_.length();

    return unescape(line_.substr(spos, epos - spos));
}

std::vector<std::string> StyleLine::get_value_array() const {
    std::vector<std::string> value;
    if (type() != StyleLineType::KEY) {
        return value;
    }
    unsigned int spos = get_value_position(line_);
    unsigned int epos = line_.length();

    unsigned int head_of_element = spos;
    for (unsigned int i = spos; i <= epos; i++) {
        if (i < epos && line_[i] == '\\') {
            i++;
            continue;
        }

        if (i == epos || line_[i] == ',') {
            std::string str;
            if (head_of_element == epos) {
                str = std::string();
            } else {
                str = unescape(
                    line_.substr(head_of_element, i - head_of_element));
            }
            value.push_back(str);
            head_of_element = i + 1;
        }
    }

    return value;
}

StyleFile::StyleFile() { setupDefaultEntries(); }

bool StyleFile::load(const std::string &filename) {
    clear();
    setupDefaultEntries();

    std::ifstream in_file(filename);
    if (!in_file) {
        return false;
    }

    clear();

    sections_.push_back(StyleLines());
    StyleLines *section = sections_.data();
    unsigned int section_id = 0;

    char buf[MAX_LINE_LENGTH];
    do {
        in_file.getline(buf, MAX_LINE_LENGTH);
        if (in_file.eof()) {
            break;
        }

        std::string dest = buf;
        StyleLine line(this, dest);
        StyleLineType type = line.type();

        if (type == StyleLineType::SECTION) {
            sections_.push_back(StyleLines());
            section = &sections_.back();
            section_id++;
        }

        section->push_back(line);

        if (section_id == 0) {
            if (line.get_key() == "Title") {
                title_ = line.get_value();
            }
        }
    } while (!in_file.eof());

    in_file.close();

    return true;
}

void StyleFile::clear() {
    title_ = std::string();
    sections_.clear();
}

const std::string &StyleFile::title() const { return title_; }

std::optional<std::string> StyleFile::getString(std::string_view section,
                                                std::string_view key) const {
    for (const auto &lines : sections_) {
        if (lines.empty()) {
            continue;
        }

        if (lines[0].get_section() != section) {
            continue;
        }

        for (const auto &line : lines) {
            if (line.get_key() == key) {
                return line.get_value();
            }
        }
    }

    return std::nullopt;
}

std::optional<std::vector<std::string>>
StyleFile::getStringArray(std::string_view section,
                          std::string_view key) const {
    const StyleLines *lines = findSection(section);
    if (!lines) {
        return std::nullopt;
    }

    // find entry
    for (const auto &line : *lines) {
        if (line.get_key() == key) {
            return line.get_value_array();
        }
    }

    return std::nullopt;
}

std::optional<std::vector<std::string>>
StyleFile::getKeyList(std::string_view section) const {
    const StyleLines *lines = findSection(section);
    if (!lines) {
        return std::nullopt;
    }

    std::vector<std::string> keys;
    for (const auto &line : *lines) {
        if (line.type() != StyleLineType::KEY) {
            continue;
        }

        keys.push_back(line.get_key());
    }
    return {std::move(keys)};
}

void StyleFile::setupDefaultEntries() {
    title_ = "User defined";
    sections_.push_back(StyleLines());

    sections_.push_back(StyleLines());
    StyleLines &newsec = sections_.back();
    std::string str = std::string("Title") + std::string("=") + escape(title_);
    newsec.push_back(StyleLine(this, str));
}

const StyleLines *StyleFile::findSection(std::string_view section) const {
    // find section
    for (const auto &lines : sections_) {
        if (lines.empty()) {
            continue;
        }

        if (lines[0].get_section() == section) {
            return &lines;
        }
    }

    return nullptr;
}
