/*
 * SPDX-FileCopyrightText: 2004 Hiroyuki Ikezoe
 * SPDX-FileCopyrightText: 2004 Takuro Ashie
 * SPDX-FileCopyrightText: 2012-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_ANTHY_KEY2KANA_BASE_H_
#define _FCITX5_ANTHY_KEY2KANA_BASE_H_

#include <fcitx/event.h>

class AnthyState;
class AnthyConfig;

class Key2KanaConvertorBase {
public:
    Key2KanaConvertorBase(AnthyState &state)
        : state_(state), caseSensitive_(true){};
    virtual ~Key2KanaConvertorBase() = default;

    virtual bool canAppend(const fcitx::KeyEvent &key,
                           bool ignore_space = false) = 0;
    virtual bool append(const fcitx::KeyEvent &key, std::string &result,
                        std::string &pending, std::string &raw) = 0;
    virtual bool append(const std::string &raw, std::string &result,
                        std::string &pending) = 0;
    virtual void clear() = 0;

    virtual bool isPending() const = 0;
    virtual std::string pending() const = 0;
    virtual std::string flushPending() = 0;
    virtual void resetPending(const std::string &result,
                              const std::string &raw) = 0;

    virtual void resetPseudoAsciiMode() {}
    virtual bool processPseudoAsciiMode(const std::string &) { return false; }

    void setCaseSensitive(bool sensitive) { caseSensitive_ = sensitive; }
    bool isCaseSensitive() { return caseSensitive_; }

protected:
    const AnthyConfig &config() const;

    AnthyState &state_;
    bool caseSensitive_;
};

#endif // _FCITX5_ANTHY_KEY2KANA_BASE_H_
