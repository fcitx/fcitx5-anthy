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

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#include "factory.h"
#include "imengine.h"
#include "utils.h"

#include <fcitx/context.h>
#include <fcitx-config/xdg.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx-utils/log.h>

#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))

#define UTF8_BRACKET_CORNER_BEGIN "\xE3\x80\x8C"
#define UTF8_BRACKET_CORNER_END   "\xE3\x80\x8D"
#define UTF8_BRACKET_WIDE_BEGIN   "\xEF\xBC\xBB"
#define UTF8_BRACKET_WIDE_END     "\xEF\xBC\xBD"
#define UTF8_MIDDLE_DOT           "\xE3\x83\xBB"
#define UTF8_SLASH_WIDE           "\xEF\xBC\x8F"

#define ACTION_CONFIG_CIRCLE_INPUT_MODE_KEY       "CircleInputModeKey"
#define ACTION_CONFIG_CIRCLE_KANA_MODE_KEY        "CircleKanaModeKey"
#define ACTION_CONFIG_CIRCLE_TYPING_METHOD_KEY    "CircleTypingMethodKey"
#define ACTION_CONFIG_LATIN_MODE_KEY              "LatinModeKey"
#define ACTION_CONFIG_WIDE_LATIN_MODE_KEY         "WideLatinModeKey"
#define ACTION_CONFIG_HIRAGANA_MODE_KEY           "HiraganaModeKey"
#define ACTION_CONFIG_KATAKANA_MODE_KEY           "KatakanaModeKey"
#define ACTION_CONFIG_HALF_KATAKANA_MODE_KEY      "HalfKatakanaModeKey"
#define ACTION_CONFIG_CANCEL_PSEUDO_ASCII_MODE_KEY  "CancelPseudoAsciiModeKey"

#define ACTION_CONFIG_INSERT_SPACE_KEY            "InsertSpaceKey"
#define ACTION_CONFIG_INSERT_ALT_SPACE_KEY        "InsertAltSpaceKey"
#define ACTION_CONFIG_INSERT_HALF_SPACE_KEY       "InsertHalfSpaceKey"
#define ACTION_CONFIG_INSERT_WIDE_SPACE_KEY       "InsertWideSpaceKey"
#define ACTION_CONFIG_BACKSPACE_KEY               "BackSpaceKey"
#define ACTION_CONFIG_DELETE_KEY                  "DeleteKey"
#define ACTION_CONFIG_COMMIT_KEY                  "CommitKey"
#define ACTION_CONFIG_COMMIT_REVERSE_LEARN_KEY    "CommitReverseLearnKey"
#define ACTION_CONFIG_CONVERT_KEY                 "ConvertKey"
#define ACTION_CONFIG_PREDICT_KEY                 "PredictKey"
#define ACTION_CONFIG_CANCEL_KEY                  "CancelKey"
#define ACTION_CONFIG_CANCEL_ALL_KEY              "CancelAllKey"

#define ACTION_CONFIG_MOVE_CARET_FIRST_KEY        "MoveCaretFirstKey"
#define ACTION_CONFIG_MOVE_CARET_LAST_KEY         "MoveCaretLastKey"
#define ACTION_CONFIG_MOVE_CARET_FORWARD_KEY      "MoveCaretForwardKey"
#define ACTION_CONFIG_MOVE_CARET_BACKWARD_KEY     "MoveCaretBackwardKey"

#define ACTION_CONFIG_SELECT_FIRST_SEGMENT_KEY    "SelectFirstSegmentKey"
#define ACTION_CONFIG_SELECT_LAST_SEGMENT_KEY     "SelectLastSegmentKey"
#define ACTION_CONFIG_SELECT_NEXT_SEGMENT_KEY     "SelectNextSegmentKey"
#define ACTION_CONFIG_SELECT_PREV_SEGMENT_KEY     "SelectPrevSegmentKey"
#define ACTION_CONFIG_SHRINK_SEGMENT_KEY          "ShrinkSegmentKey"
#define ACTION_CONFIG_EXPAND_SEGMENT_KEY          "ExpandSegmentKey"
#define ACTION_CONFIG_COMMIT_FIRST_SEGMENT_KEY    "CommitFirstSegmentKey"
#define ACTION_CONFIG_COMMIT_SELECTED_SEGMENT_KEY "CommitSelectedSegmentKey"
#define ACTION_CONFIG_COMMIT_FIRST_SEGMENT_REVERSE_LEARN_KEY    "CommitFirstSegmentReverseLearnKey"
#define ACTION_CONFIG_COMMIT_SELECTED_SEGMENT_REVERSE_LEARN_KEY "CommitSelectedSegmentReverseLearnKey"

#define ACTION_CONFIG_SELECT_FIRST_CANDIDATE_KEY  "SelectFirstCandidateKey"
#define ACTION_CONFIG_SELECT_LAST_CANDIDATE_KEY   "SelectLastCandidateKey"
#define ACTION_CONFIG_SELECT_NEXT_CANDIDATE_KEY   "SelectNextCandidateKey"
#define ACTION_CONFIG_SELECT_PREV_CANDIDATE_KEY   "SelectPrevCandidateKey"
#define ACTION_CONFIG_CANDIDATES_PAGE_UP_KEY      "CandidatesPageUpKey"
#define ACTION_CONFIG_CANDIDATES_PAGE_DOWN_KEY    "CandidatesPageDownKey"
#define ACTION_CONFIG_SELECT_CANDIDATE_1_KEY      "SelectCandidates1Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_2_KEY      "SelectCandidates2Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_3_KEY      "SelectCandidates3Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_4_KEY      "SelectCandidates4Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_5_KEY      "SelectCandidates5Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_6_KEY      "SelectCandidates6Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_7_KEY      "SelectCandidates7Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_8_KEY      "SelectCandidates8Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_9_KEY      "SelectCandidates9Key"
#define ACTION_CONFIG_SELECT_CANDIDATE_10_KEY     "SelectCandidates10Key"

#define ACTION_CONFIG_CONV_CHAR_TYPE_FORWARD_KEY  "ConvertCharTypeForwardKey"
#define ACTION_CONFIG_CONV_CHAR_TYPE_BACKWARD_KEY "ConvertCharTypeBackwardKey"
#define ACTION_CONFIG_CONV_TO_HIRAGANA_KEY        "ConvertToHiraganaKey"
#define ACTION_CONFIG_CONV_TO_KATAKANA_KEY        "ConvertToKatakanaKey"
#define ACTION_CONFIG_CONV_TO_HALF_KEY            "ConvertToHalfKey"
#define ACTION_CONFIG_CONV_TO_HALF_KATAKANA_KEY   "ConvertToHalfKatakanaKey"
#define ACTION_CONFIG_CONV_TO_WIDE_LATIN_KEY      "ConvertToWideLatinKey"
#define ACTION_CONFIG_CONV_TO_LATIN_KEY           "ConvertToLatinKey"

#define ACTION_CONFIG_RECONVERT_KEY               "ReconvertKey"

#define ACTION_CONFIG_DICT_ADMIN_KEY              "DictAdminKey"
#define ACTION_CONFIG_ADD_WORD_KEY                "AddWordKey"

#define UTF8_BRACKET_CORNER_BEGIN "\xE3\x80\x8C"
#define UTF8_BRACKET_CORNER_END   "\xE3\x80\x8D"
#define UTF8_BRACKET_WIDE_BEGIN   "\xEF\xBC\xBB"
#define UTF8_BRACKET_WIDE_END     "\xEF\xBC\xBD"
#define UTF8_MIDDLE_DOT           "\xE3\x83\xBB"
#define UTF8_SLASH_WIDE           "\xEF\xBC\x8F"

#define N_(x) (x)

AnthyInstance::AnthyInstance (FcitxInstance* instance) :
      m_owner(instance),
      m_preedit                (*this),
      m_preedit_string_visible (false),
      m_input                  (FcitxInstanceGetInputState(m_owner)),
      m_lookup_table           (FcitxInputStateGetCandidateList(m_input)),
      m_lookup_table_visible   (false),
      m_n_conv_key_pressed     (0),
      m_prev_input_mode        (FCITX_ANTHY_MODE_HIRAGANA),
      m_last_key               (),
      m_aux_up                 (FcitxInputStateGetAuxUp(m_input)),
      m_aux_down               (FcitxInputStateGetAuxDown(m_input)),
      m_cursor_pos             (0),
      m_client_preedit_msg     (FcitxInputStateGetClientPreedit(m_input)),
      m_preedit_msg            (FcitxInputStateGetPreedit(m_input)),
      m_profile                (FcitxInstanceGetProfile(m_owner)),
      m_status_installed       (false),
      m_ui_update              (false)
{
    memset(&m_config, 0, sizeof(FcitxAnthyConfig));

}

AnthyInstance::~AnthyInstance ()
{
}

// FIXME!
bool
AnthyInstance::is_nicola_thumb_shift_key (const KeyEvent &key)
{
    if (get_typing_method () != FCITX_ANTHY_TYPING_METHOD_NICOLA)
        return false;

    if (util_match_key_event (m_config.m_left_thumb_keys, key, 0xFFFF) ||
        util_match_key_event (m_config.m_right_thumb_keys, key, 0xFFFF))
    {
        return true;
    }

    return false;
}

bool
AnthyInstance::process_key_event_input (const KeyEvent &key)
{
    // prediction while typing
    if (m_config.m_predict_on_input && key.is_release &&
        m_preedit.is_preediting () && !m_preedit.is_converting ())
    {
        m_preedit.predict ();
        m_preedit.get_candidates (m_lookup_table);
    }

    if (!m_preedit.can_process_key_event (key)) {
        return false;
    }

    if (m_preedit.is_converting ()) {
        if (is_realtime_conversion ()) {
            action_revert ();
        } else if (!is_nicola_thumb_shift_key (key)) {
            action_commit (m_config.m_learn_on_auto_commit);
        }
    }

    bool need_commit = m_preedit.process_key_event (key);

    if (need_commit) {
        if (is_realtime_conversion () &&
            get_input_mode () != FCITX_ANTHY_MODE_LATIN &&
            get_input_mode () != FCITX_ANTHY_MODE_WIDE_LATIN)
        {
            m_preedit.convert (FCITX_ANTHY_CANDIDATE_DEFAULT,
                               is_single_segment ());
        }
        action_commit (m_config.m_learn_on_auto_commit);
    } else {
        if (is_realtime_conversion ()) {
            m_preedit.convert (FCITX_ANTHY_CANDIDATE_DEFAULT,
                               is_single_segment ());
            m_preedit.select_segment (-1);
        }
        //show_preedit_string ();
        m_preedit_string_visible = true;
        set_preedition ();
    }

    return true;
}

