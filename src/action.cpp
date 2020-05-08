/*
 * SPDX-FileCopyrightText: 2005 Takuro Ashie
 * SPDX-FileCopyrightText: 2017-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

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
    return util::match_key_event(*keyBindings_, key.key(),
                                 fcitx::KeyState::CapsLock);
}
