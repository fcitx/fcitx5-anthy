/*
 * SPDX-FileCopyrightText: 2004-2005 Takuro Ashie
 * SPDX-FileCopyrightText: 2012-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_ANTHY_DEFAULT_TABLES_H_
#define _FCITX5_ANTHY_DEFAULT_TABLES_H_

struct ConvRule {
    const char *string;
    const char *result;
    const char *cont;
};

struct HiraganaKatakanaRule {
    const char *hiragana;
    const char *katakana;
    const char *half_katakana;
};

struct WideRule {
    const char *code;
    const char *wide;
};

struct KeyCodeToCharRule {
    unsigned int code;
    const char *kana;
};

struct VoicedConsonantRule {
    const char *string;
    const char *voiced;
    const char *half_voiced;
};

struct NicolaRule {
    const char *key;
    const char *single;
    const char *left_shift;
    const char *right_shift;
};

// fundamental table
extern ConvRule fcitx_anthy_romaji_typing_rule[];
extern ConvRule fcitx_anthy_romaji_double_consonant_rule[];
extern ConvRule fcitx_anthy_kana_typing_rule[];
extern ConvRule fcitx_anthy_kana_voiced_consonant_rule[];

// symbol & number
extern ConvRule fcitx_anthy_half_symbol_rule[];
extern ConvRule fcitx_anthy_wide_symbol_rule[];
extern ConvRule fcitx_anthy_half_number_rule[];
extern ConvRule fcitx_anthy_wide_number_rule[];

// period table
extern ConvRule fcitx_anthy_romaji_ja_period_rule[];
extern ConvRule fcitx_anthy_romaji_wide_period_rule[];
extern ConvRule fcitx_anthy_romaji_half_period_rule[];

extern ConvRule fcitx_anthy_kana_ja_period_rule[];
extern ConvRule fcitx_anthy_kana_wide_period_rule[];
extern ConvRule fcitx_anthy_kana_half_period_rule[];

// comma table
extern ConvRule fcitx_anthy_romaji_ja_comma_rule[];
extern ConvRule fcitx_anthy_romaji_wide_comma_rule[];
extern ConvRule fcitx_anthy_romaji_half_comma_rule[];

extern ConvRule fcitx_anthy_kana_ja_comma_rule[];
extern ConvRule fcitx_anthy_kana_wide_comma_rule[];
extern ConvRule fcitx_anthy_kana_half_comma_rule[];

// bracket table
extern ConvRule fcitx_anthy_romaji_ja_bracket_rule[];
extern ConvRule fcitx_anthy_romaji_wide_bracket_rule[];

extern ConvRule fcitx_anthy_kana_ja_bracket_rule[];
extern ConvRule fcitx_anthy_kana_wide_bracket_rule[];

// slash table
extern ConvRule fcitx_anthy_romaji_ja_slash_rule[];
extern ConvRule fcitx_anthy_romaji_wide_slash_rule[];

extern ConvRule fcitx_anthy_kana_ja_slash_rule[];
extern ConvRule fcitx_anthy_kana_wide_slash_rule[];

// misc
extern HiraganaKatakanaRule fcitx_anthy_hiragana_katakana_table[];
extern WideRule fcitx_anthy_wide_table[];
extern VoicedConsonantRule fcitx_anthy_voiced_consonant_table[];

// key code
extern KeyCodeToCharRule fcitx_anthy_keypad_table[];
extern KeyCodeToCharRule fcitx_anthy_kana_table[];

// nicola
extern NicolaRule fcitx_anthy_nicola_table[];

#endif // _FCITX5_ANTHY_DEFAULT_TABLES_H_
