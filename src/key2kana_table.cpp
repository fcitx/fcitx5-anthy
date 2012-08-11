/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie
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

#include "key2kana_table.h"

// fundamental table
static Key2KanaTable romaji_table (
     ("DefaultRomajiTable"),
    fcitx_anthy_romaji_typing_rule);
static Key2KanaTable romaji_double_consonant_table (
     ("DefaultRomajiDoubleConsonantTable"),
    fcitx_anthy_romaji_double_consonant_rule);
static Key2KanaTable kana_table (
     ("DefaultKanaTable"),
    fcitx_anthy_kana_typing_rule);
static Key2KanaTable kana_voiced_consonant_table (
     ("DefaultKanaVoicedConsonantTable"),
    fcitx_anthy_kana_voiced_consonant_rule);
static Key2KanaTable nicola_table (
     ("DefaultNICOLATable"),
    fcitx_anthy_nicola_table);

// symbols
static Key2KanaTable half_symbol_table (
     ("DefaultRomajiHalfSymbolTable"),
    fcitx_anthy_half_symbol_rule);
static Key2KanaTable wide_symbol_table (
     ("DefaultRomajiWideSymbolTable"),
    fcitx_anthy_wide_symbol_rule);

// numbers
static Key2KanaTable half_number_table (
     ("DefaultRomajiHalfNumberTable"),
    fcitx_anthy_half_number_rule);
static Key2KanaTable wide_number_table (
     ("DefaultRomajiWideNumberTable"),
    fcitx_anthy_wide_number_rule);

// period
static Key2KanaTable romaji_ja_period_table (
     ("DefaultRomajiJaPeriodTable"),
    fcitx_anthy_romaji_ja_period_rule);
static Key2KanaTable romaji_wide_period_table (
     ("DefaultRomajiWidePeriodTable"),
    fcitx_anthy_romaji_wide_period_rule);
static Key2KanaTable romaji_half_period_table (
     ("DefaultRomajiHalfPeriodTable"),
    fcitx_anthy_romaji_half_period_rule);

static Key2KanaTable kana_ja_period_table (
     ("DefaultKanaJaPeriodTable"),
    fcitx_anthy_kana_ja_period_rule);
static Key2KanaTable kana_wide_period_table (
     ("DefaultKanaWidePeriodTable"),
    fcitx_anthy_kana_wide_period_rule);
static Key2KanaTable kana_half_period_table (
     ("DefaultKanaHalfPeriodTable"),
    fcitx_anthy_kana_half_period_rule);

// comma
static Key2KanaTable romaji_ja_comma_table (
     ("DefaultRomajiJaCommaTable"),
    fcitx_anthy_romaji_ja_comma_rule);
static Key2KanaTable romaji_wide_comma_table (
     ("DefaultRomajiWideCommaTable"),
    fcitx_anthy_romaji_wide_comma_rule);
static Key2KanaTable romaji_half_comma_table (
     ("DefaultRomajiHalfCommaTable"),
    fcitx_anthy_romaji_half_comma_rule);

static Key2KanaTable kana_ja_comma_table (
     ("DefaultKanaJaCommaTable"),
    fcitx_anthy_kana_ja_comma_rule);
static Key2KanaTable kana_wide_comma_table (
     ("DefaultKanaWideCommaTable"),
    fcitx_anthy_kana_wide_comma_rule);
static Key2KanaTable kana_half_comma_table (
     ("DefaultKanaHalfCommaTable"),
    fcitx_anthy_kana_half_comma_rule);

// bracket
static Key2KanaTable romaji_ja_bracket_table (
     ("DefaultRomajiJaBracketTable"),
    fcitx_anthy_romaji_ja_bracket_rule);
static Key2KanaTable romaji_wide_bracket_table (
     ("DefaultRomajiWideBracketTable"),
    fcitx_anthy_romaji_wide_bracket_rule);

