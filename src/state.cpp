/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 - 2005 Hiroyuki Ikezoe <poincare@ikezoe.net>
 *  Copyright (C) 2004 - 2005 Takuro Ashie <ashie@homa.ne.jp>
 *  Copyright (C) 2012 CSSlayer
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*
 * The original code is scim_uim_imengine.cpp in scim-uim-0.1.3.
 * Copyright (C) 2004 James Su <suzhe@tsinghua.org.cn>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "engine.h"
#include "state.h"
#include "utils.h"

#include <fcitx-utils/log.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputpanel.h>
#include <fcitx/statusarea.h>

#include <clipboard_public.h>
#include <fcitx-utils/standardpath.h>

AnthyState::AnthyState(fcitx::InputContext *ic, AnthyEngine *engine,
                       fcitx::Instance *instance)
    : ic_(ic), engine_(engine), instance_(instance), preedit_(*this),
      preeditVisible_(false), lookupTableVisible_(false), nConvKeyPressed_(0),
      prevInputMode_(InputMode::HIRAGANA), cursorPos_(0), uiUpdate_(false) {
    configure();
}

AnthyState::~AnthyState() {}

// FIXME!
bool AnthyState::isNicolaThumbShiftKey(const fcitx::KeyEvent &key) {
    if (typingMethod() != TypingMethod::NICOLA)
        return false;

    if (util::match_key_event(*config().m_key->m_left_thumb_keys, key.rawKey(),
                              fcitx::KeyStates(0xFFFF)) ||
        util::match_key_event(*config().m_key->m_right_thumb_keys, key.rawKey(),
                              fcitx::KeyStates(0xFFFF))) {
        return true;
    }

    return false;
}

bool AnthyState::processKeyEventInput(const fcitx::KeyEvent &key) {
    // prediction while typing
    if (*config().m_general->m_predict_on_input && key.isRelease() &&
        preedit_.isPreediting() && !preedit_.isConverting()) {
        preedit_.predict();
        preedit_.candidates();
    }

    if (!preedit_.canProcessKeyEvent(key)) {
        return false;
    }

    if (preedit_.isConverting()) {
        if (isRealtimeConversion()) {
            action_revert();
        } else if (!isNicolaThumbShiftKey(key)) {
            action_commit(*config().m_general->m_learn_on_auto_commit);
        }
    }

    bool need_commit = preedit_.processKeyEvent(key);

    if (need_commit) {
        if (isRealtimeConversion() && inputMode() != InputMode::LATIN &&
            inputMode() != InputMode::WIDE_LATIN) {
            preedit_.convert(FCITX_ANTHY_CANDIDATE_DEFAULT, isSingleSegment());
        }
        action_commit(*config().m_general->m_learn_on_auto_commit);
    } else {
        if (isRealtimeConversion()) {
            preedit_.convert(FCITX_ANTHY_CANDIDATE_DEFAULT, isSingleSegment());
            preedit_.selectSegment(-1);
        }
        // show_preedit_string ();
        preeditVisible_ = true;
        setPreedition();
    }

    return true;
}

bool AnthyState::processKeyEventLookupKeybind(const fcitx::KeyEvent &key) {
    decltype(actions_)::iterator it;

    if (key.isRelease())
        return false;

    lastKey_ = key.rawKey();

    /* try to find a "insert a blank" action to be not stolen a blank key
     * when entering the pseudo ascii mode.
     */
    if (pseudoAsciiMode() != 0 &&
        *config().m_general->m_romaji_pseudo_ascii_blank_behavior &&
        preedit_.isPseudoAsciiMode()) {
        it = std::find_if(actions_.begin(), actions_.end(),
                          [](const Action &action) {
                              return action.name() == "INSERT_SPACE";
                          });
        if (it != actions_.end() && it->perform(this, key)) {
            return true;
        }
    }

    for (it = actions_.begin(); it != actions_.end(); it++) {
        if (it->perform(this, key)) {
            lastKey_ = fcitx::Key();
            return true;
        }
    }

    int choose = key.rawKey().keyListIndex(util::selection_keys());
    if (choose >= 0) {
        auto candList = ic_->inputPanel().candidateList();
        if (candList && candList->size() > choose) {
            candList->candidate(choose)->select(ic_);
            lastKey_ = fcitx::Key();
            return true;
        }
    }

    lastKey_ = fcitx::Key();

    return false;
}

bool AnthyState::processKeyEventLatinMode(const fcitx::KeyEvent &key) {
    if (key.isRelease())
        return false;

    if (util::key_is_keypad(key.rawKey())) {
        std::string wide;
        auto str = util::keypad_to_string(key);
        if (*config().m_general->m_ten_key_type == TenKeyType::WIDE)
            wide = util::convert_to_wide(str);
        else
            wide = str;
        if (!wide.empty()) {
            commitString(wide);
            return true;
        } else {
            return false;
        }
    } else {
        // for Multi/Dead key
        return false;
    }
}

bool AnthyState::processKeyEventWideLatinMode(const fcitx::KeyEvent &key) {
    if (key.isRelease())
        return false;

    std::string wide;
    auto str = util::keypad_to_string(key);
    if (util::key_is_keypad(key.rawKey()) &&
        *config().m_general->m_ten_key_type == TenKeyType::HALF) {
        wide = str;
    } else {
        wide = util::convert_to_wide(str);
    }
    if (!wide.empty()) {
        commitString(wide);
        return true;
    }

    return false;
}

