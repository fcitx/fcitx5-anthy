/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  SPDX-FileCopyrightText: 2005 Takuro Ashie
 *  SPDX-FileCopyrightText: 2012 CSSlayer <wengxt@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "reading.h"
#include "engine.h"
#include "state.h"
#include "utils.h"
#include <fcitx-utils/utf8.h>

ReadingSegment::ReadingSegment() {}

ReadingSegment::~ReadingSegment() {}

static const char *find_romaji(std::string c) {
    ConvRule *table = fcitx_anthy_romaji_typing_rule;

    for (unsigned int i = 0; table[i].string; i++) {
        std::string kana = table[i].result;
        if (c == kana)
            return table[i].string;
    }

    return "";
}

static void to_half(std::string &dest, std::string &src) {
    WideRule *table = fcitx_anthy_wide_table;

    for (unsigned int i = 0; i < fcitx::utf8::length(src); i++) {
        bool found = false;
        std::string kana1 = util::utf8_string_substr(src, i, 1);
        for (unsigned int i = 0; table[i].code; i++) {
            std::string kana2 = table[i].wide;
            if (kana1 == kana2) {
                dest += table[i].code;
                found = true;
                break;
            }
        }
        if (!found)
            dest += kana1;
    }
}

// Only a romaji string can be splited with raw key string.
// Other typing method aren't supported splitting raw key string.
void ReadingSegment::split(ReadingSegments &segments) {
    if (fcitx::utf8::length(kana) <= 1)
        segments.push_back(*this);

    std::string half;
    to_half(half, kana);
    bool same_with_raw = half == raw;

    std::string::iterator it;
    for (unsigned int i = 0; i < fcitx::utf8::length(kana); i++) {
        std::string c = util::utf8_string_substr(kana, i, 1);
        ReadingSegment seg;
        seg.kana = c;
        if (same_with_raw)
            to_half(seg.raw, c);
        else
            seg.raw = find_romaji(c);
        segments.push_back(seg);
    }
}

Reading::Reading(AnthyState &anthy)
    : state_(anthy), key2kanaNormal_(anthy, key2kanaTables_), kana_(anthy),
      nicola_(anthy, nicolaTables_), key2kana_(&key2kanaNormal_),
      segmentPos_(0), caretOffset_(0) {
    nicolaTables_.setTypingMethod(TypingMethod::NICOLA);
}

Reading::~Reading() {}

bool Reading::canProcesKeyEvent(const fcitx::KeyEvent &key) {
    if (kana_.canAppend(key))
        return true;

    return key2kana_->canAppend(key);
}

bool Reading::processKeyEvent(const fcitx::KeyEvent &key) {
    if (!canProcesKeyEvent(key))
        return false;

    if (caretOffset_ != 0) {
        splitSegment(segmentPos_);
        resetPending();
    }

    bool was_pending;
    if (kana_.canAppend(key))
        was_pending = kana_.isPending();
    else
        was_pending = key2kana_->isPending();

    std::string raw;
    std::string result, pending;
    bool need_commiting;
    if (kana_.canAppend(key))
        need_commiting = kana_.append(key, result, pending, raw);
    else
        need_commiting = key2kana_->append(key, result, pending, raw);

    ReadingSegments::iterator begin = segments_.begin();

    // fix previous segment and prepare next segment if needed
    if (!result.empty() || !pending.empty()) {
        if (!was_pending || // previous segment was already fixed
            need_commiting) // previous segment has been fixed
        {
            ReadingSegment c;
            segments_.insert(begin + segmentPos_, c);
            segmentPos_++;
        }
    }

    // fill segment
    if (!result.empty() && !pending.empty()) {
        segments_[segmentPos_ - 1].kana = result;

        ReadingSegment c;
        c.raw += raw;
        c.kana = pending;
        segments_.insert(begin + segmentPos_, c);
        segmentPos_++;

    } else if (!result.empty()) {
        segments_[segmentPos_ - 1].raw += raw;
        segments_[segmentPos_ - 1].kana = result;

    } else if (!pending.empty()) {
        segments_[segmentPos_ - 1].raw += raw;
        segments_[segmentPos_ - 1].kana = pending;

    } else {
    }

    return false;
}

void Reading::finish() {
    if (!key2kana_->isPending())
        return;

    std::string result = key2kana_->flushPending();
    if (!result.empty()) {
        segments_[segmentPos_ - 1].kana = result;
    }
}

void Reading::clear() {
    key2kanaNormal_.clear();
    kana_.clear();
    nicola_.clear();
    segments_.clear();
    segmentPos_ = 0;
    caretOffset_ = 0;
}