static Key2KanaTable kana_ja_bracket_table (
     ("DefaultKanaJaBracketTable"),
    fcitx_anthy_kana_ja_bracket_rule);
static Key2KanaTable kana_wide_bracket_table (
     ("DefaultRomajiWideBracketTable"),
    fcitx_anthy_kana_wide_bracket_rule);

// slash
static Key2KanaTable romaji_ja_slash_table (
     ("DefaultRomajiJaSlashTable"),
    fcitx_anthy_romaji_ja_slash_rule);
static Key2KanaTable romaji_wide_slash_table (
     ("DefaultRomajiWideSlashTable"),
    fcitx_anthy_romaji_wide_slash_rule);

static Key2KanaTable kana_ja_slash_table (
     ("DefaultKanaJaSlashTable"),
    fcitx_anthy_kana_ja_slash_rule);
static Key2KanaTable kana_wide_slash_table (
     ("DefaultRomajiWideSlashTable"),
    fcitx_anthy_kana_wide_slash_rule);


Key2KanaRule::Key2KanaRule ()
{
}

Key2KanaRule::Key2KanaRule (std::string sequence,
                            const std::vector<std::string> &result)
    : m_sequence (sequence),
      m_result   (result)
{
}


Key2KanaRule::~Key2KanaRule ()
{
}

std::string
Key2KanaRule::get_sequence (void)
{
    return m_sequence;
}

std::string
Key2KanaRule::get_result (unsigned int idx)
{
    if (idx < m_result.size ())
        return m_result[idx];

    return std::string ();
}

void
Key2KanaRule::clear (void)
{
    m_sequence = std::string ();
    m_result.clear ();
}

bool
Key2KanaRule::is_empty (void)
{
    if (!m_sequence.empty ())
        return false;

    if (m_result.empty ())
        return true;

    for (unsigned int i = 0; i < m_result.size (); i++) {
        if (!m_result[i].empty ())
            return false;
    }

    return true;
}


Key2KanaTable::Key2KanaTable (std::string name)
    : m_name (name)
{
}

Key2KanaTable::Key2KanaTable (std::string name, ConvRule *table)
    : m_name (name)
{
    for (unsigned int i = 0; table[i].string; i++) {
        append_rule (table[i].string ? table[i].string : "",
                     table[i].result ? table[i].result : "",
                     table[i].cont   ? table[i].cont   : "");
    }
}

Key2KanaTable::Key2KanaTable (std::string name, NicolaRule *table)
    : m_name (name)
{
    for (unsigned int i = 0; table[i].key; i++) {
        append_rule (table[i].key         ? table[i].key           : "",
                     table[i].single      ? table[i].single        : "",
                     table[i].left_shift  ? table[i].left_shift    : "",
                     table[i].right_shift ? table[i].right_shift   : "");
    }
}

Key2KanaTable::~Key2KanaTable ()
{
}

void
Key2KanaTable::append_rule (std::string sequence,
                            const std::vector<std::string> &result)
{
    m_rules.push_back (Key2KanaRule (sequence, result));
}

void
Key2KanaTable::append_rule (std::string sequence,
                            std::string result,
                            std::string cont)
{
    std::vector<std::string> list;
    list.push_back (result);
    list.push_back (cont);
    m_rules.push_back (Key2KanaRule (sequence, list));
}

void
Key2KanaTable::append_rule (std::string sequence,
                            std::string normal,
                            std::string left_shift,
                            std::string right_shift)
{
    std::vector<std::string> list;
    list.push_back (normal);
    list.push_back (left_shift);
    list.push_back (right_shift);
    m_rules.push_back (Key2KanaRule (sequence, list));
}

void
Key2KanaTable::clear (void)
{
    m_rules.clear ();

}

