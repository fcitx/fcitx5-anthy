/*
 * SPDX-FileCopyrightText: 2005 Takuro Ashie
 * SPDX-FileCopyrightText: 2017-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "conversion.h"
#include "state.h"
#include "utils.h"
#include <fcitx-utils/charutils.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/utf8.h>
#include <fcitx/inputpanel.h>

static void rotateCase(std::string &str);

//
// ConversionSegment Class
//
ConversionSegment::ConversionSegment(std::string str, int cand_id,
                                     unsigned int reading_len)
    : string_(str), candidateId_(cand_id), readingLen_(reading_len) {}

const std::string &ConversionSegment::string() const { return string_; }

int ConversionSegment::candidateId() const { return candidateId_; }

unsigned int ConversionSegment::readingLength() const { return readingLen_; }

void ConversionSegment::set(std::string str, int cand_id) {
    string_ = str;
    candidateId_ = cand_id;
}

void ConversionSegment::setReadingLength(unsigned int len) {
    readingLen_ = len;
}

//
// Conversion Class
//
Conversion::Conversion(AnthyState &anthy, Reading &reading)
    : state_(anthy), reading_(reading), anthyContext_(anthy_create_context()),
      startId_(0), curSegment_(-1), predicting_(false) {
    anthy_context_set_encoding(anthyContext_.get(), ANTHY_UTF8_ENCODING);
}

Conversion::~Conversion() = default;

//
// starting and finishing
//
void Conversion::convert(std::string source, CandidateType ctype,
                         bool single_segment) {
    if (isConverting())
        return;

    clear();

    std::string dest;

    // convert
    struct anthy_conv_stat conv_stat;
    anthy_get_stat(anthyContext_.get(), &conv_stat);
    if (conv_stat.nr_segment <= 0) {
        dest = source;
        anthy_set_string(anthyContext_.get(), dest.c_str());
    }

    if (single_segment)
        joingAllSegments();

    // get information about conversion string
    anthy_get_stat(anthyContext_.get(), &conv_stat);
    if (conv_stat.nr_segment <= 0)
        return;

    // select first segment
    curSegment_ = 0;

    // create segments
    segments_.clear();
    for (int i = startId_; i < conv_stat.nr_segment; i++) {
        struct anthy_segment_stat seg_stat;
        anthy_get_segment_stat(anthyContext_.get(), i, &seg_stat);
        segments_.push_back(ConversionSegment(segmentString(i, ctype), ctype,
                                              seg_stat.seg_len));
    }
}

void Conversion::convert(CandidateType ctype, bool single_segment) {
    convert(reading_.getByChar(), ctype, single_segment);
}

void Conversion::convert(const std::string &source, bool single_segment) {
    convert(source, FCITX_ANTHY_CANDIDATE_DEFAULT, single_segment);
}

void Conversion::predict() {
    clear();

#ifdef HAS_ANTHY_PREDICTION
    std::string str;

    str = reading_.getByChar();
    anthy_set_prediction_string(anthyContext_.get(), str.c_str());

    struct anthy_prediction_stat ps;
    anthy_get_prediction_stat(anthyContext_.get(), &ps);
    if (ps.nr_prediction > 0)
        predicting_ = true;
    else
        anthy_reset_context(anthyContext_.get());
#endif /* HAS_ANTHY_PREDICTION */
}

void Conversion::clear(int segment_id) {
    if (segment_id < 0 || segments_.size() <= 0 ||
        segment_id >= (int)segments_.size() - 1) {
        // complete clear

        anthy_reset_context(anthyContext_.get());

        segments_.clear();

        startId_ = 0;
        curSegment_ = -1;
        predicting_ = false;

    } else {
        // partial clear

        // remove stored segments
        ConversionSegments::iterator it = segments_.begin();
        segments_.erase(it, it + segment_id + 1);

        // adjust selected segment
        int new_start_segment_id = startId_ + segment_id + 1;
        if (curSegment_ >= 0) {
            curSegment_ -= new_start_segment_id - startId_;
            if (curSegment_ < 0)
                curSegment_ = 0;
        }

        // adjust offset
        unsigned int clear_len = 0;
        for (int i = startId_; i < new_start_segment_id; i++) {
            struct anthy_segment_stat seg_stat;
            anthy_get_segment_stat(anthyContext_.get(), i, &seg_stat);
            clear_len += seg_stat.seg_len;
        }
        reading_.erase(0, clear_len, true);
        startId_ = new_start_segment_id;
    }
}

