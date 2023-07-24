/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  SPDX-FileCopyrightText: 2005 Takuro Ashie
 *  SPDX-FileCopyrightText: 2012 CSSlayer <wengxt@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "style_file.h"
#include <fcitx-utils/charutils.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/stringutils.h>
#include <fstream>

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
            if (i < dest.size() && dest[i] == '\\')
                i++;
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

bool StyleLine::get_section(std::string &section) const {
    if (type() != StyleLineType::SECTION) {
        return false;
    }

    auto result = fcitx::stringutils::trim(line_);
    // remove [ and ]
    result.pop_back();
    result = result.substr(1);
    section = std::move(result);

    return true;
}

bool StyleLine::get_key(std::string &key) const {
    if (type() != StyleLineType::KEY)
        return false;

    // skip space ahead.
    auto spos = line_.find_first_not_of(FCITX_WHITESPACE);
    if (spos == std::string::npos) {
        key = std::string();
        return true;
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
    for (--epos; epos >= spos && fcitx::charutils::isspace(line_[epos]); epos--)
        ;
    if (!fcitx::charutils::isspace(line_[epos]))
        epos++;

    if (spos < epos && epos <= line_.length()) {
        key = unescape(line_.substr(spos, epos - spos));
    } else
        key = std::string();

    return true;
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
    if (spos >= str.length())
        return true;
    else
        spos++;
    for (; spos < str.length() && fcitx::charutils::isspace(str[spos]); spos++)
        ;

    return spos;
}

bool StyleLine::get_value(std::string &value) const {
    if (type() != StyleLineType::KEY)
        return false;

    unsigned int spos = get_value_position(line_);
    unsigned int epos = line_.length();

    value = unescape(line_.substr(spos, epos - spos));

    return true;
}

bool StyleLine::get_value_array(std::vector<std::string> &value) const {
    if (type() != StyleLineType::KEY)
        return false;

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
            if (head_of_element == epos)
                str = std::string();
            else
                str = unescape(
                    line_.substr(head_of_element, i - head_of_element));
            value.push_back(str);
            head_of_element = i + 1;
        }
    }

    return true;
}

StyleFile::StyleFile() { setupDefaultEntries(); }

bool StyleFile::load(const std::string &filename) {
    clear();
    setupDefaultEntries();

    std::ifstream in_file(filename);
    if (!in_file)
        return false;

    clear();

    sections_.push_back(StyleLines());
    StyleLines *section = &sections_[0];
    unsigned int section_id = 0;

    char buf[MAX_LINE_LENGTH];
    do {
        in_file.getline(buf, MAX_LINE_LENGTH);
        if (in_file.eof())
            break;

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
            std::string key;
            line.get_key(key);
            if (key == "Title") {
                line.get_value(title_);
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

bool StyleFile::getString(std::string &value, std::string section,
                          std::string key) const {
    for (auto it = sections_.begin(); it != sections_.end(); it++) {
        if (it->size() <= 0)
            continue;

        std::string s, k;
        (*it)[0].get_section(s);

        if (s != section)
            continue;

        for (auto lit = it->begin(); lit != it->end(); lit++) {
            lit->get_key(k);
            if (k == key) {
                lit->get_value(value);
                return true;
            }
        }
    }

    return false;
}

bool StyleFile::getStringArray(std::vector<std::string> &value,
                               std::string section, std::string key) const {
    const StyleLines *lines = findSection(section);
    if (!lines)
        return false;

    // find entry
    for (auto lit = lines->begin(); lit != lines->end(); lit++) {
        std::string k;
        lit->get_key(k);
        if (k == key) {
            lit->get_value_array(value);
            return true;
        }
    }

    return false;
}

bool StyleFile::getKeyList(std::vector<std::string> &keys,
                           std::string section) const {
    const StyleLines *lines = findSection(section);
    if (!lines)
        return false;

    for (auto lit = lines->begin(); lit != lines->end(); lit++) {
        if (lit->type() != StyleLineType::KEY)
            continue;

        std::string key;
        lit->get_key(key);
        keys.push_back(key);
    }
    return true;
}

void StyleFile::setupDefaultEntries() {
    title_ = "User defined";
    sections_.push_back(StyleLines());

    sections_.push_back(StyleLines());
    StyleLines &newsec = sections_.back();
    std::string str = std::string("Title") + std::string("=") + escape(title_);
    newsec.push_back(StyleLine(this, str.c_str()));
}

const StyleLines *StyleFile::findSection(const std::string &section) const {
    // find section
    for (auto it = sections_.begin(); it != sections_.end(); it++) {
        if (it->size() <= 0)
            continue;

        std::string s;
        (*it)[0].get_section(s);

        if (s == section)
            return &(*it);
    }

    return nullptr;
}
