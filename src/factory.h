/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
 *  Copyright (C) 2004 Takuro Ashie
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * The original code is scim_uim_imengine.cpp in scim-uim-0.1.3.
 * Copyright (C) 2004 James Su <suzhe@tsinghua.org.cn>
 */

#ifndef __FCITX_ANTHY_FACTORY_H__
#define __FCITX_ANTHY_FACTORY_H__

#include <anthy/anthy.h>
#include <fcitx/instance.h>
#include "action.h"
#include "style_file.h"

class Key2KanaTable;

typedef enum {
    FCITX_ANTHY_MODE_HIRAGANA,
    FCITX_ANTHY_MODE_KATAKANA,
    FCITX_ANTHY_MODE_HALF_KATAKANA,
    FCITX_ANTHY_MODE_LATIN,
    FCITX_ANTHY_MODE_WIDE_LATIN,
    FCITX_ANTHY_MODE_LAST
} InputMode;

typedef enum {
    FCITX_ANTHY_SYMBOL_STYLE_JAPANESE,
    FCITX_ANTHY_SYMBOL_STYLE_WIDEBRACKET_WIDESLASH,
    FCITX_ANTHY_SYMBOL_STYLE_CORNERBRACKET_WIDESLASH,
    FCITX_ANTHY_SYMBOL_STYLE_WIDEBRACKET_MIDDLEDOT,
    FCITX_ANTHY_SYMBOL_STYLE_LAST
} SymbolStyle;

typedef enum {
    FCITX_ANTHY_PERIOD_COMMA_WIDELATIN,
    FCITX_ANTHY_PERIOD_COMMA_LATIN,
    FCITX_ANTHY_PERIOD_COMMA_JAPANESE,
    FCITX_ANTHY_PERIOD_COMMA_WIDELATIN_JAPANESE,
    FCITX_ANTHY_PERIOD_COMMA_LAST
} PeriodCommaStyle;

typedef enum {
    FCITX_ANTHY_CONVERSION_MULTI_SEGMENT,
    FCITX_ANTHY_CONVERSION_SINGLE_SEGMENT,
    FCITX_ANTHY_CONVERSION_MULTI_SEGMENT_IMMEDIATE,
    FCITX_ANTHY_CONVERSION_SINGLE_SEGMENT_IMMEDIATE,
    FCITX_ANTHY_CONVERSION_MODE_LAST
} ConversionMode;

typedef enum {
    FCITX_ANTHY_TYPING_METHOD_ROMAJI,
    FCITX_ANTHY_TYPING_METHOD_KANA,
    FCITX_ANTHY_TYPING_METHOD_NICOLA,
    FCITX_ANTHY_TYPING_METHOD_CUSTOM,
    FCITX_ANTHY_TYPING_METHOD_LAST
} TypingMethod;

typedef enum {
    FCITX_ANTHY_SPACE_TYPE_FOLLOWMODE,
    FCITX_ANTHY_SPACE_TYPE_WIDE
} SpaceType;

typedef enum {
    FCITX_ANTHY_TEN_KEY_TYPE_WIDE,
    FCITX_ANTHY_TEN_KEY_TYPE_HALF,
    FCITX_ANTHY_TEN_KEY_TYPE_FOLLOWMODE,
} TenKeyType;

typedef enum {
    FCITX_ANTHY_PERIOD_BEHAVIOR_CONVERT,
    FCITX_ANTHY_PERIOD_BEHAVIOR_COMMIT,
} PeriodBehavior;

struct AnthyKeyProfile {


    FcitxHotkey m_hk_CONVERT[2];
    FcitxHotkey m_hk_PREDICT[2];
    // candidates keys
    FcitxHotkey m_hk_CANDIDATES_PAGE_UP[2];
    FcitxHotkey m_hk_CANDIDATES_PAGE_DOWN[2];
    FcitxHotkey m_hk_SELECT_FIRST_CANDIDATE[2];
    FcitxHotkey m_hk_SELECT_LAST_CANDIDATE[2];
    FcitxHotkey m_hk_SELECT_NEXT_CANDIDATE[2];
    FcitxHotkey m_hk_SELECT_PREV_CANDIDATE[2];

    // segment keys
    FcitxHotkey m_hk_SELECT_FIRST_SEGMENT[2];
    FcitxHotkey m_hk_SELECT_LAST_SEGMENT[2];
    FcitxHotkey m_hk_SELECT_NEXT_SEGMENT[2];
    FcitxHotkey m_hk_SELECT_PREV_SEGMENT[2];
    FcitxHotkey m_hk_SHRINK_SEGMENT[2];
    FcitxHotkey m_hk_EXPAND_SEGMENT[2];
    FcitxHotkey m_hk_COMMIT_FIRST_SEGMENT[2];
    FcitxHotkey m_hk_COMMIT_SELECTED_SEGMENT[2];
    FcitxHotkey m_hk_COMMIT_FIRST_SEGMENT_REVERSE_LEARN[2];
    FcitxHotkey m_hk_COMMIT_SELECTED_SEGMENT_REVERSE_LEARN[2];

