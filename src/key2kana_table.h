//
// Copyright (C) 2005 Takuro Ashie
// Copyright (C) 2017~2017 by CSSlayer
// wengxt@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
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
  std::string result(unsigned int idx);

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

  Key2KanaRules &table() { return rules_; }

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

  std::vector<Key2KanaTable *> &get_tables() { return allTables_; };

  void setTypingMethod(TypingMethod method,
                       Key2KanaTable *fundamental_table = nullptr);
  void setSymbolHalf(bool half);
  void setNumberHalf(bool half);
  void setPeriodStyle(PeriodStyle style);
  void setCommaStyle(CommaStyle style);
  void setBracketStyle(BracketStyle style);
  void setSlashStyle(SlashStyle style);

  TypingMethod typingMethod() { return typingMethod_; }
  bool symbol_is_half() { return useHalfSymbol_; }
  bool isNumberHalf() { return useHalfNumber_; }
  PeriodStyle periodStyle() { return periodStyle_; }
  CommaStyle commaStyle() { return commaStyle_; }
  BracketStyle bracketStyle() { return bracketStyle_; }
  SlashStyle slashStyle() { return slashStyle_; }

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