void Conversion::commit(int segment_id, bool learn) {
    if (!isConverting())
        return;

    // learn
    for (unsigned int i = startId_; learn && i < segments_.size() &&
                                    (segment_id < 0 || (int)i <= segment_id);
         i++) {
        if (segments_[i].candidateId() >= 0)
            anthy_commit_segment(anthyContext_.get(), i,
                                 segments_[i].candidateId());
    }

    clear(segment_id);
}

//
// getting status
//
bool Conversion::isConverting() const { return segments_.size() > 0; }

bool Conversion::isPredicting() const { return predicting_; }

std::string Conversion::get() const {
    std::string str;
    for (auto it = segments_.begin(); it != segments_.end(); it++)
        str += it->string();
    return str;
}

unsigned int Conversion::length() const {
    unsigned int len = 0;
    for (auto it = segments_.begin(); it != segments_.end(); it++)
        len += it->string().length();
    return len;
}

unsigned int Conversion::utf8Length() const {
    unsigned int len = 0;
    for (auto it = segments_.begin(); it != segments_.end(); it++)
        len += fcitx::utf8::length(it->string());
    return len;
}

void Conversion::updatePreedit() {
    fcitx::Text preedit;
    unsigned int seg_id;
    ConversionSegments::iterator it;
    for (it = segments_.begin(), seg_id = 0; it != segments_.end();
         it++, seg_id++) {
        // create attribute for this segment
        if (it->string().length() <= 0) {
            continue;
        }

        preedit.append(it->string(), static_cast<int>(seg_id) == curSegment_
                                         ? fcitx::TextFormatFlag::HighLight
                                         : fcitx::TextFormatFlag::NoFlag);
    }
    preedit.setCursor(static_cast<int>(segmentPosition()));

    if (state_.supportClientPreedit()) {
        state_.inputContext()->inputPanel().setClientPreedit(preedit);
    } else {
        state_.inputContext()->inputPanel().setPreedit(preedit);
    }
}

//
// segments of the converted sentence
//
int Conversion::nrSegments() {
    if (!isConverting())
        return 0;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat(anthyContext_.get(), &conv_stat);

    return conv_stat.nr_segment - startId_;
}

std::string Conversion::segmentString(int segment_id, int candidate_id) {
    if (segment_id < 0) {
        if (curSegment_ < 0)
            return std::string();
        else
            segment_id = curSegment_;
    }

    struct anthy_conv_stat conv_stat;
    anthy_get_stat(anthyContext_.get(), &conv_stat);

    if (conv_stat.nr_segment <= 0)
        return std::string();

    if (startId_ < 0 || startId_ >= conv_stat.nr_segment) {
        return std::string(); // error
    }

    if (segment_id < 0 || segment_id + startId_ >= conv_stat.nr_segment) {
        return std::string(); // error
    }

    // character position of the head of segment.
    unsigned int real_seg_start = 0;
    for (int i = startId_; i < startId_ + segment_id; i++) {
        struct anthy_segment_stat seg_stat;
        anthy_get_segment_stat(anthyContext_.get(), i, &seg_stat);
        real_seg_start += seg_stat.seg_len;
    }

    int real_seg = segment_id + startId_;
    int cand;
    if (candidate_id <= FCITX_ANTHY_LAST_SPECIAL_CANDIDATE)
        cand = segments_[segment_id].candidateId();
    else
        cand = candidate_id;

    // get information of this segment
    struct anthy_segment_stat seg_stat;
    anthy_get_segment_stat(anthyContext_.get(), real_seg, &seg_stat);

    // get string of this segment
    std::string segment_str;
    if (cand < 0) {
        segment_str =
            readingSubstr(segment_id, cand, real_seg_start, seg_stat.seg_len);
    } else {
        int len =
            anthy_get_segment(anthyContext_.get(), real_seg, cand, nullptr, 0);
        if (len > 0) {
            char buf[len + 1];
            anthy_get_segment(anthyContext_.get(), real_seg, cand, buf,
                              len + 1);
            buf[len] = '\0';
            segment_str = buf;
        }
    }

    return segment_str;
}

int Conversion::selectedSegment() { return curSegment_; }

void Conversion::selectSegment(int segment_id) {
    if (!isConverting())
        return;

    if (segment_id < 0) {
        curSegment_ = -1;
        state_.resetCursor(0);
        return;
    }

    struct anthy_conv_stat conv_stat;
    anthy_get_stat(anthyContext_.get(), &conv_stat);

    int real_segment_id = segment_id + startId_;

    if (segment_id >= 0 && real_segment_id < conv_stat.nr_segment) {
        if (curSegment_ != segment_id) {
            if (static_cast<size_t>(segment_id) < segments_.size()) {
                state_.resetCursor(segments_[segment_id].candidateId());
            }
            curSegment_ = segment_id;
        }
    }
}