Key2KanaTableSet::Key2KanaTableSet ()
    : m_name                   (""),
      m_fundamental_table      (NULL),
      m_voiced_consonant_table (Key2KanaTable ("voiced consonant table")),
      m_additional_table       (NULL),
      m_typing_method          (FCITX_ANTHY_TYPING_METHOD_ROMAJI),
      m_period_style           (FCITX_ANTHY_PERIOD_JAPANESE),
      m_comma_style            (FCITX_ANTHY_COMMA_JAPANESE),
      m_bracket_style          (FCITX_ANTHY_BRACKET_JAPANESE),
      m_slash_style            (FCITX_ANTHY_SLASH_JAPANESE),
      m_use_half_symbol        (false),
      m_use_half_number        (false)
{
    set_typing_method (m_typing_method);
}

Key2KanaTableSet::~Key2KanaTableSet ()
{
}

void
Key2KanaTableSet::set_typing_method (TypingMethod method,
                                     Key2KanaTable *fundamental_table)
{
    m_typing_method = method;
    m_fundamental_table = fundamental_table;
    reset_tables ();
}

void
Key2KanaTableSet::set_symbol_width (bool half)
{
    m_use_half_symbol = half;
    reset_tables ();
}

void
Key2KanaTableSet::set_number_width (bool half)
{
    m_use_half_number = half;
    reset_tables ();
}

void
Key2KanaTableSet::set_period_style (PeriodStyle style)
{
    m_period_style = style;
    reset_tables ();
}

void
Key2KanaTableSet::set_comma_style (CommaStyle  style)
{
    m_comma_style = style;
    reset_tables ();
}

void
Key2KanaTableSet::set_bracket_style (BracketStyle style)
{
    m_bracket_style = style;
    reset_tables ();
}

void
Key2KanaTableSet::set_slash_style (SlashStyle style)
{
    m_slash_style = style;
    reset_tables ();
}

static void
create_voiced_consonant_table (Key2KanaTable &table, Key2KanaTable &fund_table)
{
    table.clear ();

    const std::string sonant_mark      = std::string ("\xE3\x82\x9B");
    const std::string half_sonant_mark = std::string ("\xE3\x82\x9C");
    std::vector<std::string> sonant_mark_list;
    std::vector<std::string> half_sonant_mark_list;

    Key2KanaRules::iterator it;
    Key2KanaRules &rules = fund_table.get_table ();
    for (it = rules.begin (); it != rules.end (); it++) {
        std::string result = it->get_result (0);
        if (result == sonant_mark)
            sonant_mark_list.push_back (it->get_sequence ());
        else if (result == half_sonant_mark)
            half_sonant_mark_list.push_back (it->get_sequence ());
    }

    VoicedConsonantRule *templ = fcitx_anthy_voiced_consonant_table;

    for (unsigned int i = 0; templ[i].string; i++) {
        if (templ[i].voiced && *templ[i].voiced) {
            std::vector<std::string>::iterator it;
            for (it = sonant_mark_list.begin ();
                 it != sonant_mark_list.end ();
                 it++)
            {
                table.append_rule (std::string (templ[i].string) + *it,
                                   std::string (templ[i].voiced),
                                   std::string ());
            }
        }
        if (templ[i].half_voiced && *templ[i].half_voiced) {
            std::vector<std::string>::iterator it;
            for (it = half_sonant_mark_list.begin ();
                 it != half_sonant_mark_list.end ();
                 it++)
            {
                table.append_rule (std::string (templ[i].string) + *it,
                                   std::string (templ[i].half_voiced),
                                   std::string ());
            }
        }
    }
}

