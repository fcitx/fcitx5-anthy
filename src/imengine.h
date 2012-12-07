/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
 *  Copyright (C) 2004 Takuro Ashie
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

#ifndef __FCITX_ANTHY_IMENGINE_H__
#define __FCITX_ANTHY_IMENGINE_H__

#include <map>
#include <anthy/anthy.h>
#include "preedit.h"
#include "key2kana_table.h"
#include "factory.h"


struct AnthyStatus {
    const char* icon;
    const char* label;
    const char* description;
};

class AnthyInstance
{
public:
    AnthyInstance (FcitxInstance* instance);
    ~AnthyInstance ();
    bool load_config();
    void save_config();

    void configure();

    bool process_key_event            (const KeyEvent& key);
    void move_preedit_caret           (unsigned int pos);
    void select_candidate             (unsigned int item);
    void update_lookup_table_page_size(unsigned int page_size);
    void reset_im                     (void);
    void reset                        (void);
    void init                         (void);

public:
    /* actions */
    bool   action_convert                     (void);
    bool   action_predict                     (void);
    bool   action_revert                      (void);
    bool   action_cancel_all                  (void);
    bool   action_commit_follow_preference    (void);
    bool   action_commit_reverse_preference   (void);
    bool   action_commit_first_segment        (void);
    bool   action_commit_selected_segment     (void);
    bool   action_commit_first_segment_reverse_preference
                                              (void);
    bool   action_commit_selected_segment_reverse_preference
                                              (void);
    bool   action_back                        (void);
    bool   action_delete                      (void);
    bool   action_insert_space                (void);
    bool   action_insert_alternative_space    (void);
    bool   action_insert_half_space           (void);
    bool   action_insert_wide_space           (void);

    bool   action_move_caret_backward         (void);
    bool   action_move_caret_forward          (void);
    bool   action_move_caret_first            (void);
    bool   action_move_caret_last             (void);

    bool   action_select_prev_segment         (void);
    bool   action_select_next_segment         (void);
    bool   action_select_first_segment        (void);
    bool   action_select_last_segment         (void);
    bool   action_shrink_segment              (void);
    bool   action_expand_segment              (void);

    bool   action_select_first_candidate      (void);
    bool   action_select_last_candidate       (void);
    bool   action_select_next_candidate       (void);
    bool   action_select_prev_candidate       (void);
    bool   action_candidates_page_up          (void);
    bool   action_candidates_page_down        (void);

    bool   action_convert_to_hiragana         (void);
    bool   action_convert_to_katakana         (void);
    bool   action_convert_to_half             (void);
    bool   action_convert_to_half_katakana    (void);
    bool   action_convert_to_latin            (void);
    bool   action_convert_to_wide_latin       (void);
    bool   action_convert_char_type_forward   (void);
    bool   action_convert_char_type_backward  (void);
    bool   action_reconvert                   (void);

    bool   action_circle_input_mode           (void);
    bool   action_circle_kana_mode            (void);
    bool   action_circle_typing_method        (void);

    bool   action_latin_mode                  (void);
    bool   action_wide_latin_mode             (void);
    bool   action_hiragana_mode               (void);
    bool   action_katakana_mode               (void);
    bool   action_half_katakana_mode          (void);
    bool   action_cancel_pseudo_ascii_mode    (void);

    bool   action_add_word                    (void);
    bool   action_launch_dict_admin_tool      (void);
    /*
    void   actoin_register_word               (void);
    */

public:
    TypingMethod
           get_typing_method                  (void);
    InputMode
           get_input_mode                     (void);
    ConversionMode
           get_conversion_mode                (void) { return m_config.m_conversion_mode; }
    PeriodCommaStyle
           get_period_style                   (void) { return m_config.m_period_comma_style; }
    SymbolStyle
           get_symbol_style                   (void) { return m_config.m_symbol_style; }
    void   set_input_mode                     (InputMode      mode);
    void   set_conversion_mode                (ConversionMode mode);
    void   set_typing_method                  (TypingMethod   method);
    void   set_period_style                   (PeriodCommaStyle period);
    void   set_symbol_style                   (SymbolStyle symbol);
    void   update_ui                          (void);

    int    get_pseudo_ascii_mode              (void);
    FcitxAnthyConfig* get_config() { return &m_config; }
    FcitxInstance* get_owner()     { return m_owner; }
    FcitxMessages* get_preedit()   { return m_preedit_msg; }
    FcitxMessages* get_client_preedit()   { return m_client_preedit_msg; }
    FcitxInputState* get_input()   { return m_input; }
    FcitxProfile* get_profile()   { return m_profile; }
    bool support_client_preedit();

    const char* get_input_mode_icon();
    const char* get_typing_method_icon();
    const char* get_conversion_mode_icon();
    const char* get_period_style_icon();
    const char* get_symbol_style_icon();

    bool   action_select_candidate            (unsigned int   i);
    void   reset_cursor(int cursor);
    void   auto_commit(FcitxIMCloseEventType type);
private:
    /* processing key event */
    bool   process_key_event_input            (const KeyEvent &key);
    bool   process_key_event_lookup_keybind   (const KeyEvent &key);
    bool   process_key_event_latin_mode       (const KeyEvent &key);
    bool   process_key_event_wide_latin_mode  (const KeyEvent &key);

    /* utility */
    void   set_preedition                     (void);
    void   set_aux_string                     (void);
    int    set_lookup_table                   (void);
    void   unset_lookup_table                 (void);
    void   install_properties                 (void);
    void   set_period_style                   (PeriodStyle    period,
                                               CommaStyle     comma);
    void   set_symbol_style                   (BracketStyle   bracket,
                                               SlashStyle     slash);
    bool   is_selecting_candidates            (void);
    void   select_candidate_no_direct         (unsigned int   item);
    bool   convert_kana                       (CandidateType  type);

    bool   action_commit                      (bool           learn, bool do_real_commit = true);

    bool   is_single_segment                  (void);
    bool   is_realtime_conversion             (void);
    char*  get_file_name(const std::string& name);

private: // FIXME!
    bool   is_nicola_thumb_shift_key          (const KeyEvent &key);
    void   commit_string(std::string str);
    void   update_aux_string(const std::string& str);
    std::string get_key_profile();
    std::string get_romaji_table();
    std::string get_kana_table();
    std::string get_nicola_table();

private:
    FcitxInstance* m_owner;

   /* for preedit */
    Preedit               m_preedit;
    bool                  m_preedit_string_visible;

    FcitxInputState* m_input;

    /* for candidates window */
    FcitxCandidateWordList*     m_lookup_table;
    bool                        m_lookup_table_visible;
    unsigned int                m_n_conv_key_pressed;

    /* for toggling latin and wide latin */
    InputMode             m_prev_input_mode;

    /* for action */
    KeyEvent              m_last_key;

    FcitxAnthyConfig m_config;
    FcitxMessages* m_aux_up;
    FcitxMessages* m_aux_down;

    int m_cursor_pos;
    FcitxMessages* m_client_preedit_msg;
    FcitxMessages* m_preedit_msg;

    std::map<std::string, Action> m_actions;
    FcitxProfile* m_profile;
    bool m_status_installed;

    FcitxUIMenu m_input_mode_menu;
    FcitxUIMenu m_typing_method_menu;
    FcitxUIMenu m_conversion_mode_menu;
    FcitxUIMenu m_period_style_menu;
    FcitxUIMenu m_symbol_style_menu;
    int m_ui_update;
};
#endif /* __FCITX_ANTHY_IMENGINE_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