bool
AnthyInstance::process_key_event_lookup_keybind (const KeyEvent& key)
{
    std::map<std::string, Action>::iterator it;

    if (key.is_release)
        return false;

    m_last_key = key;

    /* try to find a "insert a blank" action to be not stolen a blank key
     * when entering the pseudo ascii mode.
     */
    if (get_pseudo_ascii_mode () != 0 &&
        m_config.m_romaji_pseudo_ascii_blank_behavior &&
        m_preedit.is_pseudo_ascii_mode ()) {
        it = m_actions.find("INSERT_SPACE");
        if (it != m_actions.end()) {
            it->second.perform (this, key);
            return true;
        }
    }

    for (it  = m_actions.begin();
         it != m_actions.end();
         it++)
    {
        if (it->second.perform (this, key)) {
            m_last_key = KeyEvent ();
            return true;
        }
    }

    int choose = FcitxHotkeyCheckChooseKey(key.sym, key.state & (FcitxKeyState_Ctrl_Alt_Shift | FcitxKeyState_Super), DIGIT_STR_CHOOSE);
    if (choose >= 0) {
        INPUT_RETURN_VALUE retVal = FcitxCandidateWordChooseByIndex(m_lookup_table, choose);
        if (retVal != IRV_TO_PROCESS) {
            m_last_key = KeyEvent ();
            return true;
        }
    }

    m_last_key = KeyEvent ();

    return false;
}

bool
AnthyInstance::process_key_event_latin_mode (const KeyEvent &key)
{
    if (key.is_release)
        return false;

    if (util_key_is_keypad (key)) {
        std::string str;
        std::string wide;
        util_keypad_to_string (str, key);
        if (m_config.m_ten_key_type == FCITX_ANTHY_TEN_KEY_TYPE_WIDE)
            util_convert_to_wide (wide, str);
        else
            wide = str;
        if (wide.length () > 0) {
            commit_string (wide);
            return true;
        } else {
            return false;
        }
    } else {
        // for Multi/Dead key
        return false;
    }
}

bool
AnthyInstance::process_key_event_wide_latin_mode (const KeyEvent &key)
{
    if (key.is_release)
        return false;

    std::string str;
    std::string wide;
    util_keypad_to_string (str, key);
    if (util_key_is_keypad (key) && m_config.m_ten_key_type == FCITX_ANTHY_TEN_KEY_TYPE_HALF)
        wide = str;
    else
        util_convert_to_wide (wide, str);
    if (wide.length () > 0) {
        commit_string (wide);
        return true;
    }

    return false;
}

bool
AnthyInstance::process_key_event (const KeyEvent& key)
{
    // FIXME!
    // for NICOLA thumb shift key
    if (get_typing_method () == FCITX_ANTHY_TYPING_METHOD_NICOLA &&
        is_nicola_thumb_shift_key (key))
    {
        if (process_key_event_input (key))
            return true;
    }

    // lookup user defined key bindings
    if (process_key_event_lookup_keybind (key))
        return true;

    if (FcitxHotkeyIsHotKeyDigit(key.sym, key.state)
        && FcitxCandidateWordGetListSize(m_lookup_table) > 0
    ) {
        return false;
    }

    // for Latin mode
    if (m_preedit.get_input_mode () == FCITX_ANTHY_MODE_LATIN)
        return process_key_event_latin_mode (key);

    // for wide Latin mode
    if (m_preedit.get_input_mode () == FCITX_ANTHY_MODE_WIDE_LATIN)
        return process_key_event_wide_latin_mode (key);

    // for other mode
    if (get_typing_method () != FCITX_ANTHY_TYPING_METHOD_NICOLA ||
        !is_nicola_thumb_shift_key (key))
    {
        if (process_key_event_input (key))
            return true;
    }

    if (m_preedit.is_preediting ())
        return true;
    else
        return false;
}

void
AnthyInstance::move_preedit_caret (unsigned int pos)
{
    m_preedit.set_caret_pos_by_char (pos);
    // TODO
}

void
AnthyInstance::select_candidate_no_direct (unsigned int item)
{
    if (m_preedit.is_predicting () && !m_preedit.is_converting ())
        action_predict ();

    // update lookup table
    m_cursor_pos = item;

    // update preedit
    m_preedit.select_candidate (m_cursor_pos);
    set_preedition ();

    set_lookup_table();
    FcitxCandidateWordSetFocus(m_lookup_table, m_cursor_pos);

    // update aux string
    if (m_config.m_show_candidates_label)
        set_aux_string ();
}

void
AnthyInstance::select_candidate (unsigned int item)
{
    select_candidate_no_direct (item);

    unset_lookup_table ();
    action_select_next_segment();
}

void
AnthyInstance::update_lookup_table_page_size (unsigned int page_size)
{
    FcitxCandidateWordSetPageSize(m_lookup_table, page_size);
}

void
AnthyInstance::reset_im ()
{
    FcitxInstanceCleanInputWindow(m_owner);

    m_preedit.clear ();
    unset_lookup_table ();

    m_preedit_string_visible = false;
    set_preedition ();
}

void
AnthyInstance::init ()
{
    boolean flag = true;
    FcitxInstanceSetContext(m_owner, CONTEXT_IM_KEYBOARD_LAYOUT, "jp");
    FcitxInstanceSetContext(m_owner, CONTEXT_DISABLE_AUTOENG, &flag);
    FcitxInstanceSetContext(m_owner, CONTEXT_DISABLE_QUICKPHRASE, &flag);
    FcitxInstanceSetContext(m_owner, CONTEXT_DISABLE_FULLWIDTH, &flag);
    FcitxInstanceSetContext(m_owner, CONTEXT_DISABLE_AUTO_FIRST_CANDIDATE_HIGHTLIGHT, &flag);
    FcitxInstanceCleanInputWindow(m_owner);
    if (m_preedit_string_visible) {
        set_preedition ();
    }

    if (m_lookup_table_visible && is_selecting_candidates ()) {
        if (m_config.m_show_candidates_label &&
            FcitxCandidateWordGetListSize(m_lookup_table))
        {
            set_aux_string ();
        }
        set_lookup_table ();
    }

    install_properties ();
}

bool
AnthyInstance::support_client_preedit(void)
{
    FcitxInputContext* ic = FcitxInstanceGetCurrentIC(m_owner);
    if (ic && ((ic->contextCaps & CAPACITY_PREEDIT) == 0 || !m_profile->bUsePreedit))
        return false;
    else
        return true;
}


void
AnthyInstance::set_preedition (void)
{
    FcitxMessagesSetMessageCount(m_preedit_msg, 0);
    FcitxMessagesSetMessageCount(m_client_preedit_msg, 0);
    m_preedit.update_preedit();
    if (!support_client_preedit())
        FcitxInputStateSetShowCursor(m_input, true);
    FcitxInputStateSetCursorPos(m_input, m_preedit.get_caret_pos());
    FcitxInputStateSetClientCursorPos(m_input, m_preedit.get_caret_pos());
    m_ui_update = true;
}

void
AnthyInstance::update_ui (void)
{
    if (m_ui_update) {
        m_ui_update = false;
        FcitxUIUpdateInputWindow(m_owner);
    }
}

void
AnthyInstance::set_aux_string (void)
{
    if (!FcitxCandidateWordGetListSize(m_lookup_table))
        return;

    char buf[256];
    sprintf (buf, _("(%d / %d)"), m_cursor_pos + 1,
             FcitxCandidateWordGetListSize(m_lookup_table));
    update_aux_string (buf);
}

int
AnthyInstance::set_lookup_table (void)
{
    FcitxCandidateWordSetChoose(m_lookup_table, DIGIT_STR_CHOOSE);
    FcitxCandidateWordSetPageSize(m_lookup_table, m_config.m_page_size);

    // if (!is_selecting_candidates ()) {
    if (is_realtime_conversion () &&
        m_preedit.get_selected_segment () < 0)
    {
        // select latest segment
        int n = m_preedit.get_nr_segments ();
        if (n < 1)
            return 0;
        m_preedit.select_segment (n - 1);
    }

    // prepare candidates
    m_preedit.get_candidates (m_lookup_table);

    if (FcitxCandidateWordPageCount(m_lookup_table) == 0)
        return 0;

    // update preedit
    m_preedit.select_candidate (m_cursor_pos);
    set_preedition ();

    bool beyond_threshold =
        m_config.m_n_triggers_to_show_cand_win > 0 &&
        (int) m_n_conv_key_pressed >= m_config.m_n_triggers_to_show_cand_win;

    int len = FcitxCandidateWordGetListSize(m_lookup_table);

    if (!m_lookup_table_visible &&
        (m_preedit.is_predicting () || beyond_threshold))
    {
        m_lookup_table_visible = true;
        m_n_conv_key_pressed = 0;

        if (m_config.m_show_candidates_label) {
            set_aux_string ();
        }
    } else if (!m_lookup_table_visible) {
        FcitxCandidateWordReset(m_lookup_table);
    }

    m_ui_update = true;

    return len;
}

void
AnthyInstance::unset_lookup_table (void)
{
    FcitxCandidateWordReset(m_lookup_table);
    m_lookup_table_visible = false;
    m_n_conv_key_pressed = 0;
    m_cursor_pos = 0;

    FcitxMessagesSetMessageCount(m_aux_up, 0);
}

AnthyStatus input_mode_status[] = {
    {"",  "\xe3\x81\x82", N_("Hiragana") },
    {"", "\xe3\x82\xa2", N_("Katakana") },
    {"", "\xef\xbd\xb1", N_("Half width katakana") },
    {"", "A", N_("Direct input") },
    {"", "\xef\xbc\xa1", N_("Wide latin") },
};