int Conversion::segmentSize(int segment_id) {
    if (!isConverting())
        return -1;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat(anthyContext_.get(), &conv_stat);

    if (segment_id < 0) {
        if (curSegment_ < 0)
            return -1;
        else
            segment_id = curSegment_;
    }
    int real_segment_id = segment_id + startId_;

    if (real_segment_id >= conv_stat.nr_segment)
        return -1;

    struct anthy_segment_stat seg_stat;
    anthy_get_segment_stat(anthyContext_.get(), real_segment_id, &seg_stat);

    return seg_stat.seg_len;
}

void Conversion::resizeSegment(int relative_size, int segment_id) {
    if (isPredicting())
        return;
    if (!isConverting())
        return;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat(anthyContext_.get(), &conv_stat);

    int real_segment_id;

    if (segment_id < 0) {
        if (curSegment_ < 0)
            return;
        else
            segment_id = curSegment_;
        real_segment_id = segment_id + startId_;
    } else {
        real_segment_id = segment_id + startId_;
        if (curSegment_ > segment_id)
            curSegment_ = segment_id;
    }

    if (real_segment_id >= conv_stat.nr_segment)
        return;

    // do resize
    anthy_resize_segment(anthyContext_.get(), real_segment_id, relative_size);

    // reset candidates of trailing segments
    anthy_get_stat(anthyContext_.get(), &conv_stat);
    ConversionSegments::iterator start_iter = segments_.begin();
    ConversionSegments::iterator end_iter = segments_.end();
    segments_.erase(start_iter + segment_id, end_iter);
    for (int i = real_segment_id; i < conv_stat.nr_segment; i++) {
        struct anthy_segment_stat seg_stat;
        anthy_get_segment_stat(anthyContext_.get(), i, &seg_stat);
        segments_.push_back(ConversionSegment(segmentString(i - startId_, 0), 0,
                                              seg_stat.seg_len));
    }
}

unsigned int Conversion::segmentPosition(int segment_id) {
    if (segment_id < 0) {
        if (curSegment_ < 0)
            return length();
        else
            segment_id = curSegment_;
    }

    unsigned int pos = 0;

    for (unsigned int i = 0; (int)i < curSegment_ && i < segments_.size();
         i++) {
        pos += segments_[i].string().length();
    }

    return pos;
}

class AnthyCandidate : public fcitx::CandidateWord {
public:
    AnthyCandidate(AnthyState *anthy, std::string str, int idx)
        : anthy_(anthy), idx_(idx) {
        setText(fcitx::Text(std::move(str)));
    }

    void select(fcitx::InputContext *) const override {
        anthy_->actionSelectCandidate(idx_);
        anthy_->updateUI();
    }

private:
    AnthyState *anthy_;
    int idx_;
};

class AnthyCandidateList : public fcitx::CommonCandidateList {
public:
    AnthyCandidateList(AnthyState &anthy) : anthy_(anthy) {}

    void prev() override {
        fcitx::CommonCandidateList::prev();
        syncCursor();
    }
    void next() override {
        fcitx::CommonCandidateList::next();
        syncCursor();
    }
    void prevCandidate() override {
        fcitx::CommonCandidateList::prevCandidate();
        syncCursor();
    }
    void nextCandidate() override {
        fcitx::CommonCandidateList::nextCandidate();
        syncCursor();
    }

private:
    void syncCursor() {
        if (int index = globalCursorIndex(); index >= 0) {
            anthy_.selectCandidateNoDirect(index);
        }
    }
    AnthyState &anthy_;
};

