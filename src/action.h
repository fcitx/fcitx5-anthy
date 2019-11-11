//
// Copyright (C) 2005 Takuro Ashie
// Copyright (C) 2012~2017 by CSSlayer
// wengxt@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _FCITX5_ANTHY_ACTION_H_
#define _FCITX5_ANTHY_ACTION_H_

#include <fcitx-config/option.h>
#include <fcitx/event.h>
#include <string>

#define ACTION_CONFIG_CIRCLE_INPUT_MODE_KEY "CircleInputModeKey"
#define ACTION_CONFIG_CIRCLE_KANA_MODE_KEY "CircleKanaModeKey"
#define ACTION_CONFIG_CIRCLE_LATIN_HIRAGANA_MODE_KEY                           \
  "CircleLatinHiraganaModeKey"
#define ACTION_CONFIG_CIRCLE_TYPING_METHOD_KEY "CircleTypingMethodKey"
#define ACTION_CONFIG_LATIN_MODE_KEY "LatinModeKey"
#define ACTION_CONFIG_WIDE_LATIN_MODE_KEY "WideLatinModeKey"
#define ACTION_CONFIG_HIRAGANA_MODE_KEY "HiraganaModeKey"
#define ACTION_CONFIG_KATAKANA_MODE_KEY "KatakanaModeKey"
#define ACTION_CONFIG_HALF_KATAKANA_MODE_KEY "HalfKatakanaModeKey"
#define ACTION_CONFIG_CANCEL_PSEUDO_ASCII_MODE_KEY "CancelPseudoAsciiModeKey"

#define ACTION_CONFIG_INSERT_SPACE_KEY "InsertSpaceKey"
#define ACTION_CONFIG_INSERT_ALT_SPACE_KEY "InsertAltSpaceKey"
#define ACTION_CONFIG_INSERT_HALF_SPACE_KEY "InsertHalfSpaceKey"
#define ACTION_CONFIG_INSERT_WIDE_SPACE_KEY "InsertWideSpaceKey"
#define ACTION_CONFIG_BACKSPACE_KEY "BackSpaceKey"
#define ACTION_CONFIG_DELETE_KEY "DeleteKey"
#define ACTION_CONFIG_COMMIT_KEY "CommitKey"
#define ACTION_CONFIG_COMMIT_REVERSE_LEARN_KEY "CommitReverseLearnKey"
#define ACTION_CONFIG_CONVERT_KEY "ConvertKey"
#define ACTION_CONFIG_PREDICT_KEY "PredictKey"
#define ACTION_CONFIG_CANCEL_KEY "CancelKey"
#define ACTION_CONFIG_CANCEL_ALL_KEY "CancelAllKey"

#define ACTION_CONFIG_MOVE_CARET_FIRST_KEY "MoveCaretFirstKey"
#define ACTION_CONFIG_MOVE_CARET_LAST_KEY "MoveCaretLastKey"
#define ACTION_CONFIG_MOVE_CARET_FORWARD_KEY "MoveCaretForwardKey"
#define ACTION_CONFIG_MOVE_CARET_BACKWARD_KEY "MoveCaretBackwardKey"

#define ACTION_CONFIG_SELECT_FIRST_SEGMENT_KEY "SelectFirstSegmentKey"
#define ACTION_CONFIG_SELECT_LAST_SEGMENT_KEY "SelectLastSegmentKey"
#define ACTION_CONFIG_SELECT_NEXT_SEGMENT_KEY "SelectNextSegmentKey"
#define ACTION_CONFIG_SELECT_PREV_SEGMENT_KEY "SelectPrevSegmentKey"
#define ACTION_CONFIG_SHRINK_SEGMENT_KEY "ShrinkSegmentKey"
#define ACTION_CONFIG_EXPAND_SEGMENT_KEY "ExpandSegmentKey"
#define ACTION_CONFIG_COMMIT_FIRST_SEGMENT_KEY "CommitFirstSegmentKey"
#define ACTION_CONFIG_COMMIT_SELECTED_SEGMENT_KEY "CommitSelectedSegmentKey"
#define ACTION_CONFIG_COMMIT_FIRST_SEGMENT_REVERSE_LEARN_KEY                   \
  "CommitFirstSegmentReverseLearnKey"