AnthyStatus typing_method_status[] = {
    {"", N_("Romaji"), N_("Romaji") },
    {"", N_("Kana"), N_("Kana") },
    {"", N_("Nicola"), N_("Thumb shift") },
};

AnthyStatus conversion_mode_status[] = {
    {"", "\xE9\x80\xA3", N_("Multi segment") },
    {"", "\xE5\x8D\x98", N_("Single segment") },
    {"", "\xE9\x80\x90", N_("Convert as you type (Multi segment)") },
    {"", "\xE9\x80\x90", N_("Convert as you type (Single segment)") },
};

AnthyStatus period_style_status[] = {
    {"anthy-period-wide-latin", "\xEF\xBC\x8C\xEF\xBC\x8E", "\xEF\xBC\x8C\xEF\xBC\x8E" },
    {"anthy-period-latin", ",.", ",." },
    {"anthy-period-japanese", "\xE3\x80\x81\xE3\x80\x82", "\xE3\x80\x81\xE3\x80\x82" },
    {"anthy-period-wide-japanese", "\xEF\xBC\x8C\xE3\x80\x82", "\xEF\xBC\x8C\xE3\x80\x82" },
};

AnthyStatus symbol_style_status[] = {
    {"anthy-symbol", UTF8_BRACKET_CORNER_BEGIN
                        UTF8_BRACKET_CORNER_END
                        UTF8_MIDDLE_DOT,
                        UTF8_BRACKET_CORNER_BEGIN
                        UTF8_BRACKET_CORNER_END
                        UTF8_MIDDLE_DOT },
    {"anthy-symbol", UTF8_BRACKET_CORNER_BEGIN
                        UTF8_BRACKET_CORNER_END
                        UTF8_SLASH_WIDE,
                        UTF8_BRACKET_CORNER_BEGIN
                        UTF8_BRACKET_CORNER_END
                        UTF8_SLASH_WIDE },
    {"anthy-symbol", UTF8_BRACKET_WIDE_BEGIN
                        UTF8_BRACKET_WIDE_END
                        UTF8_MIDDLE_DOT,
                        UTF8_BRACKET_WIDE_BEGIN
                        UTF8_BRACKET_WIDE_END
                        UTF8_MIDDLE_DOT
    },
    {"anthy-symbol", UTF8_BRACKET_WIDE_BEGIN
                        UTF8_BRACKET_WIDE_END
                        UTF8_SLASH_WIDE,
                        UTF8_BRACKET_WIDE_BEGIN
                        UTF8_BRACKET_WIDE_END
                        UTF8_SLASH_WIDE
    },
};

const char*
GetInputModeIconName(void* arg)
{
    AnthyInstance* anthy = (AnthyInstance*) arg;
    return anthy->get_input_mode_icon();
}

const char*
GetTypingMethodIconName(void* arg)
{
    AnthyInstance* anthy = (AnthyInstance*) arg;
    return anthy->get_typing_method_icon();
}

const char*
GetConversionModeIconName(void* arg)
{
    AnthyInstance* anthy = (AnthyInstance*) arg;
    return anthy->get_conversion_mode_icon();
}

const char*
GetPeriodStyleIconName(void* arg)
{
    AnthyInstance* anthy = (AnthyInstance*) arg;
    return anthy->get_period_style_icon();
}

const char*
GetSymbolStyleIconName(void* arg)
{
    AnthyInstance* anthy = (AnthyInstance*) arg;
    return anthy->get_symbol_style_icon();
}

const char* AnthyInstance::get_input_mode_icon()
{
    return input_mode_status[m_config.m_input_mode].icon;
}

const char* AnthyInstance::get_typing_method_icon()
{
    return typing_method_status[m_config.m_typing_method].icon;
}

const char* AnthyInstance::get_conversion_mode_icon()
{
    return conversion_mode_status[m_config.m_conversion_mode].icon;
}

const char* AnthyInstance::get_period_style_icon()
{
    return period_style_status[m_config.m_period_comma_style].icon;
}

const char* AnthyInstance::get_symbol_style_icon()
{
    return symbol_style_status[m_config.m_symbol_style].icon;
}

#define DEFINE_MENU_ACTION(NAME, TYPE, FUNC) \
    void Update##NAME##Menu(struct _FcitxUIMenu *menu) \
    { \
        AnthyInstance* anthy = (AnthyInstance*) menu->priv; \
        menu->mark = anthy->get_##FUNC(); \
    } \
    boolean NAME##MenuAction(struct _FcitxUIMenu *menu, int index) \
    { \
        AnthyInstance* anthy = (AnthyInstance*) menu->priv; \
        anthy->set_##FUNC((TYPE) index); \
        anthy->save_config(); \
        return true; \
    }

DEFINE_MENU_ACTION(InputMode, InputMode, input_mode)
DEFINE_MENU_ACTION(TypingMethod, TypingMethod, typing_method)
DEFINE_MENU_ACTION(ConversionMode, ConversionMode, conversion_mode)
DEFINE_MENU_ACTION(PeriodStyle, PeriodCommaStyle, period_style)
DEFINE_MENU_ACTION(SymbolStyle, SymbolStyle, symbol_style)

void AnthyInstance::set_period_style(PeriodCommaStyle period)
{
    m_config.m_period_comma_style = period;
    FcitxUISetStatusString(m_owner,
                            "anthy-period-style",
                            _(period_style_status[period].label),
                            _(period_style_status[period].description));

    switch (m_config.m_period_comma_style)
    {
        case FCITX_ANTHY_PERIOD_COMMA_WIDELATIN:
            m_preedit.set_comma_style  (FCITX_ANTHY_COMMA_WIDE);
            m_preedit.set_period_style (FCITX_ANTHY_PERIOD_WIDE);
            break;
        case FCITX_ANTHY_PERIOD_COMMA_LATIN:
            m_preedit.set_comma_style  (FCITX_ANTHY_COMMA_HALF);
            m_preedit.set_period_style (FCITX_ANTHY_PERIOD_HALF);
            break;
        case FCITX_ANTHY_PERIOD_COMMA_WIDELATIN_JAPANESE:
            m_preedit.set_comma_style  (FCITX_ANTHY_COMMA_WIDE);
            m_preedit.set_period_style (FCITX_ANTHY_PERIOD_JAPANESE);
            break;
        case FCITX_ANTHY_PERIOD_COMMA_JAPANESE:
        default:
            m_preedit.set_comma_style  (FCITX_ANTHY_COMMA_JAPANESE);
            m_preedit.set_period_style (FCITX_ANTHY_PERIOD_JAPANESE);
            break;
    }
}

void AnthyInstance::set_symbol_style(SymbolStyle symbol)
{
    m_config.m_symbol_style = symbol;
    FcitxUISetStatusString(m_owner,
                            "anthy-symbol-style",
                            _(symbol_style_status[symbol].label),
                            _(symbol_style_status[symbol].description));
    switch (m_config.m_symbol_style)
    {
        case FCITX_ANTHY_SYMBOL_STYLE_WIDEBRACKET_WIDESLASH:
            m_preedit.set_bracket_style (FCITX_ANTHY_BRACKET_WIDE);
            m_preedit.set_slash_style   (FCITX_ANTHY_SLASH_WIDE);
        case FCITX_ANTHY_SYMBOL_STYLE_CORNERBRACKET_WIDESLASH:
            m_preedit.set_bracket_style (FCITX_ANTHY_BRACKET_JAPANESE);
            m_preedit.set_slash_style   (FCITX_ANTHY_SLASH_WIDE);
        case FCITX_ANTHY_SYMBOL_STYLE_WIDEBRACKET_MIDDLEDOT:
            m_preedit.set_bracket_style (FCITX_ANTHY_BRACKET_WIDE);
            m_preedit.set_slash_style   (FCITX_ANTHY_SLASH_JAPANESE);
        case FCITX_ANTHY_SYMBOL_STYLE_JAPANESE:
        default:
            m_preedit.set_bracket_style (FCITX_ANTHY_BRACKET_JAPANESE);
            m_preedit.set_slash_style   (FCITX_ANTHY_SLASH_JAPANESE);
            break;
    }
}


void
AnthyInstance::install_properties (void)
{
    if (!m_status_installed) {
        m_status_installed = true;

#define INIT_MENU(VARNAME, NAME, I18NNAME, STATUS_NAME, STATUS_ARRAY, SIZE) \
        FcitxUIRegisterComplexStatus(m_owner, this, \
            STATUS_NAME, \
            I18NNAME, \
            I18NNAME, \
            NULL, \
            Get##NAME##IconName \
        ); \
        FcitxMenuInit(&VARNAME); \
        VARNAME.name = strdup(I18NNAME); \
        VARNAME.candStatusBind = strdup(STATUS_NAME); \
        VARNAME.UpdateMenu = Update##NAME##Menu; \
        VARNAME.MenuAction = NAME##MenuAction; \
        VARNAME.priv = this; \
        VARNAME.isSubMenu = false; \
        for (int i = 0; i < SIZE; i ++) \
            FcitxMenuAddMenuItem(&VARNAME, _(STATUS_ARRAY[i].label), MENUTYPE_SIMPLE, NULL); \
        FcitxUIRegisterMenu(m_owner, &VARNAME); \
        FcitxUISetStatusVisable(m_owner, STATUS_NAME, false);

        INIT_MENU(m_input_mode_menu, InputMode, _("Input Mode"), "anthy-input-mode", input_mode_status, FCITX_ANTHY_MODE_LAST);
        INIT_MENU(m_typing_method_menu, TypingMethod, _("Typing Method"), "anthy-typing-method", typing_method_status, FCITX_ANTHY_TYPING_METHOD_LAST);
        INIT_MENU(m_conversion_mode_menu, ConversionMode, _("Conversion Mode"), "anthy-conversion-mode", conversion_mode_status, FCITX_ANTHY_CONVERSION_MODE_LAST);
        INIT_MENU(m_period_style_menu, PeriodStyle, _("Period Style"), "anthy-period-style", period_style_status, FCITX_ANTHY_PERIOD_COMMA_LAST);
        INIT_MENU(m_symbol_style_menu, SymbolStyle, _("Symbol Style"), "anthy-symbol-style", symbol_style_status, FCITX_ANTHY_SYMBOL_STYLE_LAST);
    }

    if (m_config.m_show_input_mode_label )

    set_input_mode(get_input_mode ());
    set_conversion_mode (m_config.m_conversion_mode);
    set_typing_method (get_typing_method ());
    set_period_style (get_period_style());
    set_symbol_style (get_symbol_style());
}

