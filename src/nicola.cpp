/*
 * SPDX-FileCopyrightText: 2004 Hiroyuki Ikezoe
 * SPDX-FileCopyrightText: 2004 Takuro Ashie
 * SPDX-FileCopyrightText: 2012-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "nicola.h"
#include "config.h"
#include "default_tables.h"
#include "engine.h"
#include "key2kana_base.h"
#include "key2kana_table.h"
#include "state.h"
#include "utils.h"
#include <algorithm>
#include <cstdint>
#include <fcitx-utils/charutils.h>
#include <fcitx-utils/eventloopinterface.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/keysym.h>
#include <fcitx-utils/log.h>
#include <fcitx/event.h>
#include <string>
#include <string_view>
#include <vector>

void NicolaTimeoutFunc(void *arg) {
    NicolaConvertor *convertor = (NicolaConvertor *)arg;
    convertor->processTimeout();
}

NicolaConvertor::NicolaConvertor(AnthyState &anthy, Key2KanaTableSet &tables)
    : Key2KanaConvertorBase(anthy), tables_(tables), processingTimeout_(false) {
}

NicolaConvertor::~NicolaConvertor() { timer_.reset(); }

bool NicolaConvertor::canAppend(const fcitx::KeyEvent &key, bool ignore_space) {
    if (key.rawKey() == throughKeyEvent) {
        throughKeyEvent = fcitx::Key();
        return false;
    }

    if (processingTimeout_ && !prevCharKey_.isValid() &&
        prevThumbKey_.isValid()) {
        const fcitx::KeyEvent event(key.inputContext(), prevThumbKey_);
        emitKeyEvent(event);
        prevThumbKey_ = fcitx::Key();
        return false;
    }

    auto sym = key.rawKey().sym();
    if (key.isRelease() &&
        (sym != prevCharKey_.sym() && sym != repeatCharKey_.sym() &&
         sym != prevThumbKey_.sym() && sym != repeatThumbKey_.sym())) {
        return false;
    }

    // ignore short cut keys of apllication.
    auto states = key.rawKey().states();
    if ((states & fcitx::KeyState::Ctrl) || (states & fcitx::KeyState::Alt) ||
        (states & fcitx::KeyState::Super)) {
        return false;
    }

    auto chr = util::get_ascii_code(key);
    if (fcitx::charutils::isprint(chr) &&
        (ignore_space || !fcitx::charutils::isspace(chr))) {
        return true;
    }

    if (isThumbKey(key.rawKey())) {
        return true;
    }

    return false;
}

void NicolaConvertor::search(const fcitx::Key &key, NicolaShiftType shift_type,
                             std::string &result, std::string &raw) {
    raw = util::get_ascii_code(key);

    std::string str1;
    if (isCaseSensitive()) {
        str1 = raw;
    } else {
        str1 = fcitx::charutils::tolower(util::get_ascii_code(key));
    }

    const std::vector<Key2KanaTable *> &tables = tables_.get_tables();
    for (unsigned int j = 0; j < tables.size(); j++) {
        if (!tables[j]) {
            continue;
        }

        const Key2KanaRules &rules = tables[j]->table();

        for (const auto &rule : rules) {
            std::string str2 = rule.sequence();

            for (unsigned int k = 0; !isCaseSensitive() && k < str2.length();
                 k++) {
                str2[k] = fcitx::charutils::tolower(str2[k]);
            }

            if (str1 == str2) {
                switch (shift_type) {
                case FCITX_ANTHY_NICOLA_SHIFT_RIGHT:
                    result = rule.result(2);
                    break;
                case FCITX_ANTHY_NICOLA_SHIFT_LEFT:
                    result = rule.result(1);
                    break;
                default:
                    result = rule.result(0);
                    break;
                }
                break;
            }
        }
    }

    if (result.empty()) {
        result = raw;
    }
}

bool NicolaConvertor::handleVoicedConsonant(std::string &result,
                                            std::string &pending) {
    const auto &table = fcitx_anthy_voiced_consonant_table;

    if (result.empty()) {
        return false;
    }

    if (pending_.empty()) {
        for (const auto &item : table) {
            if (result == item.string) {
                pending = pending_ = result;
                result = std::string();
                return false;
            }
        }

    } else if (result == "\xE3\x82\x9B") {
        // voiced consonant
        for (const auto &item : table) {
            if (pending_ == item.string) {
                result = item.voiced;
                pending_ = std::string();
                return false;
            }
        }
        return true;

    } else if (result == "\xE3\x82\x9C") {
        // half voiced consonant
        for (const auto &item : table) {
            if (pending_ == item.string) {
                result = item.half_voiced;
                pending_ = std::string();
                return false;
            }
        }
        return true;

    } else {
        pending_ = std::string();
        for (const auto &item : table) {
            if (result == item.string) {
                pending = pending_ = result;
                result = std::string();
                return true;
            }
        }
        return true;
    }

    return false;
}

bool NicolaConvertor::isCharKey(const fcitx::KeyEvent &key) {
    return !isThumbKey(key.rawKey()) &&
           fcitx::charutils::isprint(util::get_ascii_code(key));
}

bool NicolaConvertor::isThumbKey(const fcitx::Key &key) {
    return isLeftThumbKey(key) || isRightThumbKey(key);
}

bool NicolaConvertor::isLeftThumbKey(const fcitx::Key &key) {
    return util::match_key_event(*config().key->leftThumbKeys, key,
                                 fcitx::KeyStates(0xFFFF));
}

bool NicolaConvertor::isRightThumbKey(const fcitx::Key &key) {
    return util::match_key_event(*config().key->rightThumbKeys, key,
                                 fcitx::KeyStates(0xFFFF));
}

NicolaShiftType NicolaConvertor::thumbKeyType(const fcitx::Key &key) {
    if (isLeftThumbKey(key)) {
        return FCITX_ANTHY_NICOLA_SHIFT_LEFT;
    }
    if (isRightThumbKey(key)) {
        return FCITX_ANTHY_NICOLA_SHIFT_RIGHT;
    }
    return FCITX_ANTHY_NICOLA_SHIFT_NONE;
}

bool NicolaConvertor::emitKeyEvent(const fcitx::Key &key) {
    fcitx::KeyEvent keyEvent(state_.inputContext(), key);
    return emitKeyEvent(keyEvent);
}

bool NicolaConvertor::emitKeyEvent(const fcitx::KeyEvent &key) {
    throughKeyEvent = key.rawKey();

    // state_.forward_key_event (key);
    return state_.processKeyEvent(key);
}

void NicolaConvertor::processTimeout() {
    processingTimeout_ = true;
    if (prevCharKey_.isValid()) {
        fcitx::KeyEvent keyEvent(state_.inputContext(), prevCharKey_);
        state_.processKeyEvent(keyEvent);
    } else if (prevThumbKey_.isValid()) {
        fcitx::KeyEvent keyEvent(state_.inputContext(), prevThumbKey_);
        state_.processKeyEvent(keyEvent);
    }
    processingTimeout_ = false;
}

void NicolaConvertor::setAlarm(int time_msec) {
    uint64_t time = std::clamp(time_msec, 5, 1000);

    timer_ = state_.instance()->eventLoop().addTimeEvent(
        CLOCK_MONOTONIC, fcitx::now(CLOCK_MONOTONIC) + (time * 1000), 1,
        [this](fcitx::EventSourceTime *, uint64_t) {
            processTimeout();
            return true;
        });
}

bool NicolaConvertor::stop() {
    bool hasTimer = timer_.get();
    timer_.reset();
    return hasTimer;
}

int NicolaConvertor::thumbKey(const fcitx::KeyEvent &key) {
    return isLeftThumbKey(key.rawKey()) ? 1 : 2;
}

bool NicolaConvertor::append(const fcitx::KeyEvent &key, std::string &result,
                             std::string &pending, std::string &raw) {
    if (processingTimeout_) {
        search(prevCharKey_, thumbKeyType(prevThumbKey_), result, raw);
        if (!prevThumbKey_.isValid()) {
            prevCharKey_ = fcitx::Key();
            prevThumbKey_ = fcitx::Key();
        } else {
            repeatCharKey_ = prevCharKey_;
            repeatThumbKey_ = prevThumbKey_;
        }
        return handleVoicedConsonant(result, pending);
    }

    if (!key.isRelease() && util::key_is_keypad(key.rawKey())) {
        raw = util::keypad_to_string(key);

        // convert key pad string to wide
        std::string wide;
        TenKeyType ten_key_type = *config().general->tenKeyType;
        if ((ten_key_type == TenKeyType::FOLLOWMODE &&
             (state_.inputMode() == InputMode::LATIN ||
              state_.inputMode() == InputMode::WIDE_LATIN)) ||
            ten_key_type == TenKeyType::HALF) {
            wide = raw;
        } else {
            wide = util::convert_to_wide(raw);
        }

        result = wide;

        prevCharKey_ = repeatCharKey_ = fcitx::Key();
        prevThumbKey_ = repeatThumbKey_ = fcitx::Key();

        return handleVoicedConsonant(result, pending);
    }

    if (key.isRelease()) {
        if (key.rawKey() == prevCharKey_) {
            if (stop()) {
                search(prevCharKey_, thumbKeyType(prevThumbKey_), result, raw);
            }
            prevCharKey_ = fcitx::Key();
        } else if (thumbKeyType(key.rawKey()) == thumbKeyType(prevThumbKey_)) {
            if (stop()) {
                emitKeyEvent(prevThumbKey_);
            }
            prevThumbKey_ = fcitx::Key();
        }
        if (isThumbKey(key.rawKey())) {
            repeatThumbKey_ = fcitx::Key();
        } else if (key.rawKey() == repeatCharKey_) {
            repeatCharKey_ = fcitx::Key();
        }
    } else if (isThumbKey(key.rawKey())) {
        if (repeatThumbKey_.isValid() && !(key.rawKey() == repeatThumbKey_)) {
            stop();
            prevCharKey_ = prevThumbKey_ = repeatCharKey_ = repeatThumbKey_ =
                fcitx::Key();
        }
        if (prevThumbKey_.isValid()) {
            stop();
            emitKeyEvent(prevThumbKey_);
            prevThumbKey_ = key.rawKey();
            setAlarm(*config().key->nicolaTime);
        } else if (prevCharKey_.isValid()) {
            stop();
            repeatCharKey_ = prevCharKey_;
            repeatThumbKey_ = key.rawKey();
            search(prevCharKey_, thumbKeyType(repeatThumbKey_), result, raw);
        } else {
            if (thumbKeyType(repeatThumbKey_) == thumbKeyType(key.rawKey())) {
                if (repeatCharKey_.isValid()) {
                    search(repeatCharKey_, thumbKeyType(repeatThumbKey_),
                           result, raw);
                }
            } else {
                prevThumbKey_ = key.rawKey();
                setAlarm(*config().key->nicolaTime);
            }
        }
    } else if (isCharKey(key)) {
        if (repeatCharKey_.isValid() && key.rawKey() != repeatCharKey_) {
            stop();
            prevCharKey_ = prevThumbKey_ = repeatCharKey_ = repeatThumbKey_ =
                fcitx::Key();
        }
        if (prevCharKey_.isValid()) {
            stop();
            search(prevCharKey_, thumbKeyType(prevThumbKey_), result, raw);
            setAlarm(*config().key->nicolaTime);
            prevCharKey_ = key.rawKey();
        } else if (isThumbKey(prevThumbKey_)) {
            stop();
            repeatCharKey_ = key.rawKey();
            repeatThumbKey_ = prevThumbKey_;
            search(key.rawKey(), thumbKeyType(prevThumbKey_), result, raw);
        } else {
            if (key.rawKey() == repeatCharKey_) {
                if (repeatThumbKey_.isValid()) {
                    search(repeatCharKey_, thumbKeyType(repeatThumbKey_),
                           result, raw);
                }
            } else {
                setAlarm(*config().key->nicolaTime);
                prevCharKey_ = key.rawKey();
            }
        }
    } else {
        if (prevCharKey_.isValid()) {
            stop();
            search(prevCharKey_, thumbKeyType(prevThumbKey_), result, raw);
        } else if (prevThumbKey_.isValid()) {
            stop();
            emitKeyEvent(prevThumbKey_);
        }
        if (emitKeyEvent(key)) {
            return true;
        }
    }

    FCITX_ANTHY_DEBUG() << "prev: " << prevCharKey_;

    return handleVoicedConsonant(result, pending);
}

bool NicolaConvertor::append(std::string_view raw, std::string &result,
                             std::string & /*pending*/) {
    result = raw;
    pending_ = std::string();

    return false;
}

void NicolaConvertor::clear() {
    pending_ = std::string();
    prevCharKey_ = fcitx::Key();
    prevThumbKey_ = fcitx::Key();
    repeatCharKey_ = fcitx::Key();
    repeatThumbKey_ = fcitx::Key();
}

bool NicolaConvertor::isPending() const { return !pending_.empty(); }

std::string NicolaConvertor::pending() const { return pending_; }

std::string NicolaConvertor::flushPending() { return std::string(); }

void NicolaConvertor::resetPending(const std::string &result,
                                   const std::string & /*raw*/) {
    const auto &table = fcitx_anthy_voiced_consonant_table;

    pending_ = std::string();

    for (const auto &item : table) {
        if (result == item.string) {
            pending_ = result;
            return;
        }
    }
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