#define ACTION_CONFIG_COMMIT_SELECTED_SEGMENT_REVERSE_LEARN_KEY                \
  "CommitSelectedSegmentReverseLearnKey"

#define ACTION_CONFIG_SELECT_FIRST_CANDIDATE_KEY "SelectFirstCandidateKey"
#define ACTION_CONFIG_SELECT_LAST_CANDIDATE_KEY "SelectLastCandidateKey"
#define ACTION_CONFIG_SELECT_NEXT_CANDIDATE_KEY "SelectNextCandidateKey"
#define ACTION_CONFIG_SELECT_PREV_CANDIDATE_KEY "SelectPrevCandidateKey"
#define ACTION_CONFIG_SELECT_NEXT_CANDIDATE_ALTER_KEY                          \
  "SelectNextCandidateKeyAlter"
#define ACTION_CONFIG_SELECT_PREV_CANDIDATE_ALTER_KEY                          \
  "SelectPrevCandidateKeyAlter"
#define ACTION_CONFIG_CANDIDATES_PAGE_UP_KEY "CandidatesPageUpKey"
#define ACTION_CONFIG_CANDIDATES_PAGE_DOWN_KEY "CandidatesPageDownKey"
#define ACTION_CONFIG_SELECT_CANDIDATE_1_KEY "SelectCandidates1Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_2_KEY "SelectCandidates2Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_3_KEY "SelectCandidates3Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_4_KEY "SelectCandidates4Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_5_KEY "SelectCandidates5Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_6_KEY "SelectCandidates6Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_7_KEY "SelectCandidates7Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_8_KEY "SelectCandidates8Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_9_KEY "SelectCandidates9Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_10_KEY "SelectCandidates10Key"

#define ACTION_CONFIG_CONV_CHAR_TYPE_FORWARD_KEY "ConvertCharTypeForwardKey"
#define ACTION_CONFIG_CONV_CHAR_TYPE_BACKWARD_KEY "ConvertCharTypeBackwardKey"
#define ACTION_CONFIG_CONV_TO_HIRAGANA_KEY "ConvertToHiraganaKey"
#define ACTION_CONFIG_CONV_TO_KATAKANA_KEY "ConvertToKatakanaKey"
#define ACTION_CONFIG_CONV_TO_HALF_KEY "ConvertToHalfKey"
#define ACTION_CONFIG_CONV_TO_HALF_KATAKANA_KEY "ConvertToHalfKatakanaKey"
#define ACTION_CONFIG_CONV_TO_WIDE_LATIN_KEY "ConvertToWideLatinKey"
#define ACTION_CONFIG_CONV_TO_LATIN_KEY "ConvertToLatinKey"

#define ACTION_CONFIG_RECONVERT_KEY "ReconvertKey"

#define ACTION_CONFIG_DICT_ADMIN_KEY "DictAdminKey"
#define ACTION_CONFIG_ADD_WORD_KEY "AddWordKey"

class AnthyState;

typedef bool (AnthyState::*PMF)();

class Action {

public:
  Action();
  Action(const std::string &name, const fcitx::KeyList &hotkey, PMF pmf);
  FCITX_INLINE_DEFINE_DEFAULT_DTOR_AND_MOVE_WITHOUT_SPEC(Action);

public:
  bool perform(AnthyState *performer);
  bool perform(AnthyState *performer, const fcitx::KeyEvent &key);

  // bool operator<(const Action &b) { return name_ < b.name_; }

  bool matchKeyEvent(const fcitx::KeyEvent &key);

  const auto &name() const { return name_; }

private:
  std::string name_;
  PMF performFunction_;
  const fcitx::KeyList *keyBindings_;
};

#endif // _FCITX5_ANTHY_ACTION_H_