void
AnthyInstance::set_input_mode (InputMode mode)
{
    if (mode >= FCITX_ANTHY_MODE_LAST)
        return;
    if (mode != get_input_mode ()) {
        m_config.m_input_mode = mode;
        m_preedit.set_input_mode (mode);
        set_preedition ();
    }

    FcitxUISetStatusString(m_owner,
                            "anthy-input-mode",
                            _(input_mode_status[mode].label),
                            _(input_mode_status[mode].description));
}

void
AnthyInstance::set_conversion_mode (ConversionMode mode)
{
    if (mode >= FCITX_ANTHY_CONVERSION_MODE_LAST)
        return;

    m_config.m_conversion_mode = mode;

    FcitxUISetStatusString(m_owner,
                            "anthy-conversion-mode",
                            _(conversion_mode_status[mode].label),
                            _(conversion_mode_status[mode].description));
}

void
AnthyInstance::set_typing_method (TypingMethod method)
{
    if (method != get_typing_method ()) {
        m_preedit.set_typing_method (method);
        m_preedit.set_pseudo_ascii_mode (get_pseudo_ascii_mode ());
    }

    m_config.m_typing_method = method;

    FcitxUISetStatusString(m_owner,
                            "anthy-typing-method",
                            _(typing_method_status[method].label),
                            _(typing_method_status[method].description));
}

