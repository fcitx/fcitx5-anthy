//
// Copyright (C) 2004 Hiroyuki Ikezoe
// Copyright (C) 2004 Takuro Ashie
// Copyright (C) 2012~2017 by CSSlayer
// wengxt@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _FCITX5_ANTHY_KEY2KANA_H_
#define _FCITX5_ANTHY_KEY2KANA_H_

#include "default_tables.h"
#include "key2kana_base.h"
#include "key2kana_table.h"

class AnthyState;

class Key2KanaConvertor : public Key2KanaConvertorBase {
public:
    Key2KanaConvertor(AnthyState &anthy, Key2KanaTableSet &tables);
    virtual ~Key2KanaConvertor();

    bool canAppend(const fcitx::KeyEvent &key,
                   bool ignore_space = false) override;
    bool append(const fcitx::KeyEvent &key, std::string &result,
                std::string &pending, std::string &raw) override;
    void clear() override;

    bool isPending() const override;
    std::string pending() const override;
    std::string flushPending() override;
    void resetPending(const std::string &result,
                      const std::string &raw) override;
    void setPseudoAsciiMode(int mode) { pseudoAsciiMode_ = mode; }
    bool isPseudoAsciiMode() { return isInPseudoAsciiMode_; }
    bool processPseudoAsciiMode(const std::string &wstr) override;
    void resetPseudoAsciiMode() override;

private:
    bool append(const std::string &str, std::string &result,
                std::string &pending) override;

private:
    Key2KanaTableSet &tables_;

    // state
    fcitx::Key lastKey_;
    std::string pending_;
    Key2KanaRule exactMatch_;
    int pseudoAsciiMode_;
    bool isInPseudoAsciiMode_;
};

#endif // _FCITX5_ANTHY_KEY2KANA_H_
