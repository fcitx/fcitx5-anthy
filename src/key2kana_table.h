/*
 * SPDX-FileCopyrightText: 2005 Takuro Ashie
 * SPDX-FileCopyrightText: 2017-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_ANTHY_KEY2KANA_TABLE_H_
#define _FCITX5_ANTHY_KEY2KANA_TABLE_H_

#include "config.h"
#include "default_tables.h"
#include <string>
#include <vector>

enum class PeriodStyle {
    JAPANESE,
    WIDE,
    HALF,
};

enum class CommaStyle {
    JAPANESE,
    WIDE,
    HALF,
};

enum class BracketStyle {
    JAPANESE,
    WIDE,
};

enum class SlashStyle {
    JAPANESE,
    WIDE,
};

class Key2KanaRule;
class Key2KanaTable;
class Key2KanaTableSet;

typedef std::vector<Key2KanaRule> Key2KanaRules;

class Key2KanaRule {
public:
    Key2KanaRule();
    virtual ~Key2KanaRule();

    void load(std::string sequence, std::vector<std::string> result);

    const std::string &sequence() const;
    std::string result(unsigned int idx) const;

    void clear();

    bool isEmpty();

private:
    std::string sequence_;
    std::vector<std::string> result_;
};

class Key2KanaTable {
public:
    Key2KanaTable(std::string name);
    Key2KanaTable(std::string name, ConvRule *table);
    Key2KanaTable(std::string name, NicolaRule *table);
    FCITX_INLINE_DEFINE_DEFAULT_DTOR_AND_MOVE_WITHOUT_SPEC(Key2KanaTable)

    const Key2KanaRules &table() const { return rules_; }

    void appendRule(std::string sequence, std::vector<std::string> result);
    void appendRule(std::string sequence, std::string result, std::string cont);
    void appendRule(std::string sequence, std::string normal,
                    std::string left_shift, std::string right_shift);
    void clear();

private:
    std::string name_;
    Key2KanaRules rules_;
};

class Key2KanaTableSet {
public:
    Key2KanaTableSet();
    virtual ~Key2KanaTableSet();

    const std::vector<Key2KanaTable *> &get_tables() const { return allTables_; };

    void setTypingMethod(TypingMethod method,
                         Key2KanaTable *fundamental_table = nullptr);
    void setSymbolHalf(bool half);
    void setNumberHalf(bool half);
    void setPeriodStyle(PeriodStyle style);
    void setCommaStyle(CommaStyle style);
    void setBracketStyle(BracketStyle style);
    void setSlashStyle(SlashStyle style);

    TypingMethod typingMethod() const { return typingMethod_; }
    bool symbol_is_half() const { return useHalfSymbol_; }
    bool isNumberHalf() const { return useHalfNumber_; }
    PeriodStyle periodStyle() const { return periodStyle_; }
    CommaStyle commaStyle() const { return commaStyle_; }
    BracketStyle bracketStyle() const { return bracketStyle_; }
    SlashStyle slashStyle() const { return slashStyle_; }

private:
    void resetTables();

private:
    std::string name_;

    // tables
    Key2KanaTable *fundamentalTable_;
    Key2KanaTable voicedConsonantTable_;
    std::vector<Key2KanaTable *> *additionalTable_;
    std::vector<Key2KanaTable *> allTables_;

    // flags
    TypingMethod typingMethod_;
    PeriodStyle periodStyle_;
    CommaStyle commaStyle_;
    BracketStyle bracketStyle_;
    SlashStyle slashStyle_;
    bool useHalfSymbol_;
    bool useHalfNumber_;
};

#endif // _FCITX5_ANTHY_KEY2KANA_TABLE_H_
