/*
 * SPDX-FileCopyrightText: 2005 Takuro Ashie
 * SPDX-FileCopyrightText: 2012-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "key2kana_table.h"
#include "config.h"
#include "default_tables.h"
#include "style_file.h"
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// fundamental table
static Key2KanaTable romaji_table(("DefaultRomajiTable"),
                                  fcitx_anthy_romaji_typing_rule);
static Key2KanaTable
    romaji_double_consonant_table(("DefaultRomajiDoubleConsonantTable"),
                                  fcitx_anthy_romaji_double_consonant_rule);
static Key2KanaTable kana_table(("DefaultKanaTable"),
                                fcitx_anthy_kana_typing_rule);
static Key2KanaTable
    kana_voiced_consonant_table(("DefaultKanaVoicedConsonantTable"),
                                fcitx_anthy_kana_voiced_consonant_rule);
static Key2KanaTable nicola_table(("DefaultNICOLATable"),
                                  fcitx_anthy_nicola_table);

// symbols
static Key2KanaTable half_symbol_table(("DefaultRomajiHalfSymbolTable"),
                                       fcitx_anthy_half_symbol_rule);
static Key2KanaTable wide_symbol_table(("DefaultRomajiWideSymbolTable"),
                                       fcitx_anthy_wide_symbol_rule);

// numbers
static Key2KanaTable half_number_table(("DefaultRomajiHalfNumberTable"),
                                       fcitx_anthy_half_number_rule);
static Key2KanaTable wide_number_table(("DefaultRomajiWideNumberTable"),
                                       fcitx_anthy_wide_number_rule);

// period
static Key2KanaTable romaji_ja_period_table(("DefaultRomajiJaPeriodTable"),
                                            fcitx_anthy_romaji_ja_period_rule);
static Key2KanaTable
    romaji_wide_period_table(("DefaultRomajiWidePeriodTable"),
                             fcitx_anthy_romaji_wide_period_rule);
static Key2KanaTable
    romaji_half_period_table(("DefaultRomajiHalfPeriodTable"),
                             fcitx_anthy_romaji_half_period_rule);

static Key2KanaTable kana_ja_period_table(("DefaultKanaJaPeriodTable"),
                                          fcitx_anthy_kana_ja_period_rule);
static Key2KanaTable kana_wide_period_table(("DefaultKanaWidePeriodTable"),
                                            fcitx_anthy_kana_wide_period_rule);
static Key2KanaTable kana_half_period_table(("DefaultKanaHalfPeriodTable"),
                                            fcitx_anthy_kana_half_period_rule);

// comma
static Key2KanaTable romaji_ja_comma_table(("DefaultRomajiJaCommaTable"),
                                           fcitx_anthy_romaji_ja_comma_rule);
static Key2KanaTable
    romaji_wide_comma_table(("DefaultRomajiWideCommaTable"),
                            fcitx_anthy_romaji_wide_comma_rule);
static Key2KanaTable
    romaji_half_comma_table(("DefaultRomajiHalfCommaTable"),
                            fcitx_anthy_romaji_half_comma_rule);

static Key2KanaTable kana_ja_comma_table(("DefaultKanaJaCommaTable"),
                                         fcitx_anthy_kana_ja_comma_rule);
static Key2KanaTable kana_wide_comma_table(("DefaultKanaWideCommaTable"),
                                           fcitx_anthy_kana_wide_comma_rule);
static Key2KanaTable kana_half_comma_table(("DefaultKanaHalfCommaTable"),
                                           fcitx_anthy_kana_half_comma_rule);

// bracket
static Key2KanaTable
    romaji_ja_bracket_table(("DefaultRomajiJaBracketTable"),
                            fcitx_anthy_romaji_ja_bracket_rule);
static Key2KanaTable
    romaji_wide_bracket_table(("DefaultRomajiWideBracketTable"),
                              fcitx_anthy_romaji_wide_bracket_rule);

static Key2KanaTable kana_ja_bracket_table(("DefaultKanaJaBracketTable"),
                                           fcitx_anthy_kana_ja_bracket_rule);
static Key2KanaTable
    kana_wide_bracket_table(("DefaultRomajiWideBracketTable"),
                            fcitx_anthy_kana_wide_bracket_rule);

// slash
static Key2KanaTable romaji_ja_slash_table(("DefaultRomajiJaSlashTable"),
                                           fcitx_anthy_romaji_ja_slash_rule);
static Key2KanaTable
    romaji_wide_slash_table(("DefaultRomajiWideSlashTable"),
                            fcitx_anthy_romaji_wide_slash_rule);

static Key2KanaTable kana_ja_slash_table(("DefaultKanaJaSlashTable"),
                                         fcitx_anthy_kana_ja_slash_rule);
static Key2KanaTable kana_wide_slash_table(("DefaultRomajiWideSlashTable"),
                                           fcitx_anthy_kana_wide_slash_rule);

Key2KanaRule::Key2KanaRule() {}

void Key2KanaRule::load(std::string sequence, std::vector<std::string> result) {
    sequence_ = std::move(sequence);
    result_ = std::move(result);
}

Key2KanaRule::~Key2KanaRule() {}

const std::string &Key2KanaRule::sequence() const { return sequence_; }

std::string Key2KanaRule::result(unsigned int idx) const {
    if (idx < result_.size()) {
        return result_[idx];
    }

    return std::string();
}

void Key2KanaRule::clear() {
    sequence_ = std::string();
    result_.clear();
}

bool Key2KanaRule::isEmpty() {
    if (!sequence_.empty()) {
        return false;
    }

    if (result_.empty()) {
        return true;
    }

    for (const auto &i : result_) {
        if (!i.empty()) {
            return false;
        }
    }

    return true;
}

Key2KanaTable::Key2KanaTable(std::string name) : name_(std::move(name)) {}

Key2KanaTable::Key2KanaTable(std::string name, std::span<const ConvRule> table)
    : name_(std::move(name)) {
    for (const auto &item : table) {
        appendRule(std::string{item.string}, std::string{item.result},
                   std::string{item.cont});
    }
}

Key2KanaTable::Key2KanaTable(std::string name,
                             std::span<const NicolaRule> table)
    : name_(std::move(name)) {
    for (const auto &item : table) {
        appendRule(std::string{item.key}, std::string{item.single},
                   std::string{item.left_shift}, std::string{item.right_shift});
    }
}

Key2KanaTable::Key2KanaTable(std::string_view section,
                             const StyleFile &styleFile)
    : Key2KanaTable(styleFile.title()) {
    if (auto keys = styleFile.getKeyList(section)) {
        for (const auto &key : *keys) {
            if (auto array = styleFile.getStringArray(section, key)) {
                appendRule(key, std::move(*array));
            }
        }
    }
}

void Key2KanaTable::appendRule(std::string sequence,
                               std::vector<std::string> result) {
    rules_.emplace_back();
    rules_.back().load(std::move(sequence), std::move(result));
}

void Key2KanaTable::appendRule(std::string sequence, std::string result,
                               std::string cont) {
    std::vector<std::string> list;
    list.push_back(std::move(result));
    list.push_back(std::move(cont));
    appendRule(std::move(sequence), std::move(list));
}

void Key2KanaTable::appendRule(std::string sequence, std::string normal,
                               std::string left_shift,
                               std::string right_shift) {
    std::vector<std::string> list;
    list.push_back(std::move(normal));
    list.push_back(std::move(left_shift));
    list.push_back(std::move(right_shift));
    appendRule(std::move(sequence), std::move(list));
}

void Key2KanaTable::clear() { rules_.clear(); }

Key2KanaTableSet::Key2KanaTableSet()
    : fundamentalTable_(nullptr),
      voicedConsonantTable_(Key2KanaTable("voiced consonant table")),
      additionalTable_(nullptr), typingMethod_(TypingMethod::ROMAJI),
      periodStyle_(PeriodStyle::JAPANESE), commaStyle_(CommaStyle::JAPANESE),
      bracketStyle_(BracketStyle::JAPANESE), slashStyle_(SlashStyle::JAPANESE),
      useHalfSymbol_(false), useHalfNumber_(false) {
    setTypingMethod(typingMethod_);
}

Key2KanaTableSet::~Key2KanaTableSet() {}

void Key2KanaTableSet::setTypingMethod(TypingMethod method,
                                       Key2KanaTable *fundamental_table) {
    typingMethod_ = method;
    fundamentalTable_ = fundamental_table;
    resetTables();
}

void Key2KanaTableSet::setSymbolHalf(bool half) {
    useHalfSymbol_ = half;
    resetTables();
}

void Key2KanaTableSet::setNumberHalf(bool half) {
    useHalfNumber_ = half;
    resetTables();
}

void Key2KanaTableSet::setPeriodStyle(PeriodStyle style) {
    periodStyle_ = style;
    resetTables();
}

void Key2KanaTableSet::setCommaStyle(CommaStyle style) {
    commaStyle_ = style;
    resetTables();
}

void Key2KanaTableSet::setBracketStyle(BracketStyle style) {
    bracketStyle_ = style;
    resetTables();
}

void Key2KanaTableSet::setSlashStyle(SlashStyle style) {
    slashStyle_ = style;
    resetTables();
}

static void create_voiced_consonant_table(Key2KanaTable &table,
                                          const Key2KanaTable &fund_table) {
    table.clear();

    const std::string sonant_mark = std::string("\xE3\x82\x9B");
    const std::string half_sonant_mark = std::string("\xE3\x82\x9C");
    std::vector<std::string> sonant_mark_list;
    std::vector<std::string> half_sonant_mark_list;

    Key2KanaRules::const_iterator it;
    const Key2KanaRules &rules = fund_table.table();
    for (it = rules.begin(); it != rules.end(); it++) {
        std::string result = it->result(0);
        if (result == sonant_mark) {
            sonant_mark_list.push_back(it->sequence());
        } else if (result == half_sonant_mark) {
            half_sonant_mark_list.push_back(it->sequence());
        }
    }

    const auto &templ = fcitx_anthy_voiced_consonant_table;

    for (const auto &item : templ) {
        if (!item.voiced.empty()) {
            std::vector<std::string>::iterator it;
            for (it = sonant_mark_list.begin(); it != sonant_mark_list.end();
                 it++) {
                table.appendRule(std::string(item.string) + *it,
                                 std::string(item.voiced), std::string());
            }
        }
        if (!item.half_voiced.empty()) {
            std::vector<std::string>::iterator it;
            for (it = half_sonant_mark_list.begin();
                 it != half_sonant_mark_list.end(); it++) {
                table.appendRule(std::string(item.string) + *it,
                                 std::string(item.half_voiced), std::string());
            }
        }
    }
}

void Key2KanaTableSet::resetTables() {
    allTables_.clear();

    bool is_romaji = typingMethod_ == TypingMethod::ROMAJI;
    bool is_kana = typingMethod_ == TypingMethod::KANA;
    bool is_nicola = typingMethod_ == TypingMethod::NICOLA;

    // symbols table
    if (useHalfSymbol_) {
        allTables_.push_back(&half_symbol_table);
    } else {
        allTables_.push_back(&wide_symbol_table);
    }

    // numbers table
    if (useHalfNumber_) {
        allTables_.push_back(&half_number_table);
    } else {
        allTables_.push_back(&wide_number_table);
    }

    if (is_romaji || is_kana) {
        switch (periodStyle_) {
        case PeriodStyle::JAPANESE:
            if (is_romaji) {
                allTables_.push_back(&romaji_ja_period_table);
            } else {
                allTables_.push_back(&kana_ja_period_table);
            }
            break;
        case PeriodStyle::WIDE:
            if (is_romaji) {
                allTables_.push_back(&romaji_wide_period_table);
            } else {
                allTables_.push_back(&kana_wide_period_table);
            }
            break;
        case PeriodStyle::HALF:
            if (is_romaji) {
                allTables_.push_back(&romaji_half_period_table);
            } else {
                allTables_.push_back(&kana_half_period_table);
            }
            break;
        default:
            break;
        }
    }

    if (is_romaji || is_kana) {
        switch (commaStyle_) {
        case CommaStyle::JAPANESE:
            if (is_romaji) {
                allTables_.push_back(&romaji_ja_comma_table);
            } else {
                allTables_.push_back(&kana_ja_comma_table);
            }
            break;
        case CommaStyle::WIDE:
            if (is_romaji) {
                allTables_.push_back(&romaji_wide_comma_table);
            } else {
                allTables_.push_back(&kana_wide_comma_table);
            }
            break;
        case CommaStyle::HALF:
            if (is_romaji) {
                allTables_.push_back(&romaji_half_comma_table);
            } else {
                allTables_.push_back(&kana_half_comma_table);
            }
            break;
        default:
            break;
        }
    }

    if (is_romaji || is_kana) {
        switch (bracketStyle_) {
        case BracketStyle::JAPANESE:
            if (is_romaji) {
                allTables_.push_back(&romaji_ja_bracket_table);
            } else {
                allTables_.push_back(&kana_ja_bracket_table);
            }
            break;
        case BracketStyle::WIDE:
            if (is_romaji) {
                {
                    allTables_.push_back(&romaji_wide_bracket_table);
                }
            } else {
                allTables_.push_back(&kana_wide_bracket_table);
            }
            break;
        default:
            break;
        }
    }

    if (is_romaji || is_kana) {
        switch (slashStyle_) {
        case SlashStyle::JAPANESE:
            if (is_romaji) {
                allTables_.push_back(&romaji_ja_slash_table);
            } else {
                allTables_.push_back(&kana_ja_slash_table);
            }
            break;
        case SlashStyle::WIDE:
            if (is_romaji) {
                allTables_.push_back(&romaji_wide_slash_table);
            } else {
                allTables_.push_back(&kana_wide_slash_table);
            }
            break;
        default:
            break;
        }
    }

    if (!fundamentalTable_) {
        if (is_romaji) {
            allTables_.push_back(&romaji_double_consonant_table);
            allTables_.push_back(&romaji_table);
        } else if (is_kana) {
            create_voiced_consonant_table(voicedConsonantTable_, kana_table);
            allTables_.push_back(&voicedConsonantTable_);
            allTables_.push_back(&kana_table);
        } else if (is_nicola) {
            allTables_.push_back(&nicola_table);
        }
    } else {
        if (is_romaji) {
            allTables_.push_back(&romaji_double_consonant_table);
            allTables_.push_back(fundamentalTable_);
        } else if (is_kana) {
            create_voiced_consonant_table(voicedConsonantTable_,
                                          *fundamentalTable_);
            allTables_.push_back(&voicedConsonantTable_);
            allTables_.push_back(fundamentalTable_);
        } else if (is_nicola) {
            allTables_.push_back(fundamentalTable_);
        }
    }
}
