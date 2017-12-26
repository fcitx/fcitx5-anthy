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

#include "action.h"
#include "utils.h"
#include <fcitx-config/option.h>

Action::Action()
    : name_(""), performFunction_(nullptr), keyBindings_(nullptr) {}

Action::Action(const std::string &name, const fcitx::KeyList &hotkey, PMF pmf)
    : name_(name), performFunction_(pmf), keyBindings_(&hotkey) {}

bool Action::perform(AnthyState *performer) {
    if (performFunction_) {
        return (performer->*performFunction_)();
    }

    return false;
}

bool Action::perform(AnthyState *performer, const fcitx::KeyEvent &key) {
    if (!performFunction_)
        return false;

    if (!matchKeyEvent(key)) {
        return false;
    }
    return (performer->*performFunction_)();
}

bool Action::matchKeyEvent(const fcitx::KeyEvent &key) {
    return util::match_key_event(*keyBindings_, key.rawKey(),
                                 fcitx::KeyState::CapsLock);
}