bool AnthyState::processKeyEvent(const fcitx::KeyEvent &key) {
    // FIXME!
    // for NICOLA thumb shift key
    if (typingMethod() == TypingMethod::NICOLA && isNicolaThumbShiftKey(key)) {
        if (processKeyEventInput(key))
            return true;
    }

    // lookup user defined key bindings
    if (processKeyEventLookupKeybind(key))
        return true;

    if (key.rawKey().isDigit() && ic_->inputPanel().candidateList() &&
        ic_->inputPanel().candidateList()->size()) {
        return false;
    }

    // for Latin mode
    if (preedit_.inputMode() == InputMode::LATIN)
        return processKeyEventLatinMode(key);

    // for wide Latin mode
    if (preedit_.inputMode() == InputMode::WIDE_LATIN)
        return processKeyEventWideLatinMode(key);

    // for other mode
    if (typingMethod() != TypingMethod::NICOLA || !isNicolaThumbShiftKey(key)) {
        if (processKeyEventInput(key))
            return true;
    }

    if (preedit_.isPreediting())
        return true;
    else
        return false;
}

void AnthyState::movePreeditCaret(unsigned int pos) {
    preedit_.setCaretPosByChar(pos);
    // TODO
}

void AnthyState::selectCandidateNoDirect(unsigned int item) {
    if (preedit_.isPredicting() && !preedit_.isConverting())
        action_predict();

    // update lookup table
    cursorPos_ = item;

    // update preedit
    preedit_.selectCandidate(cursorPos_);
    setPreedition();

    setLookupTable();
    if (auto candList = ic_->inputPanel().candidateList()) {
        if (cursorPos_ >= 0 && cursorPos_ < candList->size()) {
            auto commonCandList =
                static_cast<fcitx::CommonCandidateList *>(candList);
            commonCandList->setGlobalCursorIndex(cursorPos_);
            commonCandList->setPage(cursorPos_ /
                                    *config().m_general->m_page_size);
        }
    }

    // update aux string
    if (*config().m_general->m_show_candidates_label)
        setAuxString();
}

void AnthyState::selectCandidate(unsigned int item) {
    selectCandidateNoDirect(item);

    unsetLookupTable();
    action_select_next_segment();
}

void AnthyState::reset() {
    ic_->inputPanel().reset();

    preedit_.clear();
    unsetLookupTable();

    preeditVisible_ = false;
    setPreedition();
}

void AnthyState::init() {
    ic_->inputPanel().reset();
    if (preeditVisible_) {
        setPreedition();
    }

    if (lookupTableVisible_ && isSelectingCandidates()) {
        if (*config().m_general->m_show_candidates_label) {
            setAuxString();
        }
        setLookupTable();
    }

    installProperties();
}

bool AnthyState::supportClientPreedit() {
    return ic_->capabilityFlags().test(fcitx::CapabilityFlag::Preedit);
}

void AnthyState::setPreedition() {
    preedit_.updatePreedit();
    uiUpdate_ = true;
}

void AnthyState::updateUI() {
    if (uiUpdate_) {
        uiUpdate_ = false;
        ic_->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
        ic_->updatePreedit();
    }
}

void AnthyState::setAuxString() {
    if (!ic_->inputPanel().candidateList() ||
        !ic_->inputPanel().candidateList()->size())
        return;

    if (auto bulk = ic_->inputPanel().candidateList()->toBulk()) {
        char buf[256];
        sprintf(buf, _("(%d / %d)"), cursorPos_ + 1, bulk->totalSize());
        updateAuxString(buf);
    }
}

int AnthyState::setLookupTable() {

    // if (!is_selecting_candidates ()) {
    if (isRealtimeConversion() && preedit_.selectedSegment() < 0) {
        // select latest segment
        int n = preedit_.nrSegments();
        if (n < 1)
            return 0;
        preedit_.selectSegment(n - 1);
    }

    // prepare candidates
    auto candList = preedit_.candidates();

    if (candList->size() == 0) {
        return 0;
    }

    // update preedit
    preedit_.selectCandidate(cursorPos_);
    setPreedition();

    bool beyond_threshold =
        *config().m_general->m_n_triggers_to_show_cand_win > 0 &&
        (int)nConvKeyPressed_ >=
            *config().m_general->m_n_triggers_to_show_cand_win;

    int len = candList->totalSize();

    if (!lookupTableVisible_ && (preedit_.isPredicting() || beyond_threshold)) {
        lookupTableVisible_ = true;
        nConvKeyPressed_ = 0;

        if (*config().m_general->m_show_candidates_label) {
            setAuxString();
        }
    } else if (!lookupTableVisible_) {
        candList.reset();
    }

    ic_->inputPanel().setCandidateList(std::move(candList));
    uiUpdate_ = true;

    return len;
}

void AnthyState::unsetLookupTable() {
    ic_->inputPanel().setCandidateList(nullptr);
    lookupTableVisible_ = false;
    nConvKeyPressed_ = 0;
    cursorPos_ = 0;

    ic_->inputPanel().setAuxUp(fcitx::Text());
}

