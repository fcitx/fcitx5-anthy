/*
 * SPDX-FileCopyrightText: 2005 Takuro Ashie
 * SPDX-FileCopyrightText: 2012-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_ANTHY_CONVERSION_H_
#define _FCITX5_ANTHY_CONVERSION_H_

#include "reading.h"
#include <anthy/anthy.h>

class AnthyState;

enum CandidateType {
    FCITX_ANTHY_CANDIDATE_DEFAULT = 0,
    FCITX_ANTHY_CANDIDATE_LATIN = -1,
    FCITX_ANTHY_CANDIDATE_WIDE_LATIN = -2,
    FCITX_ANTHY_CANDIDATE_HIRAGANA = -3,
    FCITX_ANTHY_CANDIDATE_KATAKANA = -4,
    FCITX_ANTHY_CANDIDATE_HALF_KATAKANA = -5,
    FCITX_ANTHY_CANDIDATE_HALF = -6,
    FCITX_ANTHY_LAST_SPECIAL_CANDIDATE = -7,
};

class ConversionSegment final {
public:
    ConversionSegment(std::string str, int cand_id, unsigned int reading_len);
    FCITX_INLINE_DEFINE_DEFAULT_DTOR_AND_MOVE_WITHOUT_SPEC(ConversionSegment);

    const std::string &string() const;
    int candidateId() const;
    unsigned int readingLength() const;

    void set(std::string str, int cand_id);
    void setReadingLength(unsigned int len);

private:
    std::string string_;
    int candidateId_;
    unsigned int readingLen_;
};
typedef std::vector<ConversionSegment> ConversionSegments;

class Conversion {
public:
    Conversion(AnthyState &anthy, Reading &reading);
    virtual ~Conversion();

    // starting and finishing
    void convert(std::string source, CandidateType ctype, bool single_segment);
    void convert(CandidateType type = FCITX_ANTHY_CANDIDATE_DEFAULT,
                 bool single_segment = false);
    void convert(const std::string &source, bool single_segment = false);
    void predict();
    void clear(int segment_id = -1);
    void commit(int segment_id = -1, bool learn = true);

    // getting status
    bool isConverting() const;
    bool isPredicting() const;

    std::string get() const;
    unsigned int length() const;
    unsigned int utf8Length() const;
    void updatePreedit();

    // segments of the converted sentence
    int nrSegments();
    std::string
    segmentString(int segment_id = -1,
                  int candidate_id = FCITX_ANTHY_LAST_SPECIAL_CANDIDATE);
    int selectedSegment();
    void selectSegment(int segment_id);
    int segmentSize(int segment_id = -1);
    void resizeSegment(int relative_size, int segment_id = -1);
    unsigned int segmentPosition(int segment_id = -1);

    // candidates for a segment or prediction
    std::unique_ptr<fcitx::CommonCandidateList> candidates(int segment_id = -1);
    int selectedCandidate(int segment_id = -1);
    void selectCandidate(int candidate_id, int segment_id = -1);

private:
    std::string readingSubstr(int segment_id, int candidate_id, int seg_start,
                              int seg_len);
    std::string predictionString(int candidate_id);
    void joingAllSegments();

private:
    AnthyState &state_;

    // convertors
    Reading &reading_;
    fcitx::UniqueCPtr<anthy_context, anthy_release_context> anthyContext_;

    // status variables
    ConversionSegments segments_;
    int startId_;    // number of commited segments
    int curSegment_; // relative position from m_start_id
    bool predicting_;
};

#endif // _FCITX5_ANTHY_CONVERSION_H_