//
// candidates for a segment or prediction
//
std::unique_ptr<fcitx::CommonCandidateList>
Conversion::candidates(int segment_id) {
    auto table = std::make_unique<AnthyCandidateList>(state_);
    table->setLayoutHint(*state_.config().general->candidateLayout);
    auto pageSize = *state_.config().general->pageSize;
    table->setPageSize(*state_.config().general->pageSize);
    table->setCursorPositionAfterPaging(
        fcitx::CursorPositionAfterPaging::SameAsLast);
    int selected = selectedCandidate();

    if (isPredicting()) {
#ifdef HAS_ANTHY_PREDICTION
        std::string str;
        struct anthy_prediction_stat ps;

        anthy_get_prediction_stat(anthyContext_.get(), &ps);

        for (int i = 0; i < ps.nr_prediction; i++) {
            int len = anthy_get_prediction(anthyContext_.get(), i, nullptr, 0);
            if (len <= 0)
                continue;

            std::vector<char> buf;
            buf.resize(len + 1);
            anthy_get_prediction(anthyContext_.get(), i, buf.data(), len + 1);
            buf[len] = '\0';
            table->append<AnthyCandidate>(&state_, buf.data(), i);
        }
#endif /* HAS_ANTHY_PREDICTION */
    } else if (isConverting()) {
        struct anthy_conv_stat conv_stat;
        anthy_get_stat(anthyContext_.get(), &conv_stat);

        if (conv_stat.nr_segment <= 0)
            return nullptr;

        if (segment_id < 0) {
            if (curSegment_ < 0)
                return nullptr;
            else
                segment_id = curSegment_;
        }
        int real_segment_id = segment_id + startId_;

        if (real_segment_id >= conv_stat.nr_segment)
            return nullptr;

        struct anthy_segment_stat seg_stat;
        anthy_get_segment_stat(anthyContext_.get(), real_segment_id, &seg_stat);

        for (int i = 0; i < seg_stat.nr_candidate; i++) {
            int len = anthy_get_segment(anthyContext_.get(), real_segment_id, i,
                                        nullptr, 0);
            if (len <= 0)
                continue;

            std::vector<char> buf;
            buf.resize(len + 1);
            anthy_get_segment(anthyContext_.get(), real_segment_id, i,
                              buf.data(), len + 1);
            buf[len] = '\0';

            table->append<AnthyCandidate>(&state_, buf.data(), i);
        }
    }
    if (selected >= 0 && selected < table->totalSize()) {
        table->setGlobalCursorIndex(selected);
        table->setPage(selected / pageSize);
    }
    table->setSelectionKey(util::selection_keys());

    return table;
}

int Conversion::selectedCandidate(int segment_id) {
    if (isPredicting()) {
#ifdef HAS_ANTHY_PREDICTION
        struct anthy_prediction_stat ps;

        anthy_get_prediction_stat(anthyContext_.get(), &ps);

        if (ps.nr_prediction <= 0)
            return -1;

        if (segment_id < 0) {
            if (curSegment_ < 0)
                return -1;
            else
                segment_id = curSegment_;
        } else if (segment_id >= ps.nr_prediction) {
            return -1;
        }

        return segments_[segment_id].candidateId();
#endif /* HAS_ANTHY_PREDICTION */

    } else if (isConverting()) {
        struct anthy_conv_stat cs;

        anthy_get_stat(anthyContext_.get(), &cs);

        if (cs.nr_segment <= 0)
            return -1;

        if (segment_id < 0) {
            if (curSegment_ < 0)
                return -1;
            else
                segment_id = curSegment_;
        } else if (segment_id >= cs.nr_segment) {
            return -1;
        }

        return segments_[segment_id].candidateId();
    }

    return -1;
}

void Conversion::selectCandidate(int candidate_id, int segment_id) {
    if (isPredicting()) {
#ifdef HAS_ANTHY_PREDICTION
        if (candidate_id < FCITX_ANTHY_CANDIDATE_DEFAULT)
            return;

        struct anthy_prediction_stat ps;
        anthy_get_prediction_stat(anthyContext_.get(), &ps);

        if (ps.nr_prediction <= 0)
            return;

        if (!isConverting()) {
            curSegment_ = 0;
            segments_.push_back(
                ConversionSegment(predictionString(0), 0, reading_.length()));
        }

        if (candidate_id < ps.nr_prediction) {
            segments_[0].set(predictionString(candidate_id), candidate_id);
        }
#endif /* HAS_ANTHY_PREDICTION */

    } else if (isConverting()) {
        if (candidate_id <= FCITX_ANTHY_LAST_SPECIAL_CANDIDATE)
            return;

        struct anthy_conv_stat conv_stat;
        anthy_get_stat(anthyContext_.get(), &conv_stat);

        if (conv_stat.nr_segment <= 0)
            return;

        if (segment_id < 0) {
            if (curSegment_ < 0)
                return;
            else
                segment_id = curSegment_;
        }
        int real_segment_id = segment_id + startId_;

        if (segment_id >= conv_stat.nr_segment)
            return;

        struct anthy_segment_stat seg_stat;
        anthy_get_segment_stat(anthyContext_.get(), real_segment_id, &seg_stat);

        if (candidate_id == FCITX_ANTHY_CANDIDATE_HALF) {
            switch (segments_[segment_id].candidateId()) {
            case FCITX_ANTHY_CANDIDATE_LATIN:
            case FCITX_ANTHY_CANDIDATE_WIDE_LATIN:
                candidate_id = FCITX_ANTHY_CANDIDATE_LATIN;
                break;
            default:
                candidate_id = FCITX_ANTHY_CANDIDATE_HALF_KATAKANA;
                break;
            }
        }

        if (candidate_id < seg_stat.nr_candidate) {
            segments_[segment_id].set(segmentString(segment_id, candidate_id),
                                      candidate_id);
        }
    }
}