    // direct convert keys
    FcitxHotkey m_hk_CONV_CHAR_TYPE_FORWARD[2];
    FcitxHotkey m_hk_CONV_CHAR_TYPE_BACKWARD[2];
    FcitxHotkey m_hk_CONV_TO_HIRAGANA[2];
    FcitxHotkey m_hk_CONV_TO_KATAKANA[2];
    FcitxHotkey m_hk_CONV_TO_HALF[2];
    FcitxHotkey m_hk_CONV_TO_HALF_KATAKANA[2];
    FcitxHotkey m_hk_CONV_TO_LATIN[2];
    FcitxHotkey m_hk_CONV_TO_WIDE_LATIN[2];

    // pseudo ascii mode
    FcitxHotkey m_hk_CANCEL_PSEUDO_ASCII_MODE[2];

    // caret keys
    FcitxHotkey m_hk_MOVE_CARET_FIRST[2];
    FcitxHotkey m_hk_MOVE_CARET_LAST[2];
    FcitxHotkey m_hk_MOVE_CARET_FORWARD[2];
    FcitxHotkey m_hk_MOVE_CARET_BACKWARD[2];

    // edit keys
    FcitxHotkey m_hk_BACKSPACE[2];
    FcitxHotkey m_hk_DELETE[2];
    FcitxHotkey m_hk_COMMIT[2];
    FcitxHotkey m_hk_COMMIT_REVERSE_LEARN[2];
    FcitxHotkey m_hk_CANCEL[2];
    FcitxHotkey m_hk_CANCEL_ALL[2];
    FcitxHotkey m_hk_INSERT_SPACE[2];
    FcitxHotkey m_hk_INSERT_ALT_SPACE[2];
    FcitxHotkey m_hk_INSERT_HALF_SPACE[2];
    FcitxHotkey m_hk_INSERT_WIDE_SPACE[2];

    // mode keys
    FcitxHotkey m_hk_CIRCLE_INPUT_MODE[2];
    FcitxHotkey m_hk_CIRCLE_KANA_MODE[2];
    FcitxHotkey m_hk_CIRCLE_TYPING_METHOD[2];
    FcitxHotkey m_hk_LATIN_MODE[2];
    FcitxHotkey m_hk_WIDE_LATIN_MODE[2];
    FcitxHotkey m_hk_HIRAGANA_MODE[2];
    FcitxHotkey m_hk_KATAKANA_MODE[2];
    FcitxHotkey m_hk_HALF_KATAKANA_MODE[2];

    // dict keys
    FcitxHotkey m_hk_DICT_ADMIN[2];
    FcitxHotkey m_hk_ADD_WORD[2];

    // reconvert
    FcitxHotkey m_hk_RECONVERT[2];
};

struct FcitxAnthyConfig {
    FcitxGenericConfig gconfig;
    const char* romaji_fundamental_table;
    const char* kana_fundamental_table;
    const char* nicola_fundamental_table;
    const char* m_kana_layout_ro_key;
    boolean m_predict_on_input;
    boolean m_learn_on_auto_commit;
    boolean m_romaji_pseudo_ascii_blank_behavior;
    boolean m_romaji_pseudo_ascii_mode;
    boolean m_romaji_half_symbol;
    boolean m_romaji_half_number;
    boolean m_show_input_mode_label;
    boolean m_show_symbol_style_label;
    boolean m_show_period_style_label;
    boolean m_show_conv_mode_label;
    boolean m_show_typing_method_label;
    boolean m_learn_on_manual_commit;
    boolean m_show_add_word_label;
    boolean m_use_direct_key_on_predict;
    boolean m_show_candidates_label;
    boolean m_romaji_allow_split;

    int     m_nicola_time;

    SpaceType m_space_type;
    InputMode m_input_mode;
    SymbolStyle m_symbol_style;
    PeriodCommaStyle m_period_comma_style;
    ConversionMode m_conversion_mode;
    TypingMethod m_typing_method;
    TenKeyType m_ten_key_type;
    PeriodBehavior m_behavior_on_period;

    int m_cand_win_page_size;
    int m_n_triggers_to_show_cand_win;

    FcitxHotkey m_left_thumb_keys[2];
    FcitxHotkey m_right_thumb_keys[2];
    char* m_key_theme_file;

    int m_page_size;

    AnthyKeyProfile m_key_default;
    AnthyKeyProfile m_key_profile;

    Key2KanaTable *m_custom_romaji_table;
    Key2KanaTable *m_custom_kana_table;
    Key2KanaTable *m_custom_nicola_table;
};

#endif /* __FCITX_ANTHY_FACTORY_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