void AnthyState::setPeriodCommaStyle(PeriodCommaStyle period) {
    config().m_general.mutableValue()->m_period_comma_style.setValue(period);
    engine_->periodStyleAction()->update(ic_);

    switch (period) {
    case PeriodCommaStyle::WIDELATIN:
        preedit_.setCommaStyle(CommaStyle::WIDE);
        preedit_.setPeriodStyle(PeriodStyle::WIDE);
        break;
    case PeriodCommaStyle::LATIN:
        preedit_.setCommaStyle(CommaStyle::HALF);
        preedit_.setPeriodStyle(PeriodStyle::HALF);
        break;
    case PeriodCommaStyle::WIDELATIN_JAPANESE:
        preedit_.setCommaStyle(CommaStyle::WIDE);
        preedit_.setPeriodStyle(PeriodStyle::JAPANESE);
        break;
    case PeriodCommaStyle::JAPANESE:
    default:
        preedit_.setCommaStyle(CommaStyle::JAPANESE);
        preedit_.setPeriodStyle(PeriodStyle::JAPANESE);
        break;
    }
}

void AnthyState::setSymbolStyle(SymbolStyle symbol) {
    config().m_general.mutableValue()->m_symbol_style.setValue(symbol);
    engine_->symbolStyleAction()->update(ic_);
    switch (symbol) {
    case SymbolStyle::WIDEBRACKET_WIDESLASH:
        preedit_.setBracketStyle(BracketStyle::JAPANESE);
        preedit_.setSlashStyle(SlashStyle::WIDE);
        break;
    case SymbolStyle::CORNERBRACKET_WIDESLASH:
        preedit_.setBracketStyle(BracketStyle::WIDE);
        preedit_.setSlashStyle(SlashStyle::WIDE);
        break;
    case SymbolStyle::CORNERBRACKET_MIDDLEDOT:
        preedit_.setBracketStyle(BracketStyle::WIDE);
        preedit_.setSlashStyle(SlashStyle::JAPANESE);
        break;
    case SymbolStyle::JAPANESE:
    default:
        preedit_.setBracketStyle(BracketStyle::JAPANESE);
        preedit_.setSlashStyle(SlashStyle::JAPANESE);
        break;
    }
}

void AnthyState::installProperties() {
    if (*config().m_general->m_show_candidates_label) {
        setInputMode(inputMode());
    }
    setConversionMode(*config().m_general->m_conversion_mode);
    setTypingMethod(typingMethod());
    setPeriodCommaStyle(periodCommaStyle());
    setSymbolStyle(symbolStyle());
}

void AnthyState::setInputMode(InputMode mode) {
    if (mode != inputMode()) {
        config().m_general.mutableValue()->m_input_mode.setValue(mode);
        preedit_.setInputMode(mode);
        setPreedition();
    }

    engine_->inputModeAction()->update(ic_);
    if (!engine_->constructed()) {
        return;
    }
    if (ic_->hasFocus() && instance_->inputMethod(ic_) == "anthy") {
        instance_->showInputMethodInformation(ic_);
    }
}

void AnthyState::setConversionMode(ConversionMode mode) {
    config().m_general.mutableValue()->m_conversion_mode.setValue(mode);

    engine_->conversionModeAction()->update(ic_);
}

void AnthyState::setTypingMethod(TypingMethod method) {
    if (method != typingMethod()) {
        preedit_.setTypingMethod(method);
        preedit_.setPseudoAsciiMode(pseudoAsciiMode());
    }

    config().m_general.mutableValue()->m_typing_method.setValue(method);
    engine_->typingMethodAction()->update(ic_);
}

void AnthyState::setPeriodStyle(PeriodStyle period, CommaStyle comma) {
    std::string label;

    switch (comma) {
    case CommaStyle::JAPANESE:
        label = "\xE3\x80\x81";
        break;
    case CommaStyle::WIDE:
        label = "\xEF\xBC\x8C";
        break;
    case CommaStyle::HALF:
        label = ",";
        break;
    default:
        break;
    }

    switch (period) {
    case PeriodStyle::JAPANESE:
        label += "\xE3\x80\x82";
        break;
    case PeriodStyle::WIDE:
        label += "\xEF\xBC\x8E";
        break;
    case PeriodStyle::HALF:
        label += ".";
        break;
    default:
        break;
    }

    if (!label.empty()) {
#if 0
        PropertyList::iterator it = std::find (m_properties.begin (),
                                               m_properties.end (),
                                               SCIM_PROP_PERIOD_STYLE);
        if (it != m_properties.end ()) {
            it->set_label (label.c_str ());
            update_property (*it);
        }
#endif
    }

    if (period != preedit_.periodStyle()) {
        preedit_.setPeriodStyle(period);
    }
    if (comma != preedit_.commaStyle()) {
        preedit_.setCommaStyle(comma);
    }
}

void AnthyState::setSymbolStyle(BracketStyle bracket, SlashStyle slash) {
    if (bracket != preedit_.bracketStyle()) {
        preedit_.setBracketStyle(bracket);
    }
    if (slash != preedit_.slashStyle()) {
        preedit_.setSlashStyle(slash);
    }
}

bool AnthyState::isSelectingCandidates() {
    return ic_->inputPanel().candidateList() &&
           ic_->inputPanel().candidateList()->size();
}

void AnthyState::deactivate() {
    ic_->statusArea().clearGroup(fcitx::StatusGroup::InputMethod);
}

bool AnthyState::action_convert() {
    if (!preedit_.isPreediting())
        return false;

    if (!preedit_.isConverting()) {
        // show conversion string
        preedit_.finish();
        preedit_.convert(FCITX_ANTHY_CANDIDATE_DEFAULT, isSingleSegment());
        setPreedition();
        nConvKeyPressed_++;
        setLookupTable();
        return true;
    }

    return false;
}

