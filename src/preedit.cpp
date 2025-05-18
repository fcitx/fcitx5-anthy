/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  SPDX-FileCopyrightText: 2004-2005 Takuro Ashie
 *  SPDX-FileCopyrightText: 2012 CSSlayer <wengxt@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "preedit.h"
#include "config.h"
#include "conversion.h"
#include "default_tables.h"
#include "key2kana_table.h"
#include "reading.h"
#include "state.h"
#include "utils.h"
#include <array>
#include <fcitx-utils/key.h>
#include <fcitx-utils/textformatflags.h>
#include <fcitx/candidatelist.h>
#include <fcitx/event.h>
#include <fcitx/inputpanel.h>
#include <fcitx/text.h>
#include <memory>
#include <string>

static const std::array<ConvRule, 1> *get_period_rule(TypingMethod method,
                                                      PeriodStyle period);
static const std::array<ConvRule, 1> *get_comma_rule(TypingMethod method,
                                                     CommaStyle period);

Preedit::Preedit(AnthyState &anthy)
    : state_(anthy), reading_(anthy), conversion_(state_, reading_),
      inputMode_(InputMode::HIRAGANA) {}

Preedit::~Preedit() {}

/*
 * getting status
 */
unsigned int Preedit::length() {
    if (isConverting()) {
        return conversion_.length();
    }
    return reading_.length();
}

/*
 * getting status
 */
unsigned int Preedit::utf8Length() {
    if (isConverting()) {
        return conversion_.utf8Length();
    }
    return reading_.utf8Length();
}

std::string Preedit::string() {
    if (isConverting()) {
        return conversion_.get();
    }
    if (!source_.empty()) {
        return source_;
    }
    switch (inputMode_) {
    case InputMode::KATAKANA:
        return util::convert_to_katakana(reading_.getByChar());

    case InputMode::HALF_KATAKANA:
        return util::convert_to_katakana(reading_.getByChar(), true);

    case InputMode::LATIN:
        return reading_.getRawByChar();

    case InputMode::WIDE_LATIN:
        return util::convert_to_wide(reading_.getRawByChar());

    case InputMode::HIRAGANA:
    default:
        return reading_.getByChar();
    }
}

void Preedit::updatePreedit() {
    if (isConverting()) {
        conversion_.updatePreedit();
    } else {
        bool useClientPreedit = state_.supportClientPreedit();
        fcitx::Text preedit;

        std::string s = string();
        if (!s.empty()) {
            preedit.append(s, useClientPreedit
                                  ? fcitx::TextFormatFlag::Underline
                                  : fcitx::TextFormatFlag::NoFlag);
            preedit.setCursor(static_cast<int>(caretPos()));
        }

        if (useClientPreedit) {
            state_.inputContext()->inputPanel().setClientPreedit(preedit);
        } else {
            state_.inputContext()->inputPanel().setPreedit(preedit);
        }
    }
}

bool Preedit::isPreediting() {
    return reading_.length() > 0 || conversion_.isConverting() ||
           !source_.empty();
}

bool Preedit::isConverting() { return conversion_.isConverting(); }

bool Preedit::isPredicting() { return conversion_.isPredicting(); }

bool Preedit::isReconverting() { return !source_.empty(); }

/*
 * manipulating the preedit string
 */
bool Preedit::canProcessKeyEvent(const fcitx::KeyEvent &key) {
    return reading_.canProcesKeyEvent(key);
}

bool Preedit::processKeyEvent(const fcitx::KeyEvent &key) {
    if (!reading_.canProcesKeyEvent(key)) {
        return false;
    }

    bool retval = reading_.processKeyEvent(key);

    if (inputMode_ == InputMode::LATIN || inputMode_ == InputMode::WIDE_LATIN) {
        return true;
    }
    // auto convert
    unsigned int len = reading_.length();
    if (len > 0) {
        std::string str = reading_.getRawByChar(len - 1, 1);
        if (isCommaOrPeriod(str)) {
            if (*state_.config().general->periodBehavior ==
                    PeriodBehavior::Convert &&
                length() > 1) {
                convert();
            } else if (*state_.config().general->periodBehavior ==
                       PeriodBehavior::Commit) {
                return true;
            }
        }
    }

    return retval;
}

