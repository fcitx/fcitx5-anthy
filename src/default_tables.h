/*
 * SPDX-FileCopyrightText: 2004-2005 Takuro Ashie
 * SPDX-FileCopyrightText: 2012-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_ANTHY_DEFAULT_TABLES_H_
#define _FCITX5_ANTHY_DEFAULT_TABLES_H_

#include <array>
#include <string_view>
struct ConvRule {
    std::string_view string;
    std::string_view result;
    std::string_view cont;
};

struct HiraganaKatakanaRule {
    const char *hiragana;
    const char *katakana;
    const char *half_katakana;
};

struct WideRule {
    std::string_view code;
    std::string_view wide;
};

struct KeyCodeToCharRule {
    unsigned int code;
    const char *kana;
};

struct VoicedConsonantRule {
    std::string_view string;
    std::string_view voiced;
    std::string_view half_voiced;
};

struct NicolaRule {
    const char *key;
    const char *single;
    const char *left_shift;
    const char *right_shift;
};

// fundamental table
extern const std::array<ConvRule, 286> fcitx_anthy_romaji_typing_rule;
extern const std::array<ConvRule, 18> fcitx_anthy_romaji_double_consonant_rule;
extern const std::array<ConvRule, 89> fcitx_anthy_kana_typing_rule;
extern const std::array<ConvRule, 45> fcitx_anthy_kana_voiced_consonant_rule;

// symbol & number
extern const std::array<ConvRule, 32> fcitx_anthy_half_symbol_rule;
extern const std::array<ConvRule, 32> fcitx_anthy_wide_symbol_rule;
extern const std::array<ConvRule, 10> fcitx_anthy_half_number_rule;
extern const std::array<ConvRule, 10> fcitx_anthy_wide_number_rule;

// period table
extern const std::array<ConvRule, 1> fcitx_anthy_romaji_ja_period_rule;
extern const std::array<ConvRule, 1> fcitx_anthy_romaji_wide_period_rule;
extern const std::array<ConvRule, 1> fcitx_anthy_romaji_half_period_rule;

extern const std::array<ConvRule, 1> fcitx_anthy_kana_ja_period_rule;
extern const std::array<ConvRule, 1> fcitx_anthy_kana_wide_period_rule;
extern const std::array<ConvRule, 1> fcitx_anthy_kana_half_period_rule;

// comma table
extern const std::array<ConvRule, 1> fcitx_anthy_romaji_ja_comma_rule;
extern const std::array<ConvRule, 1> fcitx_anthy_romaji_wide_comma_rule;
extern const std::array<ConvRule, 1> fcitx_anthy_romaji_half_comma_rule;

extern const std::array<ConvRule, 1> fcitx_anthy_kana_ja_comma_rule;
extern const std::array<ConvRule, 1> fcitx_anthy_kana_wide_comma_rule;
extern const std::array<ConvRule, 1> fcitx_anthy_kana_half_comma_rule;

// bracket table
extern const std::array<ConvRule, 2> fcitx_anthy_romaji_ja_bracket_rule;
extern const std::array<ConvRule, 2> fcitx_anthy_romaji_wide_bracket_rule;

extern const std::array<ConvRule, 2> fcitx_anthy_kana_ja_bracket_rule;
extern const std::array<ConvRule, 2> fcitx_anthy_kana_wide_bracket_rule;

// slash table
extern const std::array<ConvRule, 1> fcitx_anthy_romaji_ja_slash_rule;
extern const std::array<ConvRule, 1> fcitx_anthy_romaji_wide_slash_rule;

extern const std::array<ConvRule, 1> fcitx_anthy_kana_ja_slash_rule;
extern const std::array<ConvRule, 1> fcitx_anthy_kana_wide_slash_rule;

// misc
extern const std::array<HiraganaKatakanaRule, 118>
    fcitx_anthy_hiragana_katakana_table;
extern const std::array<WideRule, 94> fcitx_anthy_wide_table;
extern const std::array<VoicedConsonantRule, 20>
    fcitx_anthy_voiced_consonant_table;

// key code
extern KeyCodeToCharRule fcitx_anthy_keypad_table[];
extern KeyCodeToCharRule fcitx_anthy_kana_table[];

// nicola
extern NicolaRule fcitx_anthy_nicola_table[];

#endif // _FCITX5_ANTHY_DEFAULT_TABLES_H_