void
Key2KanaTableSet::reset_tables (void)
{
    m_all_tables.clear ();

    bool is_romaji = m_typing_method == FCITX_ANTHY_TYPING_METHOD_ROMAJI;
    bool is_kana   = m_typing_method == FCITX_ANTHY_TYPING_METHOD_KANA;
    bool is_nicola = m_typing_method == FCITX_ANTHY_TYPING_METHOD_NICOLA;

    // symbols table
    if (m_use_half_symbol)
        m_all_tables.push_back (&half_symbol_table);
    else
        m_all_tables.push_back (&wide_symbol_table);

    // numbers table
    if (m_use_half_number)
        m_all_tables.push_back (&half_number_table);
    else
        m_all_tables.push_back (&wide_number_table);

    if (is_romaji || is_kana)
    {
        switch (m_period_style) {
        case FCITX_ANTHY_PERIOD_JAPANESE:
            if (is_romaji)
                m_all_tables.push_back (&romaji_ja_period_table);
            else
                m_all_tables.push_back (&kana_ja_period_table);
            break;
        case FCITX_ANTHY_PERIOD_WIDE:
            if (is_romaji)
                m_all_tables.push_back (&romaji_wide_period_table);
            else
                m_all_tables.push_back (&kana_wide_period_table);
            break;
        case FCITX_ANTHY_PERIOD_HALF:
            if (is_romaji)
                m_all_tables.push_back (&romaji_half_period_table);
            else
                m_all_tables.push_back (&kana_half_period_table);
            break;
        default:
            break;
        }
    }

    if (is_romaji || is_kana)
    {
        switch (m_comma_style) {
        case FCITX_ANTHY_COMMA_JAPANESE:
            if (is_romaji)
                m_all_tables.push_back (&romaji_ja_comma_table);
            else
                m_all_tables.push_back (&kana_ja_comma_table);
            break;
        case FCITX_ANTHY_COMMA_WIDE:
            if (is_romaji)
                m_all_tables.push_back (&romaji_wide_comma_table);
            else
                m_all_tables.push_back (&kana_wide_comma_table);
            break;
        case FCITX_ANTHY_COMMA_HALF:
            if (is_romaji)
                m_all_tables.push_back (&romaji_half_comma_table);
            else
                m_all_tables.push_back (&kana_half_comma_table);
            break;
        default:
            break;
        }
    }

    if (is_romaji || is_kana)
    {
        switch (m_bracket_style) {
        case FCITX_ANTHY_BRACKET_JAPANESE:
            if (is_romaji)
                m_all_tables.push_back (&romaji_ja_bracket_table);
            else
                m_all_tables.push_back (&kana_ja_bracket_table);
            break;
        case FCITX_ANTHY_BRACKET_WIDE:
            if (is_romaji)
                m_all_tables.push_back (&romaji_wide_bracket_table);
            else
                m_all_tables.push_back (&kana_wide_bracket_table);
            break;
        default:
            break;
        }
    }

    if (is_romaji || is_kana)
    {
        switch (m_slash_style) {
        case FCITX_ANTHY_SLASH_JAPANESE:
            if (is_romaji)
                m_all_tables.push_back (&romaji_ja_slash_table);
            else
                m_all_tables.push_back (&kana_ja_slash_table);
            break;
        case FCITX_ANTHY_SLASH_WIDE:
            if (is_romaji)
                m_all_tables.push_back (&romaji_wide_slash_table);
            else
                m_all_tables.push_back (&kana_wide_slash_table);
            break;
        default:
            break;
        }
    }

    if (!m_fundamental_table) {
        if (is_romaji) {
            m_all_tables.push_back (&romaji_double_consonant_table);
            m_all_tables.push_back (&romaji_table);
        } else if (is_kana) {
            create_voiced_consonant_table (m_voiced_consonant_table,
                                           kana_table);
            m_all_tables.push_back (&m_voiced_consonant_table);
            m_all_tables.push_back (&kana_table);
        } else if (is_nicola) {
            m_all_tables.push_back (&nicola_table);
        }
    } else {
        if (is_romaji) {
            m_all_tables.push_back (&romaji_double_consonant_table);
            m_all_tables.push_back (m_fundamental_table);
        } else if (is_kana) {
            create_voiced_consonant_table (m_voiced_consonant_table,
                                           *m_fundamental_table);
            m_all_tables.push_back (&m_voiced_consonant_table);
            m_all_tables.push_back (m_fundamental_table);
        } else if (is_nicola) {
            m_all_tables.push_back (m_fundamental_table);
        }
    }
}