bool AnthyState::action_predict() {
    if (!preedit_.isPreediting())
        return false;

    if (preedit_.isConverting())
        return false;

    if (!preedit_.isPredicting())
        preedit_.predict();

    preedit_.selectCandidate(0);
    setPreedition();
    nConvKeyPressed_++;
    setLookupTable();
    selectCandidateNoDirect(0);

    return true;
}

bool AnthyState::action_revert() {
    if (preedit_.isReconverting()) {
        preedit_.revert();
        commitString(preedit_.string());
        reset();
        return true;
    }

    if (!preedit_.isPreediting())
        return false;

    if (!preedit_.isConverting()) {
        reset();
        return true;
    }

    if (isSelectingCandidates()) {
        ic_->inputPanel().setCandidateList(nullptr);
    }

    unsetLookupTable();
    preedit_.revert();
    setPreedition();

    return true;
}

bool AnthyState::action_cancel_all() {
    if (!preedit_.isPreediting())
        return false;

    reset();
    return true;
}

bool AnthyState::action_commit(bool learn, bool do_real_commit) {
    if (!preedit_.isPreediting())
        return false;

    if (preedit_.isConverting()) {
        if (do_real_commit)
            commitString(preedit_.string());
        if (learn)
            preedit_.commit();
    } else {
        preedit_.finish();
        if (do_real_commit)
            commitString(preedit_.string());
    }

    reset();

    return true;
}

bool AnthyState::action_commit_follow_preference() {
    return action_commit(*config().m_general->m_learn_on_manual_commit);
}

bool AnthyState::action_commit_reverse_preference() {
    return action_commit(!*config().m_general->m_learn_on_manual_commit);
}

bool AnthyState::action_back() {
    if (!preedit_.isPreediting())
        return false;

    if (preedit_.isConverting()) {
        action_revert();
        if (!isRealtimeConversion())
            return true;
    }

    preedit_.erase();

    if (preedit_.length() > 0) {
        if (isRealtimeConversion()) {
            preedit_.convert(FCITX_ANTHY_CANDIDATE_DEFAULT, isSingleSegment());
            preedit_.selectSegment(-1);
        }
        setPreedition();
    } else {
        reset();
    }

    return true;
}

bool AnthyState::action_delete() {
    if (!preedit_.isPreediting())
        return false;

    if (preedit_.isConverting()) {
        action_revert();
        if (!isRealtimeConversion())
            return true;
    }

    preedit_.erase(false);

    if (preedit_.length() > 0) {
        if (isRealtimeConversion()) {
            preedit_.convert(FCITX_ANTHY_CANDIDATE_DEFAULT, isSingleSegment());
            preedit_.selectSegment(-1);
        }
        setPreedition();
    } else {
        reset();
    }

    return true;
}

bool AnthyState::action_insert_space() {
    std::string str;
    bool is_wide = false, retval = false;

    if (preedit_.isPreediting() &&
        !*config().m_general->m_romaji_pseudo_ascii_blank_behavior)
        return false;

    if (*config().m_general->m_space_type == SpaceType::FOLLOWMODE) {
        InputMode mode = inputMode();
        if (mode == InputMode::LATIN || mode == InputMode::HALF_KATAKANA ||
            preedit_.isPseudoAsciiMode()) {
            is_wide = false;
        } else {
            is_wide = true;
        }
    } else if (*config().m_general->m_space_type == SpaceType::WIDE) {
        is_wide = true;
    }

    if (is_wide) {
        str = "\xE3\x80\x80";
        retval = true;
    } else if (typingMethod() == TypingMethod::NICOLA || // FIXME! it's a ad-hoc
                                                         // solution.
               preedit_.isPseudoAsciiMode() ||
               (lastKey_.sym() != FcitxKey_space &&
                lastKey_.sym() != FcitxKey_KP_Space)) {
        str = " ";
        retval = true;
    }

    if (retval) {
        if (preedit_.isPseudoAsciiMode()) {
            preedit_.append(lastKey_, str);
            // show_preedit_string ();
            preeditVisible_ = true;
            setPreedition();
        } else {
            commitString(str);
        }
    }

    return retval;
}

bool AnthyState::action_insert_alternative_space() {
    bool is_wide = false;

    if (preedit_.isPreediting())
        return false;

    if (*config().m_general->m_space_type == SpaceType::FOLLOWMODE) {
        InputMode mode = inputMode();
        if (mode == InputMode::LATIN || mode == InputMode::HALF_KATAKANA) {
            is_wide = true;
        } else {
            is_wide = false;
        }
    } else if (*config().m_general->m_space_type != SpaceType::WIDE) {
        is_wide = true;
    }

    if (is_wide) {
        commitString("\xE3\x80\x80");
        return true;
    } else if (typingMethod() == TypingMethod::NICOLA || // FIXME! it's a ad-hoc
                                                         // solution.
               (lastKey_.sym() != FcitxKey_space &&
                lastKey_.sym() != FcitxKey_KP_Space)) {
        commitString(" ");
        return true;
    }

    return false;
}

bool AnthyState::action_insert_half_space() {
    if (preedit_.isPreediting())
        return false;

    if (lastKey_.sym() != FcitxKey_space &&
        lastKey_.sym() != FcitxKey_KP_Space) {
        commitString(" ");
        return true;
    }

    return false;
}

bool AnthyState::action_insert_wide_space() {
    if (preedit_.isPreediting())
        return false;

    commitString("\xE3\x80\x80");

    return true;
}