//
// Utilities
//
std::string Conversion::readingSubstr(int segment_id, int candidate_id,
                                      int seg_start, int seg_len) {
    int prev_cand = 0;
    std::string string;

    if (segment_id < (int)segments_.size())
        prev_cand = segments_[segment_id].candidateId();

    switch ((CandidateType)candidate_id) {
    case FCITX_ANTHY_CANDIDATE_LATIN:
        if (prev_cand == FCITX_ANTHY_CANDIDATE_LATIN) {
            std::string str = segments_[segment_id].string();
            rotateCase(str);
            string = str;
        } else {
            string = reading_.getByChar(seg_start, seg_len,
                                        FCITX_ANTHY_STRING_LATIN);
        }
        break;

    case FCITX_ANTHY_CANDIDATE_WIDE_LATIN:
        if (prev_cand == FCITX_ANTHY_CANDIDATE_WIDE_LATIN) {
            std::string str =
                util::convert_to_half(segments_[segment_id].string());
            rotateCase(str);
            string = util::convert_to_wide(str);
        } else {
            string = reading_.getByChar(seg_start, seg_len,
                                        FCITX_ANTHY_STRING_WIDE_LATIN);
        }
        break;

    case FCITX_ANTHY_CANDIDATE_KATAKANA:
        string =
            reading_.getByChar(seg_start, seg_len, FCITX_ANTHY_STRING_KATAKANA);
        break;

    case FCITX_ANTHY_CANDIDATE_HALF_KATAKANA:
        string = reading_.getByChar(seg_start, seg_len,
                                    FCITX_ANTHY_STRING_HALF_KATAKANA);
        break;

    case FCITX_ANTHY_CANDIDATE_HALF:
        // shouldn't reach to this entry
        string = reading_.getByChar(seg_start, seg_len,
                                    FCITX_ANTHY_STRING_HALF_KATAKANA);
        break;

    case FCITX_ANTHY_CANDIDATE_HIRAGANA:
    default:
        string =
            reading_.getByChar(seg_start, seg_len, FCITX_ANTHY_STRING_HIRAGANA);
        break;
    }
    return string;
}

std::string Conversion::predictionString(int candidate_id) {
#ifdef HAS_ANTHY_PREDICTION
    if (!isPredicting())
        return std::string();

    struct anthy_prediction_stat ps;
    anthy_get_prediction_stat(anthyContext_.get(), &ps);

    if (ps.nr_prediction <= 0)
        return std::string();

    int len =
        anthy_get_prediction(anthyContext_.get(), candidate_id, nullptr, 0);
    if (len <= 0)
        return std::string();

    char buf[len + 1];
    anthy_get_prediction(anthyContext_.get(), candidate_id, buf, len + 1);
    buf[len] = '\0';

    std::string cand = buf;

    return cand;
#else  /* HAS_ANTHY_PREDICTION */
    return std::string();
#endif /* HAS_ANTHY_PREDICTION */
}

void Conversion::joingAllSegments() {
    do {
        struct anthy_conv_stat conv_stat;
        anthy_get_stat(anthyContext_.get(), &conv_stat);
        int nr_seg = conv_stat.nr_segment - startId_;

        if (nr_seg > 1)
            anthy_resize_segment(anthyContext_.get(), startId_, 1);
        else
            break;
    } while (true);
}

static void rotateCase(std::string &str) {
    bool is_mixed = false;
    for (unsigned int i = 1; i < str.length(); i++) {
        if ((fcitx::charutils::isupper(str[0]) &&
             fcitx::charutils::islower(str[i])) ||
            (fcitx::charutils::islower(str[0]) &&
             fcitx::charutils::isupper(str[i]))) {
            is_mixed = true;
            break;
        }
    }

    if (is_mixed) {
        // Anthy -> anthy, anThy -> anthy
        for (unsigned int i = 0; i < str.length(); i++)
            str[i] = fcitx::charutils::tolower(str[i]);
    } else if (isupper(str[0])) {
        // ANTHY -> Anthy
        for (unsigned int i = 1; i < str.length(); i++)
            str[i] = fcitx::charutils::tolower(str[i]);
    } else {
        // anthy -> ANTHY
        for (unsigned int i = 0; i < str.length(); i++)
            str[i] = fcitx::charutils::toupper(str[i]);
    }
}
