/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  SPDX-FileCopyrightText: 2005 Takuro Ashie
 *  SPDX-FileCopyrightText: 2012 CSSlayer <wengxt@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __FCITX_ANTHY_READING_H__
#define __FCITX_ANTHY_READING_H__

#include "config.h"
#include "kana.h"
#include "key2kana.h"
#include "key2kana_base.h"
#include "key2kana_table.h"
#include "nicola.h"
#include <fcitx/event.h>
#include <string>
#include <vector>

class AnthyState;

enum StringType {
    FCITX_ANTHY_STRING_LATIN,
    FCITX_ANTHY_STRING_WIDE_LATIN,
    FCITX_ANTHY_STRING_HIRAGANA,
    FCITX_ANTHY_STRING_KATAKANA,
    FCITX_ANTHY_STRING_HALF_KATAKANA,
};

class Reading;
class ReadingSegment;
using ReadingSegments = std::vector<ReadingSegment>;

class ReadingSegment {
    friend class Reading;

public:
    ReadingSegment();
    virtual ~ReadingSegment();

    const std::string &get() { return kana; }
    const std::string &get_raw() { return raw; }

    void split(ReadingSegments &segments);

private:
    std::string raw;
    std::string kana;
};

class Reading {
public:
    Reading(AnthyState &anthy);
    virtual ~Reading();

    bool canProcesKeyEvent(const fcitx::KeyEvent &key);
    bool processKeyEvent(const fcitx::KeyEvent &key);
    void finish();
    void clear();

    std::string getByChar(unsigned int start = 0, int length = -1,
                          StringType type = FCITX_ANTHY_STRING_HIRAGANA);
    std::string getRawByChar(unsigned int start = 0, int length = -1);
    bool append(const fcitx::KeyEvent &key, const std::string &string);
    void erase(unsigned int start = 0, int length = -1,
               bool allow_split = false);

    unsigned int length();
    unsigned int utf8Length();
    unsigned int caretPos();
    unsigned int caretPosByChar();
    void setCaretPosByChar(unsigned int pos);
    void moveCaret(int step, bool allow_split = false);

    void setTypingMethod(TypingMethod method);
    TypingMethod typingMethod() const;
    void setPeriodStyle(PeriodStyle style);
    PeriodStyle periodStyle() const;
    void setCommaStyle(CommaStyle style);
    CommaStyle commaStyle() const;
    void setBracketStyle(BracketStyle style);
    BracketStyle bracketStyle() const;
    void setSlashStyle(SlashStyle style);
    SlashStyle slashStyle() const;
    void setSymbolHalf(bool half);
    bool isSymbolHalf();
    void setNumberHalf(bool half);
    bool isNumberHalf();
    void setPseudoAsciiMode(int mode);
    bool isPseudoAsciiMode();
    void resetPseudoAsciiMode();

private:
    void resetPending();
    void splitSegment(unsigned int seg_id);

    AnthyState &state_;

    // tables
    Key2KanaTableSet key2kanaTables_;
    Key2KanaTableSet nicolaTables_;

    // convertors
    Key2KanaConvertor key2kanaNormal_;
    KanaConvertor kana_;
    NicolaConvertor nicola_;
    Key2KanaConvertorBase *key2kana_;

    // state
    ReadingSegments segments_;
    unsigned int segmentPos_;
    unsigned int caretOffset_;
};

#endif /* __FCITX_ANTHY_READING_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