bool AnthyState::action_move_caret_backward() {
    if (!preedit_.isPreediting())
        return false;
    if (preedit_.isConverting())
        return false;

    preedit_.moveCaret(-1);
    setPreedition();

    return true;
}

bool AnthyState::action_move_caret_forward() {
    if (!preedit_.isPreediting())
        return false;
    if (preedit_.isConverting())
        return false;

    preedit_.moveCaret(1);
    setPreedition();

    return true;
}

bool AnthyState::action_move_caret_first() {
    if (!preedit_.isPreediting())
        return false;
    if (preedit_.isConverting())
        return false;

    preedit_.setCaretPosByChar(0);
    setPreedition();

    return true;
}

bool AnthyState::action_move_caret_last() {
    if (!preedit_.isPreediting())
        return false;
    if (preedit_.isConverting())
        return false;

    preedit_.setCaretPosByChar(preedit_.utf8Length());
    setPreedition();

    return true;
}

bool AnthyState::action_select_prev_segment() {
    if (!preedit_.isConverting())
        return false;

    unsetLookupTable();

    int idx = preedit_.selectedSegment();
    if (idx - 1 < 0) {
        int n = preedit_.nrSegments();
        if (n <= 0)
            return false;
        preedit_.selectSegment(n - 1);
    } else {
        preedit_.selectSegment(idx - 1);
    }
    setPreedition();

    return true;
}

bool AnthyState::action_select_next_segment() {
    if (!preedit_.isConverting())
        return false;

    unsetLookupTable();

    int idx = preedit_.selectedSegment();
    if (idx < 0) {
        preedit_.selectSegment(0);
    } else {
        int n = preedit_.nrSegments();
        if (n <= 0)
            return false;
        if (idx + 1 >= n)
            preedit_.selectSegment(0);
        else
            preedit_.selectSegment(idx + 1);
    }
    setPreedition();

    return true;
}

bool AnthyState::action_select_first_segment() {
    if (!preedit_.isConverting())
        return false;

    unsetLookupTable();

    preedit_.selectSegment(0);
    setPreedition();

    return true;
}

bool AnthyState::action_select_last_segment() {
    if (!preedit_.isConverting())
        return false;

    int n = preedit_.nrSegments();
    if (n <= 0)
        return false;

    unsetLookupTable();

    preedit_.selectSegment(n - 1);
    setPreedition();

    return true;
}

bool AnthyState::action_shrink_segment() {
    if (!preedit_.isConverting())
        return false;

    unsetLookupTable();

    preedit_.resizeSegment(-1);
    setPreedition();

    return true;
}

bool AnthyState::action_expand_segment() {
    if (!preedit_.isConverting())
        return false;

    unsetLookupTable();

    preedit_.resizeSegment(1);
    setPreedition();

    return true;
}

bool AnthyState::action_commit_first_segment() {
    if (!preedit_.isConverting()) {
        if (preedit_.isPreediting()) {
            return action_commit(*config().m_general->m_learn_on_manual_commit);
        } else {
            return false;
        }
    }

    unsetLookupTable();

    commitString(preedit_.segmentString(0));
    if (*config().m_general->m_learn_on_manual_commit)
        preedit_.commit(0);
    else
        preedit_.clear(0);

    setPreedition();

    return true;
}

bool AnthyState::action_commit_selected_segment() {
    if (!preedit_.isConverting()) {
        if (preedit_.isPreediting()) {
            return action_commit(*config().m_general->m_learn_on_manual_commit);
        } else {
            return false;
        }
    }

    unsetLookupTable();

    for (int i = 0; i <= preedit_.selectedSegment(); i++)
        commitString(preedit_.segmentString(i));
    if (*config().m_general->m_learn_on_manual_commit)
        preedit_.commit(preedit_.selectedSegment());
    else
        preedit_.clear(preedit_.selectedSegment());

    setPreedition();

    return true;
}

bool AnthyState::action_commit_first_segment_reverse_preference() {
    if (!preedit_.isConverting()) {
        if (preedit_.isPreediting()) {
            return action_commit(
                !*config().m_general->m_learn_on_manual_commit);
        } else {
            return false;
        }
    }

    unsetLookupTable();

    commitString(preedit_.segmentString(0));
    if (!*config().m_general->m_learn_on_manual_commit)
        preedit_.commit(0);
    else
        preedit_.clear(0);

    setPreedition();

    return true;
}

bool AnthyState::action_commit_selected_segment_reverse_preference() {
    if (!preedit_.isConverting()) {
        if (preedit_.isPreediting()) {
            return action_commit(
                !*config().m_general->m_learn_on_manual_commit);
        } else {
            return false;
        }
    }

    unsetLookupTable();

    for (int i = 0; i <= preedit_.selectedSegment(); i++)
        commitString(preedit_.segmentString(i));
    if (!*config().m_general->m_learn_on_manual_commit)
        preedit_.commit(preedit_.selectedSegment());
    else
        preedit_.clear(preedit_.selectedSegment());

    setPreedition();

    return true;
}

bool AnthyState::action_select_next_candidate() {
    if (!preedit_.isConverting())
        return false;

    // if (!is_selecting_candidates ())
    int end = setLookupTable();

    if (cursorPos_ >= end - 1)
        cursorPos_ = 0;
    else
        cursorPos_++;
    nConvKeyPressed_++;

    selectCandidateNoDirect(cursorPos_);
    return true;
}

