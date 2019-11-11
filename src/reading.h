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

#ifndef __FCITX_ANTHY_READING_H__
#define __FCITX_ANTHY_READING_H__

#include "engine.h"
#include "kana.h"
#include "key2kana.h"
#include "nicola.h"

class AnthyState;

typedef enum {
  FCITX_ANTHY_STRING_LATIN,
  FCITX_ANTHY_STRING_WIDE_LATIN,
  FCITX_ANTHY_STRING_HIRAGANA,
  FCITX_ANTHY_STRING_KATAKANA,
  FCITX_ANTHY_STRING_HALF_KATAKANA,
} StringType;

class Reading;
class ReadingSegment;
typedef std::vector<ReadingSegment> ReadingSegments;

class ReadingSegment {
  friend class Reading;

public:
  ReadingSegment();
  virtual ~ReadingSegment();

  const std::string &get() { return kana; }
  const std::string &get_raw() { return raw; }

  void split(ReadingSegments &segments);

private:
  std::string raw;
  std::string kana;
};

class Reading {
public:
  Reading(AnthyState &anthy);
  virtual ~Reading();

  bool canProcesKeyEvent(const fcitx::KeyEvent &key);
  bool processKeyEvent(const fcitx::KeyEvent &key);
  void finish();
  void clear();

  std::string getByChar(unsigned int start = 0, int length = -1,
                        StringType type = FCITX_ANTHY_STRING_HIRAGANA);
  std::string getRawByChar(unsigned int start = 0, int length = -1);
  bool append(const fcitx::KeyEvent &key, const std::string &string);
  void erase(unsigned int start = 0, int length = -1, bool allow_split = false);

  unsigned int length();
  unsigned int utf8Length();
  unsigned int caretPos();
  unsigned int caretPosByChar();
  void setCaretPosByChar(unsigned int pos);
  void moveCaret(int step, bool allow_split = false);

  void setTypingMethod(TypingMethod method);
  TypingMethod typingMethod();
  void setPeriodStyle(PeriodStyle style);
  PeriodStyle periodStyle();
  void setCommaStyle(CommaStyle style);
  CommaStyle commaStyle();
  void setBracketStyle(BracketStyle style);
  BracketStyle bracketStyle();
  void setSlashStyle(SlashStyle style);
  SlashStyle slashStyle();
  void setSymbolHalf(bool half);
  bool isSymbolHalf();
  void setNumberHalf(bool half);
  bool isNumberHalf();
  void setPseudoAsciiMode(int mode);
  bool isPseudoAsciiMode();
  void resetPseudoAsciiMode();

private:
  void resetPending();
  void splitSegment(unsigned int seg_id);

private:
  AnthyState &state_;

  // tables
  Key2KanaTableSet key2kanaTables_;
  Key2KanaTableSet nicolaTables_;

  // convertors
  Key2KanaConvertor key2kanaNormal_;
  KanaConvertor kana_;
  NicolaConvertor nicola_;
  Key2KanaConvertorBase *key2kana_;

  // state
  ReadingSegments segments_;
  unsigned int segmentPos_;
  unsigned int caretOffset_;
};

#endif /* __FCITX_ANTHY_READING_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
