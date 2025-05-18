/*
 * SPDX-FileCopyrightText: 2004 Hiroyuki Ikezoe
 * SPDX-FileCopyrightText: 2004 Takuro Ashie
 * SPDX-FileCopyrightText: 2017-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

/*
 * The original code is scim_uim_imengine.cpp in scim-uim-0.1.3.
 * SPDX-FileCopyrightText: 2004 James Su <suzhe@tsinghua.org.cn>
 */

#ifndef _FCITX5_ANTHY_STATE_H_
#define _FCITX5_ANTHY_STATE_H_

#include "action.h"
#include "config.h"
#include "conversion.h"
#include "engine.h"
#include "preedit.h"
#include <anthy/anthy.h>
#include <fcitx-utils/key.h>
#include <fcitx/candidatelist.h>
#include <fcitx/event.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>
#include <fcitx/menu.h>
#include <memory>
#include <string>
#include <vector>

class AnthyState : public fcitx::InputContextProperty {
public:
    AnthyState(fcitx::InputContext *ic, AnthyEngine *engine,
               fcitx::Instance *instance);
    ~AnthyState();

    bool needCopy() const override { return true; }

    void copyTo(fcitx::InputContextProperty *property) override;

    void configure();
    void saveConfig() { engine_->saveConfig(); }

    bool processKeyEvent(const fcitx::KeyEvent &key);
    void movePreeditCaret(unsigned int pos);
    void selectCandidate(unsigned int item);
    void reset();
    void deactivate();

    auto inputContext() { return ic_; }

    /* actions */
    bool action_convert();
    bool action_predict();
    bool action_revert();
    bool action_cancel_all();
    bool action_commit_follow_preference();
    bool action_commit_reverse_preference();
    bool action_commit_first_segment();
    bool action_commit_selected_segment();
    bool action_commit_first_segment_reverse_preference();
    bool action_commit_selected_segment_reverse_preference();
    bool action_back();
    bool action_delete();
    bool action_insert_space();
    bool action_insert_alternative_space();
    bool action_insert_half_space();
    bool action_insert_wide_space();

    bool action_move_caret_backward();
    bool action_move_caret_forward();
    bool action_move_caret_first();
    bool action_move_caret_last();

    bool action_select_prev_segment();
    bool action_select_next_segment();
    bool action_select_first_segment();
    bool action_select_last_segment();
    bool action_shrink_segment();
    bool action_expand_segment();

    bool action_select_first_candidate();
    bool action_select_last_candidate();
    bool action_select_next_candidate();
    bool action_select_prev_candidate();
    bool action_candidates_page_up();
    bool action_candidates_page_down();

    bool action_convert_to_hiragana();
    bool action_convert_to_katakana();
    bool action_circle_latin_hiragana_mode();
    bool action_convert_to_half();
    bool action_convert_to_half_katakana();
    bool action_convert_to_latin();
    bool action_convert_to_wide_latin();
    bool action_convert_char_type_forward();
    bool action_convert_char_type_backward();
    bool action_reconvert();

    bool action_circle_input_mode();
    bool action_circle_kana_mode();
    bool action_circle_typing_method();

    bool action_latin_mode();
    bool action_wide_latin_mode();
    bool action_hiragana_mode();
    bool action_katakana_mode();
    bool action_half_katakana_mode();
    bool action_cancel_pseudo_ascii_mode();

    bool action_add_word() const;
    bool action_launch_dict_admin_tool() const;
    /*
    void   actoin_register_word               ();
    */

    TypingMethod typingMethod() const;
    InputMode inputMode() const;
    // Only input mode is per-state.
    void setInputMode(InputMode mode, bool propagate = false);
    void syncConversionMode();
    void syncTypingMethod();
    void syncPeriodCommaStyle();
    void syncSymbolStyle();
    void updateUI();

    int pseudoAsciiMode() const;
    const AnthyConfig &config() const { return engine_->config(); }
    auto engine() { return engine_; }
    fcitx::Instance *instance() { return instance_; }
    bool supportClientPreedit();

    bool actionSelectCandidate(unsigned int i);
    void resetCursor(int cursor);
    void autoCommit(fcitx::InputContextEvent &event);
    void selectCandidateNoDirect(unsigned int item);

private:
    /* processing key event */
    bool processKeyEventInput(const fcitx::KeyEvent &key);
    bool processKeyEventLookupKeybind(const fcitx::KeyEvent &key);
    bool processKeyEventLatinMode(const fcitx::KeyEvent &key);
    bool processKeyEventWideLatinMode(const fcitx::KeyEvent &key);

    /* utility */
    void setPreedition();
    void setAuxString();
    std::shared_ptr<fcitx::CandidateList> setLookupTable();
    void unsetLookupTable();
    bool isSelectingCandidates();
    bool convertKana(CandidateType type);

    bool action_commit(bool learn, bool do_real_commit = true);

    bool isSingleSegment();
    bool isRealtimeConversion();

    bool isNicolaThumbShiftKey(const fcitx::KeyEvent &key) const;
    void commitString(const std::string &str);
    void updateAuxString(const std::string &str);

    fcitx::InputContext *ic_;
    AnthyEngine *engine_;
    fcitx::Instance *instance_;

    /* for preedit */
    Preedit preedit_;
    bool preeditVisible_;

    /* for candidates window */
    // FcitxCandidateWordList *m_lookup_table;
    bool lookupTableVisible_;
    unsigned int nConvKeyPressed_;

    /* for toggling latin and wide latin */
    InputMode prevInputMode_;

    /* for action */
    fcitx::Key lastKey_;

    int cursorPos_;

    // Order matters so we can't use map here.
    std::vector<Action> actions_;
    bool uiUpdate_;
};

#endif // _FCITX5_ANTHY_STATE_H_