bool Preedit::append(const fcitx::KeyEvent &key, const std::string &string) {
    return reading_.append(key, string);
}
bool Preedit::append(const fcitx::Key &key, const std::string &string) {
    fcitx::KeyEvent event(state_.inputContext(), key);
    return reading_.append(event, string);
}

void Preedit::erase(bool backward) {
    if (reading_.utf8Length() <= 0) {
        return;
    }

    // cancel conversion
    revert();

    // erase
    TypingMethod method = typingMethod();
    bool allow_split = method == TypingMethod::ROMAJI &&
                       *state_.config().general->romajiAllowSplit;
    if (backward && reading_.caretPosByChar() == 0) {
        return;
    }
    if (!backward && reading_.caretPosByChar() >= reading_.utf8Length()) {
        return;
    }
    if (backward) {
        reading_.moveCaret(-1, allow_split);
    }
    reading_.erase(reading_.caretPosByChar(), 1, allow_split);
}

void Preedit::finish() { reading_.finish(); }

/*
 * manipulating conversion string
 */
void Preedit::convert(CandidateType type, bool single_segment) {
    if (source_.empty()) {
        conversion_.convert(type, single_segment);
    } else {
        conversion_.convert(source_, single_segment);
    }
}

void Preedit::convert(const std::string &source, bool single_segment) {
    conversion_.convert(source, single_segment);
    source_ = source;
}

void Preedit::revert() { conversion_.clear(); }

void Preedit::commit(int segment_id, bool learn) {
    if (conversion_.isConverting()) {
        conversion_.commit(segment_id, learn);
    }
    if (!conversion_.isConverting()) {
        clear();
    }
}

int Preedit::nrSegments() { return conversion_.nrSegments(); }

std::string Preedit::segmentString(int segment_id) {
    return conversion_.segmentString(segment_id);
}

int Preedit::selectedSegment() { return conversion_.selectedSegment(); }

void Preedit::selectSegment(int segment_id) {
    conversion_.selectSegment(segment_id);
}

int Preedit::segmentSize(int segment_id) {
    return conversion_.segmentSize(segment_id);
}

void Preedit::resizeSegment(int relative_size, int segment_id) {
    conversion_.resizeSegment(relative_size, segment_id);
}

/*
 * candidates for a segment
 */
std::unique_ptr<fcitx::CommonCandidateList>
Preedit::candidates(int segment_id) {
    return conversion_.candidates(segment_id);
}

int Preedit::selectedCandidate(int segment_id) {
    return conversion_.selectedCandidate(segment_id);
}

void Preedit::selectCandidate(int candidate_id, int segment_id) {
    conversion_.selectCandidate(candidate_id, segment_id);
}

/*
 * manipulating the caret
 */
unsigned int Preedit::caretPos() {
    if (isConverting()) {
        return conversion_.segmentPosition();
    }
    if (inputMode() == InputMode::HALF_KATAKANA) {
        // FIXME! It's ad-hoc
        std::string substr;
        substr = reading_.getByChar(0, reading_.caretPosByChar(),
                                    FCITX_ANTHY_STRING_HALF_KATAKANA);
        return substr.length();
    }
    return reading_.caretPos();
}

void Preedit::setCaretPosByChar(unsigned int pos) {
    if (isConverting()) {
        return;
    }

    reading_.setCaretPosByChar(pos);
}

void Preedit::moveCaret(int step) {
    if (isConverting()) {
        return;
    }

    TypingMethod method = typingMethod();
    bool allow_split = method == TypingMethod::ROMAJI &&
                       *state_.config().general->romajiAllowSplit;

    reading_.moveCaret(step, allow_split);
}

void Preedit::predict() { conversion_.predict(); }

/*
 * clear all string
 */
void Preedit::clear(int segment_id) {
    // FIXME! We should add implementation not only for conversion string but
    // also for reading string.

    if (!isConverting()) {
        reading_.clear();
        conversion_.clear();
        source_ = std::string();
        return;
    }

    conversion_.clear(segment_id);
    if (conversion_.nrSegments() <= 0) {
        reading_.clear();
        source_ = std::string();
    }
}

