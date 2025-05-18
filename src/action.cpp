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
#include <fcitx-utils/key.h>
#include <fcitx-utils/keysym.h>
#include <fcitx/event.h>
#include <functional>
#include <string>
#include <utility>

Action::Action() : performFunction_(nullptr), keyBindings_(nullptr) {}

Action::Action(std::string name, const fcitx::KeyList &hotkey,
               std::function<bool()> pmf)
    : name_(std::move(name)), performFunction_(std::move(pmf)),
      keyBindings_(&hotkey) {}

bool Action::perform() {
    if (performFunction_) {
        return performFunction_();
    }

    return false;
}

bool Action::perform(const fcitx::KeyEvent &key) {
    if (!performFunction_) {
        return false;
    }

    if (!matchKeyEvent(key)) {
        return false;
    }
    return performFunction_();
}

bool Action::matchKeyEvent(const fcitx::KeyEvent &key) {
    return util::match_key_event(*keyBindings_, key.key(),
                                 fcitx::KeyState::CapsLock);
}