bool AnthyState::action_select_prev_candidate() {
    if (!preedit_.isConverting())
        return false;
    // if (!is_selecting_candidates ())
    int end = setLookupTable();

    if (end < 0)
        end = 0;
    if (cursorPos_ == 0)
        cursorPos_ = end - 1;
    else
        cursorPos_--;
    nConvKeyPressed_++;

    if (auto candList = ic_->inputPanel().candidateList()) {
        if (candList->size() > cursorPos_ && cursorPos_ >= 0) {
            auto commonCandList =
                static_cast<fcitx::CommonCandidateList *>(candList);
            commonCandList->setGlobalCursorIndex(cursorPos_);
            commonCandList->setPage(cursorPos_ /
                                    *config().m_general->m_page_size);
        }
    }

    selectCandidateNoDirect(cursorPos_);

    return true;
}

bool AnthyState::action_select_first_candidate() {
    if (!preedit_.isConverting())
        return false;
    if (!isSelectingCandidates())
        return false;

    cursorPos_ = 0;
    nConvKeyPressed_++;
    selectCandidateNoDirect(cursorPos_);
    return true;
}

bool AnthyState::action_select_last_candidate() {
    if (!preedit_.isConverting())
        return false;
    if (!isSelectingCandidates())
        return false;

    int end = ic_->inputPanel().candidateList()->toBulk()->totalSize() - 1;
    if (end < 0)
        end = 0;
    cursorPos_ = end;
    nConvKeyPressed_++;
    selectCandidateNoDirect(cursorPos_);
    return true;
}

bool AnthyState::action_candidates_page_up() {
    if (!preedit_.isConverting())
        return false;
    if (!isSelectingCandidates())
        return false;
    if (!lookupTableVisible_)
        return false;

    if (cursorPos_ - *config().m_general->m_page_size >= 0) {
        cursorPos_ -= *config().m_general->m_page_size;
        selectCandidateNoDirect(cursorPos_);
    }

    return true;
}

bool AnthyState::action_candidates_page_down() {
    if (!preedit_.isConverting())
        return false;
    if (!isSelectingCandidates())
        return false;
    if (!lookupTableVisible_)
        return false;

    int end = ic_->inputPanel().candidateList()->toBulk()->totalSize();

    if (cursorPos_ + *config().m_general->m_page_size < end) {
        cursorPos_ += *config().m_general->m_page_size;
        selectCandidateNoDirect(cursorPos_);
    }

    return true;
}

bool AnthyState::actionSelectCandidate(unsigned int i) {
    // FIXME! m_lookup_table_visible should be set as true also on predicting
    if (!lookupTableVisible_ && !preedit_.isPredicting())
        return false;

    if (preedit_.isPredicting() && !preedit_.isConverting() &&
        *config().m_general->m_use_direct_key_on_predict) {
        ic_->inputPanel().setCandidateList(preedit_.candidates());
        selectCandidate(i);
        return true;
    } else if (preedit_.isConverting() && isSelectingCandidates()) {
        selectCandidate(i);
        return true;
    }

    return false;
}

template <typename T>
T cycle_enum(T v, T size) {
    return static_cast<T>((static_cast<int>(v) + 1) % static_cast<int>(size));
}

bool AnthyState::action_circle_input_mode() {
    InputMode mode = inputMode();

    mode = cycle_enum(mode, InputMode::LAST);

    setInputMode(mode);
    saveConfig();

    return true;
}

bool AnthyState::action_circle_typing_method() {
    TypingMethod method;

    method = typingMethod();
    method = cycle_enum(method, TypingMethod::NICOLA);

    setTypingMethod(method);
    saveConfig();

    return true;
}

bool AnthyState::action_circle_kana_mode() {
    InputMode mode;

    if (inputMode() == InputMode::LATIN ||
        inputMode() == InputMode::WIDE_LATIN) {
        mode = InputMode::HIRAGANA;
    } else {
        switch (inputMode()) {
        case InputMode::HIRAGANA:
            mode = InputMode::KATAKANA;
            break;
        case InputMode::KATAKANA:
            mode = InputMode::HALF_KATAKANA;
            break;
        case InputMode::HALF_KATAKANA:
        default:
            mode = InputMode::HIRAGANA;
            break;
        }
    }

    setInputMode(mode);
    saveConfig();

    return true;
}

bool AnthyState::action_circle_latin_hiragana_mode() {
    InputMode mode = inputMode();

    if (mode == InputMode::LATIN) {
        mode = InputMode::HIRAGANA;
    } else if (mode == InputMode::HIRAGANA) {
        mode = InputMode::LATIN;
    }

    setInputMode(mode);
    saveConfig();

    return true;
}

bool AnthyState::action_latin_mode() {
    setInputMode(InputMode::LATIN);
    saveConfig();
    return true;
}

bool AnthyState::action_wide_latin_mode() {
    setInputMode(InputMode::WIDE_LATIN);
    saveConfig();
    return true;
}

bool AnthyState::action_hiragana_mode() {
    setInputMode(InputMode::HIRAGANA);
    saveConfig();
    return true;
}

bool AnthyState::action_katakana_mode() {
    setInputMode(InputMode::KATAKANA);
    saveConfig();
    return true;
}

bool AnthyState::action_half_katakana_mode() {
    setInputMode(InputMode::HALF_KATAKANA);
    saveConfig();
    return true;
}

bool AnthyState::action_cancel_pseudo_ascii_mode() {
    if (!preedit_.isPreediting())
        return false;
    if (!preedit_.isPseudoAsciiMode())
        return false;

    preedit_.resetPseudoAsciiMode();

    return true;
}

