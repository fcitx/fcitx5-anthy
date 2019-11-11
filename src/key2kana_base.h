//
// Copyright (C) 2004 Hiroyuki Ikezoe
// Copyright (C) 2004 Takuro Ashie
// Copyright (C) 2012~2017 by CSSlayer
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
#ifndef _FCITX5_ANTHY_KEY2KANA_BASE_H_
#define _FCITX5_ANTHY_KEY2KANA_BASE_H_

#include <fcitx/event.h>

class AnthyState;
class AnthyConfig;

class Key2KanaConvertorBase {
public:
  Key2KanaConvertorBase(AnthyState &state)
      : state_(state), caseSensitive_(true){};
  virtual ~Key2KanaConvertorBase() = default;

  virtual bool canAppend(const fcitx::KeyEvent &key,
                         bool ignore_space = false) = 0;
  virtual bool append(const fcitx::KeyEvent &key, std::string &result,
                      std::string &pending, std::string &raw) = 0;
  virtual bool append(const std::string &raw, std::string &result,
                      std::string &pending) = 0;
  virtual void clear() = 0;

  virtual bool isPending() const = 0;
  virtual std::string pending() const = 0;
  virtual std::string flushPending() = 0;
  virtual void resetPending(const std::string &result,
                            const std::string &raw) = 0;

  virtual void resetPseudoAsciiMode() {}
  virtual bool processPseudoAsciiMode(const std::string &) { return false; }

  void setCaseSensitive(bool sensitive) { caseSensitive_ = sensitive; }
  bool isCaseSensitive() { return caseSensitive_; }

protected:
  AnthyConfig &config();

  AnthyState &state_;
  bool caseSensitive_;
};

#endif // _FCITX5_ANTHY_KEY2KANA_BASE_H_
