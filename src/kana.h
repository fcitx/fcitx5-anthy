//
// Copyright (C) 2005 Takuro Ashie
// Copyright (C) 2017~2017 by CSSlayer
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
#ifndef _FCITX5_ANTHY_KANA_H_
#define _FCITX5_ANTHY_KANA_H_

#include "default_tables.h"
#include "key2kana_base.h"
#include "key2kana_table.h"

class AnthyState;

class KanaConvertor : public Key2KanaConvertorBase {
public:
    KanaConvertor(AnthyState &anthy);
    virtual ~KanaConvertor();

    bool canAppend(const fcitx::KeyEvent &key,
                   bool ignore_space = false) override;
    bool append(const fcitx::KeyEvent &key, std::string &result,
                std::string &pending, std::string &raw) override;
    bool append(const std::string &raw, std::string &result,
                std::string &pending) override;
    void clear() override;

    bool isPending() const override;
    std::string pending() const override;
    std::string flushPending() override;
    void resetPending(const std::string &result,
                      const std::string &raw) override;

private:
    // state
    std::string pending_;
};

#endif // _FCITX5_ANTHY_KANA_H_