std::string Reading::getByChar(unsigned int start, int len, StringType type) {
    std::string str;
    unsigned int pos = 0, end = len > 0 ? start + len : utf8Length() - start;
    std::string kana;
    std::string raw;

    if (start >= end)
        return str;
    if (start >= utf8Length())
        return str;

    switch (type) {
    case FCITX_ANTHY_STRING_LATIN:
        raw = getRawByChar(start, len);
        str = raw;
        return str;

    case FCITX_ANTHY_STRING_WIDE_LATIN:
        raw = getRawByChar(start, len);
        str = util::convert_to_wide(raw);
        return str;

    default:
        break;
    }

    for (unsigned int i = 0; i < segments_.size(); i++) {
        if (pos >= start ||
            pos + fcitx::utf8::length(segments_[i].kana) > start) {
            unsigned int startstart = 0, len;

            if (pos >= start)
                startstart = 0;
            else
                startstart = pos - start;

            if (pos + fcitx::utf8::length(segments_[i].kana) > end)
                len = end - start;
            else
                len = fcitx::utf8::length(segments_[i].kana);

            kana +=
                util::utf8_string_substr(segments_[i].kana, startstart, len);
        }

        pos += fcitx::utf8::length(segments_[i].kana);
        if (pos >= end)
            break;
    }

    switch (type) {
    case FCITX_ANTHY_STRING_HIRAGANA:
        str = kana;
        break;

    case FCITX_ANTHY_STRING_KATAKANA:
        str = util::convert_to_katakana(kana);
        break;

    case FCITX_ANTHY_STRING_HALF_KATAKANA:
        str = util::convert_to_katakana(kana, true);
        break;

    default:
        break;
    }

    return str;
}

std::string Reading::getRawByChar(unsigned int start, int len) {
    std::string str;
    unsigned int pos = 0, end = len > 0 ? start + len : utf8Length() - start;

    if (start >= end)
        return str;

    for (unsigned int i = 0; i < segments_.size(); i++) {
        if (pos >= start ||
            pos + fcitx::utf8::length(segments_[i].kana) > start) {
            // FIXME!
            str += segments_[i].raw;
        }

        pos += fcitx::utf8::length(segments_[i].kana);

        if (pos >= end)
            break;
    }

    return str;
}

void Reading::splitSegment(unsigned int seg_id) {
    if (seg_id >= segments_.size())
        return;

    unsigned int pos = 0;
    for (unsigned int i = 0; i < seg_id && i < segments_.size(); i++)
        pos += segments_[i].kana.length();

    unsigned int caret = caretPos();
    unsigned int seg_len = segments_[seg_id].kana.length();
    bool caret_was_in_the_segment = false;
    if (caret > pos && caret < pos + seg_len)
        caret_was_in_the_segment = true;

    ReadingSegments segments;
    segments_[seg_id].split(segments);
    segments_.erase(segments_.begin() + seg_id);
    for (int j = segments.size() - 1; j >= 0; j--) {
        segments_.insert(segments_.begin() + seg_id, segments[j]);
        if (segmentPos_ > seg_id)
            segmentPos_++;
    }

    if (caret_was_in_the_segment) {
        segmentPos_ += caretOffset_;
        caretOffset_ = 0;
    }
}

bool Reading::append(const fcitx::KeyEvent &key, const std::string &string) {
    bool was_pending;
    std::string result, pending;
    bool need_commiting;

    if (!kana_.canAppend(key, true) && !key2kana_->canAppend(key, true))
        return false;

    if (caretOffset_ != 0) {
        splitSegment(segmentPos_);
        resetPending();
    }

    if (kana_.canAppend(key))
        was_pending = kana_.isPending();
    else
        was_pending = key2kana_->isPending();

    if (kana_.canAppend(key))
        need_commiting = kana_.append(string, result, pending);
    else
        need_commiting = key2kana_->append(string, result, pending);

    ReadingSegments::iterator begin = segments_.begin();

    // fix previous segment and prepare next segment if needed
    if (!result.empty() || !pending.empty()) {
        if (!was_pending || // previous segment was already fixed
            need_commiting) // previous segment has been fixed
        {
            ReadingSegment c;
            segments_.insert(begin + segmentPos_, c);
            segmentPos_++;
        }
    }

    // fill segment
    if (!result.empty() && !pending.empty()) {
        segments_[segmentPos_ - 1].kana = result;

        ReadingSegment c;
        c.raw += string;
        c.kana = pending;
        segments_.insert(begin + segmentPos_, c);
        segmentPos_++;

    } else if (!result.empty()) {
        segments_[segmentPos_ - 1].raw += string;
        segments_[segmentPos_ - 1].kana = result;

    } else if (!pending.empty()) {
        segments_[segmentPos_ - 1].raw += string;
        segments_[segmentPos_ - 1].kana = pending;

    } else {
    }

    return false;
}