void
AnthyInstance::set_period_style (PeriodStyle period,
                                 CommaStyle  comma)
{
    std::string label;

    switch (comma) {
    case FCITX_ANTHY_COMMA_JAPANESE:
        label = "\xE3\x80\x81";
        break;
    case FCITX_ANTHY_COMMA_WIDE:
        label = "\xEF\xBC\x8C";
        break;
    case FCITX_ANTHY_COMMA_HALF:
        label = ",";
        break;
    default:
        break;
    }

    switch (period) {
    case FCITX_ANTHY_PERIOD_JAPANESE:
        label += "\xE3\x80\x82";
        break;
    case FCITX_ANTHY_PERIOD_WIDE:
        label += "\xEF\xBC\x8E";
        break;
    case FCITX_ANTHY_PERIOD_HALF:
        label += ".";
        break;
    default:
        break;
    }

    if (label.length () > 0) {
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

    if (period != m_preedit.get_period_style ())
        m_preedit.set_period_style (period);
    if (comma != m_preedit.get_comma_style ())
        m_preedit.set_comma_style (comma);
}

void
AnthyInstance::set_symbol_style (BracketStyle bracket,
                                 SlashStyle   slash)
{
    std::string label;

    switch (bracket) {
    case FCITX_ANTHY_BRACKET_JAPANESE:
        label = UTF8_BRACKET_CORNER_BEGIN UTF8_BRACKET_CORNER_END;
        break;
    case FCITX_ANTHY_BRACKET_WIDE:
        label = UTF8_BRACKET_WIDE_BEGIN UTF8_BRACKET_WIDE_END;
        break;
    default:
        break;
    }

    switch (slash) {
    case FCITX_ANTHY_SLASH_JAPANESE:
        label += UTF8_MIDDLE_DOT;
        break;
    case FCITX_ANTHY_SLASH_WIDE:
        label += UTF8_SLASH_WIDE;
        break;
    default:
        break;
    }

    if (label.length () > 0) {
#if 0
        PropertyList::iterator it = std::find (m_properties.begin (),
                                               m_properties.end (),
                                               SCIM_PROP_SYMBOL_STYLE);
        if (it != m_properties.end ()) {
            it->set_label (label.c_str ());
            update_property (*it);
        }
#endif
    }

    if (bracket != m_preedit.get_bracket_style ())
        m_preedit.set_bracket_style (bracket);
    if (slash != m_preedit.get_slash_style ())
        m_preedit.set_slash_style (slash);
}

bool
AnthyInstance::is_selecting_candidates (void)
{
    if (FcitxCandidateWordGetListSize(m_lookup_table))
        return true;
    else
        return false;
}

void AnthyInstance::reset(void )
{
    FcitxIM* im = FcitxInstanceGetCurrentIM(m_owner);
#define RESET_STATUS(CONFIG_NAME, STATUS_NAME) \
    if (m_config.CONFIG_NAME &&  im && strcmp(im->uniqueName, "anthy") == 0) \
        FcitxUISetStatusVisable(m_owner, STATUS_NAME, true); \
    else \
        FcitxUISetStatusVisable(m_owner, STATUS_NAME, false);

    RESET_STATUS(m_show_input_mode_label, "anthy-input-mode")
    RESET_STATUS(m_show_typing_method_label, "anthy-typing-method")
    RESET_STATUS(m_show_conv_mode_label, "anthy-conversion-mode")
    RESET_STATUS(m_show_period_style_label, "anthy-period-style")
    RESET_STATUS(m_show_symbol_style_label, "anthy-symbol-style")
}

bool
AnthyInstance::action_convert (void)
{
    if (!m_preedit.is_preediting ())
        return false;

    if (!m_preedit.is_converting ()) {
        // show conversion string
        m_preedit.finish ();
        m_preedit.convert (FCITX_ANTHY_CANDIDATE_DEFAULT,
                           is_single_segment ());
        set_preedition ();
        m_n_conv_key_pressed++;
        set_lookup_table ();
        return true;
    }

    return false;
}

bool
AnthyInstance::action_predict (void)
{
    if (!m_preedit.is_preediting ())
        return false;

    if (m_preedit.is_converting ())
        return false;

    if (!m_preedit.is_predicting ())
        m_preedit.predict ();

    m_preedit.select_candidate (0);
    set_preedition ();
    m_n_conv_key_pressed++;
    set_lookup_table ();
    select_candidate_no_direct (0);

    return true;
}


bool
AnthyInstance::action_revert (void)
{
    if (m_preedit.is_reconverting ()) {
        m_preedit.revert ();
        commit_string (m_preedit.get_string ());
        reset_im ();
        return true;
    }

    if (!m_preedit.is_preediting ())
        return false;

    if (!m_preedit.is_converting ()) {
        reset_im ();
        return true;
    }

    if (is_selecting_candidates ()) {
        FcitxCandidateWordReset(m_lookup_table);
    }

    unset_lookup_table ();
    m_preedit.revert ();
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_cancel_all (void)
{
    if (!m_preedit.is_preediting ())
        return false;

    reset_im ();
    return true;
}

bool
AnthyInstance::action_commit (bool learn)
{
    if (!m_preedit.is_preediting ())
        return false;

    if (m_preedit.is_converting ()) {
        commit_string (m_preedit.get_string ());
        if (learn)
            m_preedit.commit ();
    } else {
        m_preedit.finish ();
        commit_string (m_preedit.get_string ());
    }

    reset_im ();

    return true;
}

bool
AnthyInstance::action_commit_follow_preference (void)
{
    return action_commit (m_config.m_learn_on_manual_commit);
}

bool
AnthyInstance::action_commit_reverse_preference (void)
{
    return action_commit (!m_config.m_learn_on_manual_commit);
}

bool
AnthyInstance::action_back (void)
{
    if (!m_preedit.is_preediting ())
        return false;

    if (m_preedit.is_converting ()) {
        action_revert ();
        if (!is_realtime_conversion ())
            return true;
    }

    m_preedit.erase ();

    if (m_preedit.get_length () > 0) {
        if (is_realtime_conversion ()) {
            m_preedit.convert (FCITX_ANTHY_CANDIDATE_DEFAULT,
                               is_single_segment ());
            m_preedit.select_segment (-1);
        }
        set_preedition ();
    } else {
        reset_im ();
    }

    return true;
}

bool
AnthyInstance::action_delete (void)
{
    if (!m_preedit.is_preediting ())
        return false;

    if (m_preedit.is_converting ()) {
        action_revert ();
        if (!is_realtime_conversion ())
            return true;
    }

    m_preedit.erase (false);

    if (m_preedit.get_length () > 0) {
        if (is_realtime_conversion ()) {
            m_preedit.convert (FCITX_ANTHY_CANDIDATE_DEFAULT,
                               is_single_segment ());
            m_preedit.select_segment (-1);
        }
        set_preedition ();
    } else {
        reset_im ();
    }

    return true;
}

bool
AnthyInstance::action_insert_space (void)
{
    std::string str;
    bool is_wide = false, retval = false;

    if (m_preedit.is_preediting () && !m_config.m_romaji_pseudo_ascii_blank_behavior)
        return false;

    if (m_config.m_space_type == FCITX_ANTHY_SPACE_TYPE_FOLLOWMODE) {
        InputMode mode = get_input_mode ();
        if (mode == FCITX_ANTHY_MODE_LATIN ||
            mode == FCITX_ANTHY_MODE_HALF_KATAKANA ||
            m_preedit.is_pseudo_ascii_mode ())
        {
            is_wide = false;
        } else {
            is_wide = true;
        }
    } else if (m_config.m_space_type == FCITX_ANTHY_SPACE_TYPE_WIDE) {
        is_wide = true;
    }

    if (is_wide) {
        str = "\xE3\x80\x80";
        retval = true;
    } else if (get_typing_method () == FCITX_ANTHY_TYPING_METHOD_NICOLA || // FIXME! it's a ad-hoc solution.
               m_preedit.is_pseudo_ascii_mode () ||
               (m_last_key.sym != FcitxKey_space &&
                m_last_key.sym != FcitxKey_KP_Space))
    {
        str = " ";
        retval = true;
    }

    if (retval) {
        if (m_preedit.is_pseudo_ascii_mode ()) {
            m_preedit.append (m_last_key, str);
            // show_preedit_string ();
            m_preedit_string_visible = true;
            set_preedition ();
        } else {
            commit_string (str);
        }
    }

    return retval;
}

bool
AnthyInstance::action_insert_alternative_space (void)
{
    bool is_wide = false;

    if (m_preedit.is_preediting ())
        return false;

    if (m_config.m_space_type == FCITX_ANTHY_SPACE_TYPE_FOLLOWMODE) {
        InputMode mode = get_input_mode ();
        if (mode == FCITX_ANTHY_MODE_LATIN ||
            mode == FCITX_ANTHY_MODE_HALF_KATAKANA)
        {
            is_wide = true;
        } else {
            is_wide = false;
        }
    } else if (m_config.m_space_type != FCITX_ANTHY_SPACE_TYPE_WIDE) {
        is_wide = true;
    }

    if (is_wide) {
        commit_string ("\xE3\x80\x80");
        return true;
    } else if (get_typing_method () == FCITX_ANTHY_TYPING_METHOD_NICOLA || // FIXME! it's a ad-hoc solution.
               (m_last_key.sym != FcitxKey_space &&
                m_last_key.sym != FcitxKey_KP_Space))
    {
        commit_string (" ");
        return true;
    }

    return false;
}

bool
AnthyInstance::action_insert_half_space (void)
{
    if (m_preedit.is_preediting ())
        return false;

    if (m_last_key.sym != FcitxKey_space &&
        m_last_key.sym != FcitxKey_KP_Space)
    {
        commit_string (" ");
        return true;
    }

    return false;
}

bool
AnthyInstance::action_insert_wide_space (void)
{
    if (m_preedit.is_preediting ())
        return false;

    commit_string ("\xE3\x80\x80");

    return true;
}

bool
AnthyInstance::action_move_caret_backward (void)
{
    if (!m_preedit.is_preediting ())
        return false;
    if (m_preedit.is_converting ())
        return false;

    m_preedit.move_caret(-1);
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_move_caret_forward (void)
{
    if (!m_preedit.is_preediting ())
        return false;
    if (m_preedit.is_converting ())
        return false;

    m_preedit.move_caret(1);
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_move_caret_first (void)
{
    if (!m_preedit.is_preediting ())
        return false;
    if (m_preedit.is_converting ())
        return false;

    m_preedit.set_caret_pos_by_char (0);
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_move_caret_last (void)
{
    if (!m_preedit.is_preediting ())
        return false;
    if (m_preedit.is_converting ())
        return false;

    m_preedit.set_caret_pos_by_char (m_preedit.get_length_by_char ());
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_select_prev_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    unset_lookup_table ();

    int idx = m_preedit.get_selected_segment ();
    if (idx - 1 < 0) {
        int n = m_preedit.get_nr_segments ();
        if (n <= 0) return false;
        m_preedit.select_segment (n - 1);
    } else {
        m_preedit.select_segment (idx - 1);
    }
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_select_next_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    unset_lookup_table ();

    int idx = m_preedit.get_selected_segment ();
    if (idx < 0) {
        m_preedit.select_segment(0);
    } else {
        int n = m_preedit.get_nr_segments ();
        if (n <= 0)
            return false;
        if (idx + 1 >= n)
            m_preedit.select_segment(0);
        else
            m_preedit.select_segment(idx + 1);
    }
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_select_first_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    unset_lookup_table ();

    m_preedit.select_segment(0);
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_select_last_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    int n = m_preedit.get_nr_segments ();
    if (n <= 0) return false;

    unset_lookup_table ();

    m_preedit.select_segment(n - 1);
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_shrink_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    unset_lookup_table ();

    m_preedit.resize_segment (-1);
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_expand_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    unset_lookup_table ();

    m_preedit.resize_segment (1);
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_commit_first_segment (void)
{
    if (!m_preedit.is_converting ()) {
        if (m_preedit.is_preediting ()) {
            return action_commit (m_config.m_learn_on_manual_commit);
        } else {
            return false;
        }
    }

    unset_lookup_table ();

    commit_string (m_preedit.get_segment_string (0));
    if (m_config.m_learn_on_manual_commit)
        m_preedit.commit (0);
    else
        m_preedit.clear (0);

    set_preedition ();

    return true;
}

bool
AnthyInstance::action_commit_selected_segment (void)
{
    if (!m_preedit.is_converting ()) {
        if (m_preedit.is_preediting ()) {
            return action_commit (m_config.m_learn_on_manual_commit);
        } else {
            return false;
        }
    }

    unset_lookup_table ();

    for (int i = 0; i <= m_preedit.get_selected_segment (); i++)
        commit_string (m_preedit.get_segment_string (i));
    if (m_config.m_learn_on_manual_commit)
        m_preedit.commit (m_preedit.get_selected_segment ());
    else
        m_preedit.clear (m_preedit.get_selected_segment ());

    set_preedition ();

    return true;
}

bool
AnthyInstance::action_commit_first_segment_reverse_preference (void)
{
    if (!m_preedit.is_converting ()) {
        if (m_preedit.is_preediting ()) {
            return action_commit (!m_config.m_learn_on_manual_commit);
        } else {
            return false;
        }
    }

    unset_lookup_table ();

    commit_string (m_preedit.get_segment_string (0));
    if (!m_config.m_learn_on_manual_commit)
        m_preedit.commit (0);
    else
        m_preedit.clear (0);

    set_preedition ();

    return true;
}

bool
AnthyInstance::action_commit_selected_segment_reverse_preference (void)
{
    if (!m_preedit.is_converting ()) {
        if (m_preedit.is_preediting ()) {
            return action_commit (!m_config.m_learn_on_manual_commit);
        } else {
            return false;
        }
    }

    unset_lookup_table ();

    for (int i = 0; i <= m_preedit.get_selected_segment (); i++)
        commit_string (m_preedit.get_segment_string (i));
    if (!m_config.m_learn_on_manual_commit)
        m_preedit.commit (m_preedit.get_selected_segment ());
    else
        m_preedit.clear (m_preedit.get_selected_segment ());

    set_preedition ();

    return true;
}

bool
AnthyInstance::action_select_next_candidate (void)
{
    if (!m_preedit.is_converting ())
        return false;

    //if (!is_selecting_candidates ())
    int end = set_lookup_table ();

    if (m_cursor_pos >= end - 1)
        m_cursor_pos = 0;
    else
        m_cursor_pos ++;
    m_n_conv_key_pressed++;

    select_candidate_no_direct (m_cursor_pos);
    return true;
}

bool
AnthyInstance::action_select_prev_candidate (void)
{
    if (!m_preedit.is_converting ()) return false;
    //if (!is_selecting_candidates ())
    int end = set_lookup_table ();

    if (end < 0)
        end = 0;
    if (m_cursor_pos == 0)
        m_cursor_pos = end;
    else
        m_cursor_pos --;
    m_n_conv_key_pressed++;

    FcitxCandidateWordSetFocus(m_lookup_table, m_cursor_pos);

    select_candidate_no_direct (m_cursor_pos);

    return true;
}

bool
AnthyInstance::action_select_first_candidate (void)
{
    if (!m_preedit.is_converting ()) return false;
    if (!is_selecting_candidates ()) return false;

    m_cursor_pos = 0;
    m_n_conv_key_pressed++;
    select_candidate_no_direct (m_cursor_pos);
    return true;
}

bool
AnthyInstance::action_select_last_candidate (void)
{
    if (!m_preedit.is_converting ()) return false;
    if (!is_selecting_candidates ()) return false;

    int end = FcitxCandidateWordGetListSize(m_lookup_table) - 1;
    if (end < 0)
        end = 0;
    m_cursor_pos = 0;
    m_n_conv_key_pressed++;
    select_candidate_no_direct (m_cursor_pos);
    return true;
}

bool
AnthyInstance::action_candidates_page_up(void)
{
    if (!m_preedit.is_converting ()) return false;
    if (!is_selecting_candidates ()) return false;
    if (!m_lookup_table_visible) return false;

    if (m_cursor_pos - m_config.m_page_size >= 0) {
        m_cursor_pos -= m_config.m_page_size;
        select_candidate_no_direct (m_cursor_pos);
    }

    return true;
}

bool
AnthyInstance::action_candidates_page_down (void)
{
    if (!m_preedit.is_converting ()) return false;
    if (!is_selecting_candidates ()) return false;
    if (!m_lookup_table_visible) return false;

    int end = FcitxCandidateWordGetListSize(m_lookup_table);

    if (m_cursor_pos + m_config.m_page_size < end) {
        FcitxLog(INFO, "page down");
        m_cursor_pos += m_config.m_page_size;
        select_candidate_no_direct (m_cursor_pos);
    }

    return true;
}

bool
AnthyInstance::action_select_candidate (unsigned int i)
{
    // FIXME! m_lookup_table_visible should be set as true also on predicting
    if (!m_lookup_table_visible && !m_preedit.is_predicting ())
        return false;

    if (m_preedit.is_predicting () && !m_preedit.is_converting () &&
        m_config.m_use_direct_key_on_predict)
    {
        m_preedit.get_candidates (m_lookup_table);
        select_candidate (i);
        return true;
    } else if (m_preedit.is_converting () && is_selecting_candidates ()) {
        select_candidate (i);
        return true;
    }

    return false;
}

bool
AnthyInstance::action_circle_input_mode (void)
{
    InputMode mode = get_input_mode ();

    mode = (InputMode) ((mode + 1) % FCITX_ANTHY_MODE_LAST);

    set_input_mode (mode);
    save_config();

    return true;
}

bool
AnthyInstance::action_circle_typing_method (void)
{
    TypingMethod method;

    method = get_typing_method ();
    method = (TypingMethod) ((method + 1) % FCITX_ANTHY_TYPING_METHOD_NICOLA);

    set_typing_method (method);
    save_config();

    return true;
}

bool
AnthyInstance::action_circle_kana_mode (void)
{
    InputMode mode;

    if (get_input_mode () == FCITX_ANTHY_MODE_LATIN ||
        get_input_mode () == FCITX_ANTHY_MODE_WIDE_LATIN)
    {
        mode = FCITX_ANTHY_MODE_HIRAGANA;
    } else {
        switch (get_input_mode ()) {
        case FCITX_ANTHY_MODE_HIRAGANA:
            mode = FCITX_ANTHY_MODE_KATAKANA;
            break;
        case FCITX_ANTHY_MODE_KATAKANA:
            mode = FCITX_ANTHY_MODE_HALF_KATAKANA;
            break;
        case FCITX_ANTHY_MODE_HALF_KATAKANA:
        default:
            mode = FCITX_ANTHY_MODE_HIRAGANA;
            break;
        }
    }

    set_input_mode (mode);
    save_config();

    return true;
}

bool
AnthyInstance::action_latin_mode (void)
{
    set_input_mode (FCITX_ANTHY_MODE_LATIN);
    save_config();
    return true;
}

bool
AnthyInstance::action_wide_latin_mode (void)
{
    set_input_mode (FCITX_ANTHY_MODE_WIDE_LATIN);
    save_config();
    return true;
}

bool
AnthyInstance::action_hiragana_mode (void)
{
    set_input_mode (FCITX_ANTHY_MODE_HIRAGANA);
    save_config();
    return true;
}

bool
AnthyInstance::action_katakana_mode (void)
{
    set_input_mode (FCITX_ANTHY_MODE_KATAKANA);
    save_config();
    return true;
}

bool
AnthyInstance::action_half_katakana_mode (void)
{
    set_input_mode (FCITX_ANTHY_MODE_HALF_KATAKANA);
    save_config();
    return true;
}

bool
AnthyInstance::action_cancel_pseudo_ascii_mode (void)
{
    if (!m_preedit.is_preediting ())
        return false;
    if (!m_preedit.is_pseudo_ascii_mode ())
        return false;

    m_preedit.reset_pseudo_ascii_mode ();

    return true;
}

bool
AnthyInstance::convert_kana (CandidateType type)
{
    if (!m_preedit.is_preediting ())
        return false;

    if (m_preedit.is_reconverting ())
        return false;

    unset_lookup_table ();

    if (m_preedit.is_converting ()) {
        int idx = m_preedit.get_selected_segment ();
        if (idx < 0) {
            action_revert ();
            m_preedit.finish ();
            m_preedit.convert (type, true);
        } else {
            m_preedit.select_candidate (type);
        }
    } else {
        m_preedit.finish ();
        m_preedit.convert (type, true);
    }

    set_preedition ();

    return true;
}

bool
AnthyInstance::action_convert_to_hiragana (void)
{
    return convert_kana (FCITX_ANTHY_CANDIDATE_HIRAGANA);
}

bool
AnthyInstance::action_convert_to_katakana (void)
{
    return convert_kana (FCITX_ANTHY_CANDIDATE_KATAKANA);
}

bool
AnthyInstance::action_convert_to_half (void)
{
    return convert_kana (FCITX_ANTHY_CANDIDATE_HALF);
}

bool
AnthyInstance::action_convert_to_half_katakana (void)
{
    return convert_kana (FCITX_ANTHY_CANDIDATE_HALF_KATAKANA);
}

bool
AnthyInstance::action_convert_to_latin (void)
{
    return convert_kana (FCITX_ANTHY_CANDIDATE_LATIN);
}

bool
AnthyInstance::action_convert_to_wide_latin (void)
{
    return convert_kana (FCITX_ANTHY_CANDIDATE_WIDE_LATIN);
}

bool
AnthyInstance::action_convert_char_type_forward (void)
{
    if (!m_preedit.is_preediting ())
        return false;

    unset_lookup_table ();

    if (m_preedit.is_converting ()) {
        int idx = m_preedit.get_selected_segment ();
        if (idx < 0) {
            action_revert ();
            m_preedit.finish ();
            m_preedit.convert (FCITX_ANTHY_CANDIDATE_HIRAGANA, true);
        } else {
            int cand = m_preedit.get_selected_candidate ();
            switch (cand)
            {
            case FCITX_ANTHY_CANDIDATE_HIRAGANA:
                m_preedit.select_candidate (FCITX_ANTHY_CANDIDATE_KATAKANA);
                break;
            case FCITX_ANTHY_CANDIDATE_KATAKANA:
                m_preedit.select_candidate (FCITX_ANTHY_CANDIDATE_HALF_KATAKANA);
                break;
            case FCITX_ANTHY_CANDIDATE_HALF_KATAKANA:
                m_preedit.select_candidate (FCITX_ANTHY_CANDIDATE_WIDE_LATIN);
                break;
            case FCITX_ANTHY_CANDIDATE_WIDE_LATIN:
                m_preedit.select_candidate (FCITX_ANTHY_CANDIDATE_LATIN);
                break;
            case FCITX_ANTHY_CANDIDATE_LATIN:
                m_preedit.select_candidate (FCITX_ANTHY_CANDIDATE_HIRAGANA);
                break;
            default:
                m_preedit.select_candidate (FCITX_ANTHY_CANDIDATE_HIRAGANA);
                break;
            }
        }
    } else {
        m_preedit.finish ();
        m_preedit.convert (FCITX_ANTHY_CANDIDATE_HIRAGANA, true);
    }

    set_preedition ();

    return true;
}

bool
AnthyInstance::action_convert_char_type_backward (void)
{
    if (!m_preedit.is_preediting ())
        return false;

    unset_lookup_table ();

    if (m_preedit.is_converting ()) {
        int idx = m_preedit.get_selected_segment ();
        if (idx < 0) {
            action_revert ();
            m_preedit.finish ();
            m_preedit.convert (FCITX_ANTHY_CANDIDATE_HIRAGANA, true);
        } else {
            int cand = m_preedit.get_selected_candidate ();
            switch (cand)
            {
            case FCITX_ANTHY_CANDIDATE_HIRAGANA:
                m_preedit.select_candidate (FCITX_ANTHY_CANDIDATE_LATIN);
                break;
            case FCITX_ANTHY_CANDIDATE_KATAKANA:
                m_preedit.select_candidate (FCITX_ANTHY_CANDIDATE_HIRAGANA);
                break;
            case FCITX_ANTHY_CANDIDATE_HALF_KATAKANA:
                m_preedit.select_candidate (FCITX_ANTHY_CANDIDATE_KATAKANA);
                break;
            case FCITX_ANTHY_CANDIDATE_WIDE_LATIN:
                m_preedit.select_candidate (FCITX_ANTHY_CANDIDATE_HALF_KATAKANA);
                break;
            case FCITX_ANTHY_CANDIDATE_LATIN:
                m_preedit.select_candidate (FCITX_ANTHY_CANDIDATE_WIDE_LATIN);
                break;
            default:
                m_preedit.select_candidate (FCITX_ANTHY_CANDIDATE_HIRAGANA);
                break;
            }
        }
    } else {
        m_preedit.finish ();
        m_preedit.convert (FCITX_ANTHY_CANDIDATE_HIRAGANA, true);
    }

    set_preedition ();

    return true;
}

bool
AnthyInstance::action_reconvert (void)
{
    if (m_preedit.is_preediting ())
        return false;

    return true;
}

bool
AnthyInstance::action_add_word (void)
{
    util_launch_program (m_config.m_add_word_command);

    return true;
}

bool
AnthyInstance::action_launch_dict_admin_tool (void)
{
    util_launch_program (m_config.m_dict_admin_command);

    return true;
}

#if 0
void
AnthyInstance::action_regist_word (void)
{
}
#endif

TypingMethod
AnthyInstance::get_typing_method (void)
{
    return m_preedit.get_typing_method ();
}

InputMode
AnthyInstance::get_input_mode (void)
{
    return m_preedit.get_input_mode ();
}

bool
AnthyInstance::is_single_segment (void)
{
    if (m_config.m_conversion_mode == FCITX_ANTHY_CONVERSION_SINGLE_SEGMENT ||
        m_config.m_conversion_mode == FCITX_ANTHY_CONVERSION_SINGLE_SEGMENT_IMMEDIATE)
        return true;
    else
        return false;
}

bool
AnthyInstance::is_realtime_conversion (void)
{
    if (m_config.m_conversion_mode == FCITX_ANTHY_CONVERSION_MULTI_SEGMENT_IMMEDIATE ||
        m_config.m_conversion_mode == FCITX_ANTHY_CONVERSION_SINGLE_SEGMENT_IMMEDIATE)
        return true;
    else
        return false;
}

int
AnthyInstance::get_pseudo_ascii_mode (void)
{
    int retval = 0;
    TypingMethod m = get_typing_method ();

    if (m == FCITX_ANTHY_TYPING_METHOD_ROMAJI) {
        if (m_config.m_romaji_pseudo_ascii_mode)
            retval |= FCITX_ANTHY_PSEUDO_ASCII_TRIGGERED_CAPITALIZED;
    }

    return retval;
}

void AnthyInstance::commit_string(std::string str)
{
    FcitxInputContext* ic = FcitxInstanceGetCurrentIC(m_owner);
    FcitxInstanceCommitString(m_owner, ic, str.c_str());
}

#define APPEND_ACTION(key, func)                                               \
{                                                                              \
    FcitxHotkey* hk = NULL;                                                    \
    std::string name = #key;                                                   \
    if (loaded) {                                                              \
        std::string str = (ACTION_CONFIG_##key##_KEY);                         \
        std::string keystr;                                                    \
        style.get_string (keystr, "KeyBindings", str);                         \
        FcitxHotkeySetKey(keystr.c_str(), m_config.m_key_profile.m_hk_##key);  \
        hk = m_config.m_key_profile.m_hk_##key;                                \
    } else                                                                     \
        hk = m_config.m_key_default.m_hk_##key;                                \
    PMF f;                                                                     \
    f = &AnthyInstance::func;                                                  \
    m_actions[name] = Action (name, hk, f);                                    \
}

CONFIG_DESC_DEFINE(GetFcitxAnthyConfigDesc, "fcitx-anthy.desc")

CONFIG_BINDING_BEGIN(FcitxAnthyConfig)
CONFIG_BINDING_REGISTER("General", "InputMode", m_input_mode)
CONFIG_BINDING_REGISTER("General", "TypingMethod", m_typing_method)
CONFIG_BINDING_REGISTER("General", "ConversionMode", m_conversion_mode)
CONFIG_BINDING_REGISTER("General", "PeriodStyle", m_period_comma_style)
CONFIG_BINDING_REGISTER("General", "SymbolStyle", m_symbol_style)
CONFIG_BINDING_REGISTER("General", "PageSize", m_page_size)
CONFIG_BINDING_REGISTER("General", "LearnOnManualCommit", m_learn_on_manual_commit)
CONFIG_BINDING_REGISTER("General", "LearnOnAutoCommit", m_learn_on_auto_commit)
CONFIG_BINDING_REGISTER("General", "AllowSplit", m_romaji_allow_split)
CONFIG_BINDING_REGISTER("General", "UseDirectKeyOnPredict", m_use_direct_key_on_predict)
CONFIG_BINDING_REGISTER("General", "NTriggersToShowCandWin", m_n_triggers_to_show_cand_win)
CONFIG_BINDING_REGISTER("General", "ShowCandidatesLabel", m_show_candidates_label)

CONFIG_BINDING_REGISTER("Interface", "ShowInputMode", m_show_input_mode_label)
CONFIG_BINDING_REGISTER("Interface", "ShowTypingMethod", m_show_typing_method_label)
CONFIG_BINDING_REGISTER("Interface", "ShowConversionMode", m_show_conv_mode_label)
CONFIG_BINDING_REGISTER("Interface", "ShowPeriodStyle", m_show_period_style_label)
CONFIG_BINDING_REGISTER("Interface", "ShowSymbolStyle", m_show_symbol_style_label)

CONFIG_BINDING_REGISTER("KeyProfile", "KeyBindingProfile", m_key_profile_enum)
CONFIG_BINDING_REGISTER("KeyProfile", "RomajiTable", m_romaji_table_enum)
CONFIG_BINDING_REGISTER("KeyProfile", "KanaTable", m_kana_table_enum)
CONFIG_BINDING_REGISTER("KeyProfile", "NicolaTable", m_nicola_table_enum)
CONFIG_BINDING_REGISTER("KeyProfile", "CustomKeyBindingProfile", m_key_theme_file)
CONFIG_BINDING_REGISTER("KeyProfile", "CustomRomajiTable", m_romaji_fundamental_table)
CONFIG_BINDING_REGISTER("KeyProfile", "CustomKanaTable", m_kana_fundamental_table)
CONFIG_BINDING_REGISTER("KeyProfile", "CustomNicolaTable", m_nicola_fundamental_table)

CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CIRCLE_INPUT_MODE_KEY, m_key_default.m_hk_CIRCLE_INPUT_MODE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CIRCLE_KANA_MODE_KEY, m_key_default.m_hk_CIRCLE_KANA_MODE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CIRCLE_TYPING_METHOD_KEY, m_key_default.m_hk_CIRCLE_TYPING_METHOD)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_LATIN_MODE_KEY, m_key_default.m_hk_LATIN_MODE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_WIDE_LATIN_MODE_KEY, m_key_default.m_hk_WIDE_LATIN_MODE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_HIRAGANA_MODE_KEY, m_key_default.m_hk_HIRAGANA_MODE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_KATAKANA_MODE_KEY, m_key_default.m_hk_KATAKANA_MODE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_HALF_KATAKANA_MODE_KEY, m_key_default.m_hk_HALF_KATAKANA_MODE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CANCEL_PSEUDO_ASCII_MODE_KEY, m_key_default.m_hk_CANCEL_PSEUDO_ASCII_MODE)

CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_INSERT_SPACE_KEY, m_key_default.m_hk_INSERT_SPACE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_INSERT_ALT_SPACE_KEY, m_key_default.m_hk_INSERT_ALT_SPACE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_INSERT_HALF_SPACE_KEY, m_key_default.m_hk_INSERT_HALF_SPACE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_INSERT_WIDE_SPACE_KEY, m_key_default.m_hk_INSERT_WIDE_SPACE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_BACKSPACE_KEY, m_key_default.m_hk_BACKSPACE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_DELETE_KEY, m_key_default.m_hk_DELETE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_COMMIT_KEY, m_key_default.m_hk_COMMIT)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_COMMIT_REVERSE_LEARN_KEY, m_key_default.m_hk_COMMIT_REVERSE_LEARN)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CONVERT_KEY, m_key_default.m_hk_CONVERT)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_PREDICT_KEY, m_key_default.m_hk_PREDICT)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CANCEL_KEY, m_key_default.m_hk_CANCEL)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CANCEL_ALL_KEY, m_key_default.m_hk_CANCEL_ALL)

CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_MOVE_CARET_FIRST_KEY, m_key_default.m_hk_MOVE_CARET_FIRST)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_MOVE_CARET_LAST_KEY, m_key_default.m_hk_MOVE_CARET_LAST)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_MOVE_CARET_FORWARD_KEY, m_key_default.m_hk_MOVE_CARET_FORWARD)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_MOVE_CARET_BACKWARD_KEY, m_key_default.m_hk_MOVE_CARET_BACKWARD)

CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_SELECT_FIRST_SEGMENT_KEY, m_key_default.m_hk_SELECT_FIRST_SEGMENT)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_SELECT_LAST_SEGMENT_KEY, m_key_default.m_hk_SELECT_LAST_SEGMENT)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_SELECT_NEXT_SEGMENT_KEY, m_key_default.m_hk_SELECT_NEXT_SEGMENT)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_SELECT_PREV_SEGMENT_KEY, m_key_default.m_hk_SELECT_PREV_SEGMENT)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_SHRINK_SEGMENT_KEY, m_key_default.m_hk_SHRINK_SEGMENT)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_EXPAND_SEGMENT_KEY, m_key_default.m_hk_EXPAND_SEGMENT)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_COMMIT_FIRST_SEGMENT_KEY, m_key_default.m_hk_COMMIT_FIRST_SEGMENT)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_COMMIT_SELECTED_SEGMENT_KEY, m_key_default.m_hk_COMMIT_SELECTED_SEGMENT)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_COMMIT_FIRST_SEGMENT_REVERSE_LEARN_KEY, m_key_default.m_hk_COMMIT_FIRST_SEGMENT_REVERSE_LEARN)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_COMMIT_SELECTED_SEGMENT_REVERSE_LEARN_KEY, m_key_default.m_hk_COMMIT_SELECTED_SEGMENT_REVERSE_LEARN)

CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_SELECT_FIRST_CANDIDATE_KEY, m_key_default.m_hk_SELECT_FIRST_CANDIDATE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_SELECT_LAST_CANDIDATE_KEY, m_key_default.m_hk_SELECT_LAST_CANDIDATE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_SELECT_NEXT_CANDIDATE_KEY, m_key_default.m_hk_SELECT_NEXT_CANDIDATE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_SELECT_PREV_CANDIDATE_KEY, m_key_default.m_hk_SELECT_PREV_CANDIDATE)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CANDIDATES_PAGE_UP_KEY, m_key_default.m_hk_CANDIDATES_PAGE_UP)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CANDIDATES_PAGE_DOWN_KEY, m_key_default.m_hk_CANDIDATES_PAGE_DOWN)

CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CONV_CHAR_TYPE_FORWARD_KEY, m_key_default.m_hk_CONV_CHAR_TYPE_FORWARD)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CONV_CHAR_TYPE_BACKWARD_KEY, m_key_default.m_hk_CONV_CHAR_TYPE_BACKWARD)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CONV_TO_HIRAGANA_KEY, m_key_default.m_hk_CONV_TO_HIRAGANA)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CONV_TO_KATAKANA_KEY, m_key_default.m_hk_CONV_TO_KATAKANA)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CONV_TO_HALF_KEY, m_key_default.m_hk_CONV_TO_HALF)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CONV_TO_HALF_KATAKANA_KEY, m_key_default.m_hk_CONV_TO_HALF_KATAKANA)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CONV_TO_WIDE_LATIN_KEY, m_key_default.m_hk_CONV_TO_WIDE_LATIN)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_CONV_TO_LATIN_KEY, m_key_default.m_hk_CONV_TO_LATIN)

CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_RECONVERT_KEY, m_key_default.m_hk_RECONVERT)

CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_DICT_ADMIN_KEY, m_key_default.m_hk_DICT_ADMIN)
CONFIG_BINDING_REGISTER("Key", ACTION_CONFIG_ADD_WORD_KEY, m_key_default.m_hk_ADD_WORD)
CONFIG_BINDING_REGISTER("Key", "LeftThumbKey", m_left_thumb_keys)
CONFIG_BINDING_REGISTER("Key", "RightThumbKey", m_right_thumb_keys)
CONFIG_BINDING_REGISTER("Key", "KanaLayoutRoKey", m_kana_layout_ro_key)
CONFIG_BINDING_REGISTER("Key", "NicolaTime", m_nicola_time)

CONFIG_BINDING_REGISTER("Command", "AddWord", m_add_word_command)
CONFIG_BINDING_REGISTER("Command", "DictAdmin", m_dict_admin_command)
CONFIG_BINDING_END()

std::string AnthyInstance::get_key_profile()
{
    const char* key_profile[] = {
        "",
        "atok.sty",
        "canna.sty",
        "msime.sty",
        "vje-delta.sty"
        "wnn.sty",
        m_config.m_key_theme_file
    };

    if (m_config.m_key_profile_enum >= ARRAY_LEN(key_profile))
        m_config.m_key_profile_enum = 0;

    return key_profile[m_config.m_key_profile_enum];
}


std::string AnthyInstance::get_romaji_table()
{
    const char* key_profile[] = {
        "",
        "atok.sty",
        "azik.sty",
        "canna.sty",
        "msime.sty",
        "vje-delta.sty",
        "wnn.sty",
        m_config.m_romaji_fundamental_table
    };

    if (m_config.m_romaji_table_enum >= ARRAY_LEN(key_profile))
        m_config.m_romaji_table_enum = 0;

    return key_profile[m_config.m_romaji_table_enum];
}


std::string AnthyInstance::get_kana_table()
{
    const char* key_profile[] = {
        "",
        "101kana.sty",
        "tsuki-2-203-101.sty",
        "tsuki-2-203-106.sty",
        m_config.m_kana_fundamental_table
    };

    if (m_config.m_kana_table_enum >= ARRAY_LEN(key_profile))
        m_config.m_kana_table_enum = 0;

    return key_profile[m_config.m_kana_table_enum];
}


