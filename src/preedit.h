/*
 * SPDX-FileCopyrightText: 2004 Takuro Ashie
 * SPDX-FileCopyrightText: 2017-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_ANTHY_PREEDIT_H_
#define _FCITX5_ANTHY_PREEDIT_H_

#include "config.h"
#include "conversion.h"
#include "key2kana_table.h"
#include "reading.h"
#include <anthy/anthy.h>
#include <fcitx-utils/key.h>
#include <fcitx/candidatelist.h>
#include <fcitx/event.h>
#include <memory>
#include <string>

#define FCITX_ANTHY_PSEUDO_ASCII_TRIGGERED_CAPITALIZED (1 << 0)
#define FCITX_ANTHY_PSEUDO_ASCII_TRIGGERED_COUPLE_OF_CAPITAL (1 << 1)

class AnthyState;

class Preedit {
public:
    Preedit(AnthyState &anthy);
    virtual ~Preedit();

    // getting status
    unsigned int length();
    unsigned int utf8Length();
    std::string string();
    void updatePreedit();

    bool isPreediting();
    bool isConverting();
    bool isPredicting();
    bool isReconverting();

    // for handling the preedit string
    bool canProcessKeyEvent(const fcitx::KeyEvent &key);
    // return true if commiting is needed.
    bool processKeyEvent(const fcitx::KeyEvent &key);
    bool append(const fcitx::KeyEvent &key, const std::string &string);
    bool append(const fcitx::Key &key, const std::string &string);
    void erase(bool backward = true);
    void finish();

    // for handling the conversion string
    void convert(CandidateType type = FCITX_ANTHY_CANDIDATE_DEFAULT,
                 bool single_segment = false);
    void convert(const std::string &source, bool single_segment = false);
    void revert();
    void commit(int segment_id = -1, bool learn = true);

    // for prediction
    void predict();

    // segments of the converted sentence
    int nrSegments();
    std::string segmentString(int segment_id = -1);
    int selectedSegment();
    void selectSegment(int segment_id);
    int segmentSize(int segment_id = -1);
    void resizeSegment(int relative_size, int segment_id = -1);

    // candidates for a segment
    std::unique_ptr<fcitx::CommonCandidateList> candidates(int segment_id = -1);
    int selectedCandidate(int segment_id = -1);
    void selectCandidate(int candidate_id, int segment_id = -1);

    // for handling the caret
    unsigned int caretPos();
    void setCaretPosByChar(unsigned int pos);
    void moveCaret(int step);

    // clear all or part of the string.
    void clear(int segment_id = -1);

    // preferences
    void setInputMode(InputMode mode);
    InputMode inputMode() const;
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
    void setNumberHalf(bool half);
    void setPseudoAsciiMode(int mode);
    bool isPseudoAsciiMode();
    void resetPseudoAsciiMode();

private:
    bool isCommaOrPeriod(const std::string &str) const;

    AnthyState &state_;

    // converter objects
    Reading reading_;
    Conversion conversion_;

    // mode flags
    InputMode inputMode_;

    // source string for reconversion
    std::string source_;
};

#endif // _FCITX5_ANTHY_PREEDIT_H_