void Reading::erase(unsigned int start, int len, bool allow_split) {
    if (segments_.size() <= 0)
        return;

    if (utf8Length() < start)
        return;

    if (len < 0)
        len = utf8Length() - start;

    // erase
    unsigned int pos = 0;
    for (int i = 0; i <= (int)segments_.size(); i++) {
        if (pos < start) {
            // we have not yet reached start position.

            if (i == (int)segments_.size())
                break;

            pos += fcitx::utf8::length(segments_[i].kana);

        } else if (pos == start) {
            // we have reached start position.

            if (i == (int)segments_.size())
                break;

            if (allow_split &&
                pos + fcitx::utf8::length(segments_[i].kana) > start + len) {
                // we have overshooted the end position!
                // we have to split this segment
                splitSegment(i);

            } else {
                // This segment is completely in the rage, erase it!
                len -= fcitx::utf8::length(segments_[i].kana);
                segments_.erase(segments_.begin() + i);
                if ((int)segmentPos_ > i)
                    segmentPos_--;
            }

            // retry from the same position
            i--;

        } else {
            // we have overshooted the start position!

            if (allow_split) {
                pos -= fcitx::utf8::length(segments_[i - 1].kana);
                splitSegment(i - 1);

                // retry from the previous position
                i -= 2;

            } else {
                // we have overshooted the start position, but have not been
                // allowed to split the segment.
                // So remove all string of previous segment.
                len -= pos - start;
                pos -= fcitx::utf8::length(segments_[i - 1].kana);
                segments_.erase(segments_.begin() + i - 1);
                if ((int)segmentPos_ > i - 1)
                    segmentPos_--;

                // retry from the previous position
                i -= 2;
            }
        }

        // Now all letters in the range are removed.
        // Exit the loop.
        if (len <= 0)
            break;
    }

    // reset values
    if (segments_.size() <= 0) {
        clear();
    } else {
        resetPending();
    }
}

void Reading::resetPending() {
    if (key2kana_->isPending())
        key2kana_->clear();
    if (kana_.isPending())
        kana_.clear();

    if (segmentPos_ <= 0)
        return;

    key2kana_->resetPending(segments_[segmentPos_ - 1].kana,
                            segments_[segmentPos_ - 1].raw);
    kana_.resetPending(segments_[segmentPos_ - 1].kana,
                       segments_[segmentPos_ - 1].raw);

    // FIXME! this code breaks pending state on normal input mode.
    key2kana_->resetPseudoAsciiMode();
    for (unsigned int i = 0; i < segmentPos_; i++) {
        key2kana_->processPseudoAsciiMode(segments_[i].kana);
    }
}

unsigned int Reading::length() {
    unsigned int len = 0;
    for (unsigned int i = 0; i < segments_.size(); i++)
        len += segments_[i].kana.length();
    return len;
}

unsigned int Reading::utf8Length() {
    unsigned int len = 0;
    for (unsigned int i = 0; i < segments_.size(); i++)
        len += fcitx::utf8::length(segments_[i].kana);
    return len;
}

unsigned int Reading::caretPosByChar() {
    unsigned int pos = 0;

    unsigned int i;
    for (i = 0; i < segmentPos_ && i < segments_.size(); i++) {
        pos += fcitx::utf8::length(segments_[i].kana);
    }

    pos += caretOffset_;

    return pos;
}

unsigned int Reading::caretPos() {
    unsigned int pos = 0;

    unsigned int i;
    for (i = 0; i < segmentPos_ && i < segments_.size(); i++) {
        pos += segments_[i].kana.length();
    }

    if (i < segments_.size() && caretOffset_) {
        auto iter = segments_[i].kana.begin();
        pos += fcitx::utf8::ncharByteLength(iter, caretOffset_);
    }

    return pos;
}

// FIXME! add "allow_split" argument.
void Reading::setCaretPosByChar(unsigned int pos) {
    if (pos == caretPosByChar())
        return;

    key2kana_->clear();
    kana_.clear();

    if (pos >= utf8Length()) {
        segmentPos_ = segments_.size();

    } else if (pos == 0 || segments_.size() <= 0) {
        segmentPos_ = 0;

    } else {
        unsigned int i, tmp_pos = 0;

        for (i = 0; tmp_pos <= pos; i++)
            tmp_pos += fcitx::utf8::length(segments_[i].kana);

        if (tmp_pos == pos) {
            segmentPos_ = i + 1;
        } else if (tmp_pos < caretPosByChar()) {
            segmentPos_ = i;
        } else if (tmp_pos > caretPosByChar()) {
            segmentPos_ = i + 1;
        }
    }

    resetPending();
}

