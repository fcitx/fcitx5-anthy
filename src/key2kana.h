/*
 * SPDX-FileCopyrightText: 2004 Hiroyuki Ikezoe
 * SPDX-FileCopyrightText: 2004 Takuro Ashie
 * SPDX-FileCopyrightText: 2012-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_ANTHY_KEY2KANA_H_
#define _FCITX5_ANTHY_KEY2KANA_H_

#include "key2kana_base.h"
#include "key2kana_table.h"
#include <fcitx-utils/key.h>
#include <fcitx/event.h>
#include <string>

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
    bool isPseudoAsciiMode() const { return isInPseudoAsciiMode_; }
    bool processPseudoAsciiMode(const std::string &wstr) override;
    void resetPseudoAsciiMode() override;

private:
    bool append(const std::string &str, std::string &result,
                std::string &pending) override;

    Key2KanaTableSet &tables_;

    // state
    fcitx::Key lastKey_;
    std::string pending_;
    Key2KanaRule exactMatch_;
    int pseudoAsciiMode_;
    bool isInPseudoAsciiMode_;
};

#endif // _FCITX5_ANTHY_KEY2KANA_H_