std::string AnthyInstance::get_nicola_table()
{
    const char* key_profile[] = {
        "",
        "nicola-a.sty",
        "nicola-f.sty",
        "nicola-j.sty",
        "oasys100j.sty"
        "tron-dvorak.sty",
        "tron-qwerty-jp.sty",
        m_config.m_nicola_fundamental_table
    };

    if (m_config.m_nicola_table_enum >= ARRAY_LEN(key_profile))
        m_config.m_nicola_table_enum = 0;

    return key_profile[m_config.m_nicola_table_enum];
}

bool AnthyInstance::load_config()
{
    FcitxConfigFileDesc *configDesc = GetFcitxAnthyConfigDesc();
    if (!configDesc)
        return false;

    FILE *fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-anthy.config", "r", NULL);

    if (!fp) {
        if (errno == ENOENT)
            save_config();
    }
    FcitxConfigFile *cfile = FcitxConfigParseConfigFileFp(fp, configDesc);

    FcitxAnthyConfigConfigBind(&m_config, cfile, configDesc);
    FcitxConfigBindSync(&m_config.gconfig);

    if (fp)
        fclose(fp);

    configure();
    return true;
}

void AnthyInstance::save_config()
{
    FcitxConfigFileDesc *configDesc = GetFcitxAnthyConfigDesc();
    FILE *fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-anthy.config", "w", NULL);
    FcitxConfigSaveConfigFileFp(fp, &m_config.gconfig, configDesc);
    if (fp)
        fclose(fp);
}

char* AnthyInstance::get_file_name(const std::string& name)
{
    char* retFile = NULL;
    FILE* fp = FcitxXDGGetFileWithPrefix("anthy", name.c_str(), "r", &retFile);
    if (fp) {
        fclose(fp);
    }
    return retFile;
}

void AnthyInstance::configure()
{
    StyleFile style;
    std::string file;
    bool loaded = false;

    // load key bindings
    char* filename = get_file_name(get_key_profile());
    if (filename)
        loaded = style.load (filename);

    fcitx_utils_free(filename);

    // clear old actions
    m_actions.clear ();
    // convert key
    APPEND_ACTION (CONVERT,                 action_convert);
    APPEND_ACTION (PREDICT,                 action_predict);

    // candidates keys
    APPEND_ACTION (CANDIDATES_PAGE_UP,      action_candidates_page_up);
    APPEND_ACTION (CANDIDATES_PAGE_DOWN,    action_candidates_page_down);
    APPEND_ACTION (SELECT_FIRST_CANDIDATE,  action_select_first_candidate);
    APPEND_ACTION (SELECT_LAST_CANDIDATE,   action_select_last_candidate);
    APPEND_ACTION (SELECT_NEXT_CANDIDATE,   action_select_next_candidate);
    APPEND_ACTION (SELECT_PREV_CANDIDATE,   action_select_prev_candidate);

    // segment keys
    APPEND_ACTION (SELECT_FIRST_SEGMENT,    action_select_first_segment);
    APPEND_ACTION (SELECT_LAST_SEGMENT,     action_select_last_segment);
    APPEND_ACTION (SELECT_NEXT_SEGMENT,     action_select_next_segment);
    APPEND_ACTION (SELECT_PREV_SEGMENT,     action_select_prev_segment);
    APPEND_ACTION (SHRINK_SEGMENT,          action_shrink_segment);
    APPEND_ACTION (EXPAND_SEGMENT,          action_expand_segment);
    APPEND_ACTION (COMMIT_FIRST_SEGMENT,    action_commit_first_segment);
    APPEND_ACTION (COMMIT_SELECTED_SEGMENT, action_commit_selected_segment);
    APPEND_ACTION (COMMIT_FIRST_SEGMENT_REVERSE_LEARN,
                   action_commit_first_segment_reverse_preference);
    APPEND_ACTION (COMMIT_SELECTED_SEGMENT_REVERSE_LEARN,
                   action_commit_selected_segment_reverse_preference);

    // direct convert keys
    APPEND_ACTION (CONV_CHAR_TYPE_FORWARD,  action_convert_char_type_forward);
    APPEND_ACTION (CONV_CHAR_TYPE_BACKWARD, action_convert_char_type_backward);
    APPEND_ACTION (CONV_TO_HIRAGANA,        action_convert_to_hiragana);
    APPEND_ACTION (CONV_TO_KATAKANA,        action_convert_to_katakana);
    APPEND_ACTION (CONV_TO_HALF,            action_convert_to_half);
    APPEND_ACTION (CONV_TO_HALF_KATAKANA,   action_convert_to_half_katakana);
    APPEND_ACTION (CONV_TO_LATIN,           action_convert_to_latin);
    APPEND_ACTION (CONV_TO_WIDE_LATIN,      action_convert_to_wide_latin);

    // pseudo ascii mode
    APPEND_ACTION (CANCEL_PSEUDO_ASCII_MODE,action_cancel_pseudo_ascii_mode);

    // caret keys
    APPEND_ACTION (MOVE_CARET_FIRST,        action_move_caret_first);
    APPEND_ACTION (MOVE_CARET_LAST,         action_move_caret_last);
    APPEND_ACTION (MOVE_CARET_FORWARD,      action_move_caret_forward);
    APPEND_ACTION (MOVE_CARET_BACKWARD,     action_move_caret_backward);

    // edit keys
    APPEND_ACTION (BACKSPACE,               action_back);
    APPEND_ACTION (DELETE,                  action_delete);
    APPEND_ACTION (COMMIT,                  action_commit_follow_preference);
    APPEND_ACTION (COMMIT_REVERSE_LEARN,    action_commit_reverse_preference);
    APPEND_ACTION (CANCEL,                  action_revert);
    APPEND_ACTION (CANCEL_ALL,              action_cancel_all);
    APPEND_ACTION (INSERT_SPACE,            action_insert_space);
    APPEND_ACTION (INSERT_ALT_SPACE,        action_insert_alternative_space);
    APPEND_ACTION (INSERT_HALF_SPACE,       action_insert_half_space);
    APPEND_ACTION (INSERT_WIDE_SPACE,       action_insert_wide_space);

    // mode keys
    APPEND_ACTION (CIRCLE_INPUT_MODE,       action_circle_input_mode);
    APPEND_ACTION (CIRCLE_KANA_MODE,        action_circle_kana_mode);
    APPEND_ACTION (CIRCLE_TYPING_METHOD,    action_circle_typing_method);
    APPEND_ACTION (LATIN_MODE,              action_latin_mode);
    APPEND_ACTION (WIDE_LATIN_MODE,         action_wide_latin_mode);
    APPEND_ACTION (HIRAGANA_MODE,           action_hiragana_mode);
    APPEND_ACTION (KATAKANA_MODE,           action_katakana_mode);
    APPEND_ACTION (HALF_KATAKANA_MODE,      action_half_katakana_mode);

    // dict keys
    APPEND_ACTION (DICT_ADMIN,              action_launch_dict_admin_tool);
    APPEND_ACTION (ADD_WORD,                action_add_word);

    // reconvert
    APPEND_ACTION (RECONVERT,               action_reconvert);

    // load custom romaji table
    if (m_config.m_custom_romaji_table) {
        delete m_config.m_custom_romaji_table;
        m_config.m_custom_romaji_table = NULL;
    }
    const char *section_romaji = "RomajiTable/FundamentalTable";
    filename = get_file_name(get_romaji_table());
    if (filename && style.load (filename)) {
        m_config.m_custom_romaji_table = style.get_key2kana_table (section_romaji);
    }
    fcitx_utils_free(filename);

    // load custom kana table
    if (m_config.m_custom_kana_table) {
        delete m_config.m_custom_kana_table;
        m_config.m_custom_kana_table = NULL;
    }
    const char *section_kana = "KanaTable/FundamentalTable";
    filename = get_file_name(get_kana_table());
    if (filename && style.load (filename)) {
        m_config.m_custom_kana_table = style.get_key2kana_table (section_kana);
    }
    fcitx_utils_free(filename);

    // load custom NICOLA table
    if (m_config.m_custom_nicola_table) {
        delete m_config.m_custom_nicola_table;
        m_config.m_custom_nicola_table = NULL;
    }
    const char *section_nicola = "NICOLATable/FundamentalTable";
    filename = get_file_name(get_nicola_table());
    if (filename && style.load (filename)) {
        m_config.m_custom_nicola_table = style.get_key2kana_table (section_nicola);
    }
    fcitx_utils_free(filename);

    // set romaji settings
    m_preedit.set_symbol_width (m_config.m_romaji_half_symbol);
    m_preedit.set_number_width (m_config.m_romaji_half_number);

    // set input mode
    m_preedit.set_input_mode (m_config.m_input_mode);

    // set typing method and pseudo ASCII mode
    m_preedit.set_typing_method (m_config.m_typing_method);
    m_preedit.set_pseudo_ascii_mode (get_pseudo_ascii_mode ());

    // set period style
    set_period_style(m_config.m_period_comma_style);

    // set symbol style
    set_symbol_style(m_config.m_symbol_style);

    // setup toolbar
    install_properties ();
}

void AnthyInstance::update_aux_string(const std::string& str)
{
    FcitxMessages* aux;
    aux = m_aux_up;
    FcitxMessagesSetMessageCount(aux, 0);
    FcitxMessagesAddMessageAtLast(aux, MSG_TIPS, "%s", str.c_str());
    m_ui_update = true;
}

void AnthyInstance::reset_cursor(int cursor)
{
    if (cursor >= 0)
        m_cursor_pos = cursor;
    else
        cursor = 0;
}

void AnthyInstance::auto_commit(FcitxIMCloseEventType type)
{
    if (type == CET_LostFocus) {
        action_commit(m_config.m_learn_on_auto_commit);
    } else if (type == CET_ChangeByUser) {
        reset_im();
    } else if (type == CET_ChangeByInactivate) {
        FcitxGlobalConfig* config = FcitxInstanceGetGlobalConfig(m_owner);
        if (config->bSendTextWhenSwitchEng)
            action_commit(m_config.m_learn_on_manual_commit);
        else
            reset_im();
    }
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
