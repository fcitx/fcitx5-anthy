/*
 * SPDX-FileCopyrightText: 2005 Takuro Ashie
 * SPDX-FileCopyrightText: 2017-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
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
