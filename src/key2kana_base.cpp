/*
 * SPDX-FileCopyrightText: 2017-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "key2kana_base.h"
#include "state.h"

const AnthyConfig &Key2KanaConvertorBase::config() const {
    return state_.config();
}