void Reading::moveCaret(int step, bool allow_split) {
    if (step == 0)
        return;

    key2kana_->clear();
    kana_.clear();

    if (allow_split) {
        unsigned int pos = caretPosByChar();
        if (step < 0 && pos < static_cast<unsigned int>(abs(step))) {
            // lower limit
            segmentPos_ = 0;

        } else if (step > 0 && pos + step > utf8Length()) {
            // upper limit
            segmentPos_ = segments_.size();

        } else {
            unsigned int new_pos = pos + step;
            ReadingSegments::iterator it;
            pos = 0;
            segmentPos_ = 0;
            caretOffset_ = 0;
            for (it = segments_.begin(); pos < new_pos; it++) {
                if (pos + fcitx::utf8::length(it->kana) > new_pos) {
                    caretOffset_ = new_pos - pos;
                    break;
                } else {
                    segmentPos_++;
                    pos += fcitx::utf8::length(it->kana);
                }
            }
        }

    } else {
        if (step < 0 && segmentPos_ < static_cast<unsigned int>(abs(step))) {
            // lower limit
            segmentPos_ = 0;

        } else if (step > 0 && segmentPos_ + step > segments_.size()) {
            // upper limit
            segmentPos_ = segments_.size();

        } else {
            // other
            segmentPos_ += step;
        }
    }

    resetPending();
}

void Reading::setTypingMethod(TypingMethod method) {
    Key2KanaTable *fundamental_table = nullptr;

    if (method == TypingMethod::NICOLA) {
        fundamental_table = state_.engine()->customNicolaTable();
        key2kana_ = &nicola_;
        nicolaTables_.setTypingMethod(method, fundamental_table);
        nicola_.setCaseSensitive(true);
    } else if (method == TypingMethod::KANA) {
        fundamental_table = state_.engine()->customKanaTable();
        key2kana_ = &key2kanaNormal_;
        key2kanaTables_.setTypingMethod(method, fundamental_table);
        key2kanaNormal_.setCaseSensitive(true);
    } else {
        fundamental_table = state_.engine()->customRomajiTable();
        key2kana_ = &key2kanaNormal_;
        key2kanaTables_.setTypingMethod(method, fundamental_table);
        key2kanaNormal_.setCaseSensitive(false);
    }
}

TypingMethod Reading::typingMethod() {
    if (key2kana_ == &nicola_)
        return TypingMethod::NICOLA;
    else
        return key2kanaTables_.typingMethod();
}

void Reading::setPeriodStyle(PeriodStyle style) {
    key2kanaTables_.setPeriodStyle(style);
}

PeriodStyle Reading::periodStyle() { return key2kanaTables_.periodStyle(); }

void Reading::setCommaStyle(CommaStyle style) {
    key2kanaTables_.setCommaStyle(style);
}

CommaStyle Reading::commaStyle() { return key2kanaTables_.commaStyle(); }

void Reading::setBracketStyle(BracketStyle style) {
    key2kanaTables_.setBracketStyle(style);
}

BracketStyle Reading::bracketStyle() { return key2kanaTables_.bracketStyle(); }

void Reading::setSlashStyle(SlashStyle style) {
    key2kanaTables_.setSlashStyle(style);
}

SlashStyle Reading::slashStyle() { return key2kanaTables_.slashStyle(); }

void Reading::setSymbolHalf(bool half) { key2kanaTables_.setSymbolHalf(half); }

bool Reading::isSymbolHalf() { return key2kanaTables_.symbol_is_half(); }

void Reading::setNumberHalf(bool half) { key2kanaTables_.setNumberHalf(half); }

bool Reading::isNumberHalf() { return key2kanaTables_.isNumberHalf(); }

void Reading::setPseudoAsciiMode(int mode) {
    key2kanaNormal_.setPseudoAsciiMode(mode);
}

bool Reading::isPseudoAsciiMode() {
    return key2kanaNormal_.isPseudoAsciiMode();
}

void Reading::resetPseudoAsciiMode() {
    if (key2kanaNormal_.isPseudoAsciiMode() && key2kanaNormal_.isPending()) {
        ReadingSegment c;
        ReadingSegments::iterator it = segments_.begin();

        /* separate to another segment */
        key2kanaNormal_.resetPseudoAsciiMode();
        segments_.insert(it + segmentPos_, c);
        segmentPos_++;
    }
}
