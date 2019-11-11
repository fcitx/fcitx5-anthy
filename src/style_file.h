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
#ifndef _FCITX5_ANTHY_STYLE_FILE_H_
#define _FCITX5_ANTHY_STYLE_FILE_H_

#include <string>
#include <vector>

class Key2KanaTable;
class StyleLine;
class StyleSection;
class StyleFile;

typedef std::vector<StyleLine> StyleLines;
typedef std::vector<StyleLines> StyleSections;
typedef std::vector<StyleFile> StyleFiles;

enum class StyleLineType {
  UNKNOWN,
  SPACE,
  COMMENT,
  SECTION,
  KEY,
};

class StyleLine {
public:
  StyleLine(StyleFile *style_file, std::string line);
  ~StyleLine();

public:
  StyleLineType type();
  std::string line() { return line_; }
  bool get_section(std::string &section);
  bool get_key(std::string &key);
  bool get_value(std::string &value);
  bool get_value_array(std::vector<std::string> &value);

private:
  StyleFile *styleFile_;
  std::string line_;
  StyleLineType type_;
};

class StyleFile {
public:
  StyleFile();
  FCITX_INLINE_DEFINE_DEFAULT_DTOR_AND_MOVE_WITHOUT_SPEC(StyleFile);

public:
  bool load(const std::string &filename);

  const std::string &title() const;

  bool getKeyList(std::vector<std::string> &keys, std::string section);
  bool getString(std::string &value, std::string section, std::string key);
  bool getStringArray(std::vector<std::string> &value, std::string section,
                      std::string key);

public: // for getting specific data
  Key2KanaTable key2kanaTable(std::string section);

private:
  void clear();
  void setupDefaultEntries();
  StyleLines *findSection(const std::string &section);

private:
  std::string title_;
  StyleSections sections_;
};

#endif // _FCITX5_ANTHY_STYLE_FILE_H_