/*
 * preference
 */
void Preedit::setInputMode(InputMode mode) { inputMode_ = mode; }

InputMode Preedit::inputMode() const { return inputMode_; }

void Preedit::setTypingMethod(TypingMethod method) {
    reading_.setTypingMethod(method);
}

TypingMethod Preedit::typingMethod() const { return reading_.typingMethod(); }

void Preedit::setPeriodStyle(PeriodStyle style) {
    reading_.setPeriodStyle(style);
}

PeriodStyle Preedit::periodStyle() const { return reading_.periodStyle(); }

void Preedit::setCommaStyle(CommaStyle style) { reading_.setCommaStyle(style); }

CommaStyle Preedit::commaStyle() const { return reading_.commaStyle(); }

void Preedit::setBracketStyle(BracketStyle style) {
    reading_.setBracketStyle(style);
}

BracketStyle Preedit::bracketStyle() const { return reading_.bracketStyle(); }

void Preedit::setSlashStyle(SlashStyle style) { reading_.setSlashStyle(style); }

SlashStyle Preedit::slashStyle() const { return reading_.slashStyle(); }

void Preedit::setSymbolHalf(bool half) { reading_.setSymbolHalf(half); }

void Preedit::setNumberHalf(bool half) { reading_.setNumberHalf(half); }

void Preedit::setPseudoAsciiMode(int mode) {
    reading_.setPseudoAsciiMode(mode);
}

bool Preedit::isPseudoAsciiMode() { return reading_.isPseudoAsciiMode(); }

void Preedit::resetPseudoAsciiMode() { reading_.resetPseudoAsciiMode(); }

bool Preedit::isCommaOrPeriod(const std::string &str) const {
    TypingMethod typing = typingMethod();
    PeriodStyle period = periodStyle();
    CommaStyle comma = commaStyle();

    const auto *period_rule = get_period_rule(typing, period);
    const auto *comma_rule = get_comma_rule(typing, comma);

    if (period_rule) {
        for (const auto &item : *period_rule) {
            if (item.string == str) {
                return true;
            }
        }
    }
    if (comma_rule) {
        for (const auto &item : *comma_rule) {
            if (item.string == str) {
                return true;
            }
        }
    }

    return false;
}

/*
 * utilities
 */
static const std::array<ConvRule, 1> *get_period_rule(TypingMethod method,
                                                      PeriodStyle period) {
    switch (method) {
    case TypingMethod::KANA:
        switch (period) {
        case PeriodStyle::WIDE:
            return &fcitx_anthy_kana_wide_period_rule;
        case PeriodStyle::HALF:
            return &fcitx_anthy_kana_half_period_rule;
        case PeriodStyle::JAPANESE:
        default:
            return &fcitx_anthy_kana_ja_period_rule;
        };
        break;

    case TypingMethod::ROMAJI:
    default:
        switch (period) {
        case PeriodStyle::WIDE:
            return &fcitx_anthy_romaji_wide_period_rule;
        case PeriodStyle::HALF:
            return &fcitx_anthy_romaji_half_period_rule;
        case PeriodStyle::JAPANESE:
        default:
            return &fcitx_anthy_romaji_ja_period_rule;
        };
        break;
    };

    return nullptr;
}

static const std::array<ConvRule, 1> *get_comma_rule(TypingMethod method,
                                                     CommaStyle period) {
    switch (method) {
    case TypingMethod::KANA:
        switch (period) {
        case CommaStyle::WIDE:
            return &fcitx_anthy_kana_wide_comma_rule;
        case CommaStyle::HALF:
            return &fcitx_anthy_kana_half_comma_rule;
        case CommaStyle::JAPANESE:
        default:
            return &fcitx_anthy_kana_ja_comma_rule;
        };
        break;

    case TypingMethod::ROMAJI:
    default:
        switch (period) {
        case CommaStyle::WIDE:
            return &fcitx_anthy_romaji_wide_comma_rule;
        case CommaStyle::HALF:
            return &fcitx_anthy_romaji_half_comma_rule;
        case CommaStyle::JAPANESE:
        default:
            return &fcitx_anthy_romaji_ja_comma_rule;
        };
        break;
    };

    return nullptr;
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
