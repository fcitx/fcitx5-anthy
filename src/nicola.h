/*
 * SPDX-FileCopyrightText: 2004 Hiroyuki Ikezoe
 * SPDX-FileCopyrightText: 2004 Takuro Ashie
 * SPDX-FileCopyrightText: 2012 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __FCITX_ANTHY_NICOLA_H__
#define __FCITX_ANTHY_NICOLA_H__

#include <fcitx-utils/event.h>
#include <stdint.h>
#include <sys/time.h>

#include "key2kana_base.h"
#include "key2kana_table.h"

class AnthyState;

typedef enum {
    FCITX_ANTHY_NICOLA_SHIFT_NONE,
    FCITX_ANTHY_NICOLA_SHIFT_LEFT,
    FCITX_ANTHY_NICOLA_SHIFT_RIGHT,
} NicolaShiftType;

class NicolaConvertor : public Key2KanaConvertorBase {
public:
    NicolaConvertor(AnthyState &anthy, Key2KanaTableSet &tables);
    virtual ~NicolaConvertor();

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

public:
    void processTimeout();

private:
    void search(const fcitx::Key &key, NicolaShiftType shift_type,
                std::string &result, std::string &raw);
    bool handleVoicedConsonant(std::string &result, std::string &pending);
    bool isCharKey(const fcitx::KeyEvent &key);
    bool isThumbKey(const fcitx::Key &key);
    bool isLeftThumbKey(const fcitx::Key &key);
    bool isRightThumbKey(const fcitx::Key &key);
    NicolaShiftType thumbKeyType(const fcitx::Key &key);
    bool emitKeyEvent(const fcitx::Key &key);
    bool emitKeyEvent(const fcitx::KeyEvent &key);
    void setAlarm(int time_msec);
    bool stop();
    int thumbKey(const fcitx::KeyEvent &key);

private:
    Key2KanaTableSet &tables_;

    // state
    fcitx::Key prevCharKey_;

    fcitx::Key repeatCharKey_;

    std::unique_ptr<fcitx::EventSourceTime> timer_;
    bool processingTimeout_;

    std::string pending_;
    fcitx::Key throughKeyEvent;
    fcitx::Key repeatThumbKey_;
    fcitx::Key prevThumbKey_;
};

#endif /* __FCITX_ANTHY_NICOLA_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