bool AnthyState::convertKana(CandidateType type) {
    if (!preedit_.isPreediting())
        return false;

    if (preedit_.isReconverting())
        return false;

    unsetLookupTable();

    if (preedit_.isConverting()) {
        int idx = preedit_.selectedSegment();
        if (idx < 0) {
            action_revert();
            preedit_.finish();
            preedit_.convert(type, true);
        } else {
            preedit_.selectCandidate(type);
        }
    } else {
        preedit_.finish();
        preedit_.convert(type, true);
    }

    setPreedition();

    return true;
}

bool AnthyState::action_convert_to_hiragana() {
    return convertKana(FCITX_ANTHY_CANDIDATE_HIRAGANA);
}

bool AnthyState::action_convert_to_katakana() {
    return convertKana(FCITX_ANTHY_CANDIDATE_KATAKANA);
}

bool AnthyState::action_convert_to_half() {
    return convertKana(FCITX_ANTHY_CANDIDATE_HALF);
}

bool AnthyState::action_convert_to_half_katakana() {
    return convertKana(FCITX_ANTHY_CANDIDATE_HALF_KATAKANA);
}

bool AnthyState::action_convert_to_latin() {
    return convertKana(FCITX_ANTHY_CANDIDATE_LATIN);
}

bool AnthyState::action_convert_to_wide_latin() {
    return convertKana(FCITX_ANTHY_CANDIDATE_WIDE_LATIN);
}

bool AnthyState::action_convert_char_type_forward() {
    if (!preedit_.isPreediting())
        return false;

    unsetLookupTable();

    if (preedit_.isConverting()) {
        int idx = preedit_.selectedSegment();
        if (idx < 0) {
            action_revert();
            preedit_.finish();
            preedit_.convert(FCITX_ANTHY_CANDIDATE_HIRAGANA, true);
        } else {
            int cand = preedit_.selectedCandidate();
            switch (cand) {
            case FCITX_ANTHY_CANDIDATE_HIRAGANA:
                preedit_.selectCandidate(FCITX_ANTHY_CANDIDATE_KATAKANA);
                break;
            case FCITX_ANTHY_CANDIDATE_KATAKANA:
                preedit_.selectCandidate(FCITX_ANTHY_CANDIDATE_HALF_KATAKANA);
                break;
            case FCITX_ANTHY_CANDIDATE_HALF_KATAKANA:
                preedit_.selectCandidate(FCITX_ANTHY_CANDIDATE_WIDE_LATIN);
                break;
            case FCITX_ANTHY_CANDIDATE_WIDE_LATIN:
                preedit_.selectCandidate(FCITX_ANTHY_CANDIDATE_LATIN);
                break;
            case FCITX_ANTHY_CANDIDATE_LATIN:
                preedit_.selectCandidate(FCITX_ANTHY_CANDIDATE_HIRAGANA);
                break;
            default:
                preedit_.selectCandidate(FCITX_ANTHY_CANDIDATE_HIRAGANA);
                break;
            }
        }
    } else {
        preedit_.finish();
        preedit_.convert(FCITX_ANTHY_CANDIDATE_HIRAGANA, true);
    }

    setPreedition();

    return true;
}

bool AnthyState::action_convert_char_type_backward() {
    if (!preedit_.isPreediting())
        return false;

    unsetLookupTable();

    if (preedit_.isConverting()) {
        int idx = preedit_.selectedSegment();
        if (idx < 0) {
            action_revert();
            preedit_.finish();
            preedit_.convert(FCITX_ANTHY_CANDIDATE_HIRAGANA, true);
        } else {
            int cand = preedit_.selectedCandidate();
            switch (cand) {
            case FCITX_ANTHY_CANDIDATE_HIRAGANA:
                preedit_.selectCandidate(FCITX_ANTHY_CANDIDATE_LATIN);
                break;
            case FCITX_ANTHY_CANDIDATE_KATAKANA:
                preedit_.selectCandidate(FCITX_ANTHY_CANDIDATE_HIRAGANA);
                break;
            case FCITX_ANTHY_CANDIDATE_HALF_KATAKANA:
                preedit_.selectCandidate(FCITX_ANTHY_CANDIDATE_KATAKANA);
                break;
            case FCITX_ANTHY_CANDIDATE_WIDE_LATIN:
                preedit_.selectCandidate(FCITX_ANTHY_CANDIDATE_HALF_KATAKANA);
                break;
            case FCITX_ANTHY_CANDIDATE_LATIN:
                preedit_.selectCandidate(FCITX_ANTHY_CANDIDATE_WIDE_LATIN);
                break;
            default:
                preedit_.selectCandidate(FCITX_ANTHY_CANDIDATE_HIRAGANA);
                break;
            }
        }
    } else {
        preedit_.finish();
        preedit_.convert(FCITX_ANTHY_CANDIDATE_HIRAGANA, true);
    }

    setPreedition();

    return true;
}

bool AnthyState::action_reconvert() {
    if (preedit_.isPreediting())
        return false;

    if (!ic_->capabilityFlags().test(fcitx::CapabilityFlag::SurroundingText)) {
        return true;
    }
    if (!ic_->surroundingText().isValid()) {
        return true;
    }

    const std::string surrounding_text(ic_->surroundingText().text());
    uint cursor_pos = ic_->surroundingText().cursor();
    uint anchor_pos = ic_->surroundingText().anchor();
    int32_t relative_selected_length = 0;

    if (cursor_pos == anchor_pos) {
        if (engine_->clipboard()) {
            auto primary_text =
                engine_->clipboard()->call<fcitx::IClipboard::primary>(ic_);
            uint new_anchor_pos = 0;
            if (util::surrounding_get_anchor_pos_from_selection(
                    surrounding_text, primary_text, cursor_pos,
                    &new_anchor_pos)) {
                anchor_pos = new_anchor_pos;
            } else {
                return true;
            }
        } else {
            // There is no selection text.
            return true;
        }
    }

    if (!util::surrounding_get_safe_delta(cursor_pos, anchor_pos,
                                          &relative_selected_length)) {
        return true;
    }

    const uint32_t selection_start = std::min(cursor_pos, anchor_pos);
    const uint32_t selection_length = abs(relative_selected_length);
    std::string text = util::utf8_string_substr(
        surrounding_text, selection_start, selection_length);

    ic_->surroundingText().deleteText(
        cursor_pos > anchor_pos ? -relative_selected_length : 0,
        selection_length);

    preedit_.convert(text);
    setPreedition();
    setLookupTable();

    return true;
}

bool AnthyState::action_add_word() {
    util::launch_program(*config().m_command->m_add_word_command);

    return true;
}

bool AnthyState::action_launch_dict_admin_tool() {
    util::launch_program(*config().m_command->m_dict_admin_command);

    return true;
}

TypingMethod AnthyState::typingMethod() { return preedit_.typingMethod(); }

InputMode AnthyState::inputMode() { return preedit_.inputMode(); }

bool AnthyState::isSingleSegment() {
    return (conversionMode() == ConversionMode::SINGLE_SEGMENT ||
            conversionMode() == ConversionMode::SINGLE_SEGMENT_IMMEDIATE);
}

bool AnthyState::isRealtimeConversion() {
    return (conversionMode() == ConversionMode::MULTI_SEGMENT_IMMEDIATE ||
            conversionMode() == ConversionMode::SINGLE_SEGMENT_IMMEDIATE);
}

int AnthyState::pseudoAsciiMode() {
    int retval = 0;
    TypingMethod m = typingMethod();

    if (m == TypingMethod::ROMAJI) {
        if (*config().m_general->m_romaji_pseudo_ascii_mode)
            retval |= FCITX_ANTHY_PSEUDO_ASCII_TRIGGERED_CAPITALIZED;
    }

    return retval;
}

void AnthyState::commitString(const std::string &str) {
    ic_->commitString(str);
}

#define APPEND_ACTION(key, func)                                               \
    {                                                                          \
        const fcitx::KeyList *hk;                                              \
        std::string name = #key;                                               \
        if (loaded) {                                                          \
            std::string str = (ACTION_CONFIG_##key##_KEY);                     \
            std::string keystr;                                                \
            style.getString(keystr, "KeyBindings", str);                       \
            key_profile().m_hk_##key = fcitx::Key::keyListFromString(keystr);  \
            hk = &key_profile().m_hk_##key;                                    \
        } else                                                                 \
            hk = &*config().m_key->m_hk_##key;                                 \
        PMF f;                                                                 \
        f = &AnthyState::func;                                                 \
        actions_[name] = Action(name, *hk, f);                                 \
    }

void AnthyState::configure() {
    auto keyProfile = engine_->keyProfile();

    // clear old actions
    actions_.clear();
// convert key
#define FOREACH_ACTION(key, func)                                              \
    {                                                                          \
        std::string name = #key;                                               \
        const fcitx::KeyList *hk;                                              \
        if (keyProfile) {                                                      \
            hk = &keyProfile->m_hk_##key;                                      \
        } else                                                                 \
            hk = &*config().m_key->m_hk_##key;                                 \
        PMF f;                                                                 \
        f = &AnthyState::func;                                                 \
        actions_.emplace_back(name, *hk, f);                                   \
    }
#include "action_defs.h"
#undef FOREACH_ACTION

    // set romaji settings
    preedit_.setSymbolHalf(*config().m_general->m_romaji_half_symbol);
    preedit_.setNumberHalf(*config().m_general->m_romaji_half_number);

    // set input mode
    preedit_.setInputMode(*config().m_general->m_input_mode);

    // set typing method and pseudo ASCII mode
    preedit_.setTypingMethod(*config().m_general->m_typing_method);
    preedit_.setPseudoAsciiMode(pseudoAsciiMode());

    // set period style
    setPeriodCommaStyle(*config().m_general->m_period_comma_style);

    // set symbol style
    setSymbolStyle(*config().m_general->m_symbol_style);

    // setup toolbar
    installProperties();
}

void AnthyState::updateAuxString(const std::string &str) {
    fcitx::Text aux;
    aux.append(str);
    ic_->inputPanel().setAuxUp(std::move(aux));
    uiUpdate_ = true;
}

void AnthyState::resetCursor(int cursor) {
    if (cursor >= 0)
        cursorPos_ = cursor;
    else
        cursor = 0;
}

void AnthyState::autoCommit(fcitx::InputContextEvent &event) {
    if (event.type() == fcitx::EventType::InputContextFocusOut) {
        action_commit(*config().m_general->m_learn_on_auto_commit, false);
    } else if (event.type() ==
               fcitx::EventType::InputContextSwitchInputMethod) {
        action_commit(*config().m_general->m_learn_on_auto_commit);
    }
    reset();
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
