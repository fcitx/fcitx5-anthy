/*
 * SPDX-FileCopyrightText: 2004 Hiroyuki Ikezoe
 * SPDX-FileCopyrightText: 2004 Takuro Ashie
 * SPDX-FileCopyrightText: 2017-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "key2kana.h"
#include "config.h"
#include "key2kana_base.h"
#include "key2kana_table.h"
#include "state.h"
#include "utils.h"
#include <fcitx-utils/charutils.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/keysym.h>
#include <fcitx-utils/utf8.h>
#include <fcitx/event.h>
#include <fcitx/inputmethodmanager.h>
#include <fcitx/instance.h>
#include <string>
#include <vector>

Key2KanaConvertor::Key2KanaConvertor(AnthyState &anthy,
                                     Key2KanaTableSet &tables)
    : Key2KanaConvertorBase(anthy), tables_(tables),
      isInPseudoAsciiMode_(false) {
    setCaseSensitive(false);
    setPseudoAsciiMode(0);
}

Key2KanaConvertor::~Key2KanaConvertor() {}

bool Key2KanaConvertor::canAppend(const fcitx::KeyEvent &key,
                                  bool ignore_space) {
    // ignore key release.
    if (key.isRelease()) {
        return false;
    }

    auto state = key.rawKey().states();
    // ignore short cut keys of apllication.
    if ((state & fcitx::KeyState::Ctrl) || (state & fcitx::KeyState::Alt) ||
        (state & fcitx::KeyState::Super)) {
        return false;
    }

    auto chr = util::get_ascii_code(key);
    if (fcitx::charutils::isprint(chr) &&
        (ignore_space || !fcitx::charutils::isspace(chr))) {
        return true;
    }

    if (util::key_is_keypad(key.rawKey())) {
        return true;
    }

    return false;
}

bool Key2KanaConvertor::append(const fcitx::KeyEvent &key, std::string &result,
                               std::string &pending, std::string &raw) {
    if (!canAppend(key)) {
        return false;
    }

    lastKey_ = key.rawKey();

    raw = util::keypad_to_string(key);

    if (util::key_is_keypad(key.rawKey())) {
        bool retval = false;
        std::string wide;
        TenKeyType ten_key_type = *config().general->tenKeyType;

        // convert key pad string to wide
        if ((ten_key_type == TenKeyType::FOLLOWMODE &&
             (state_.inputMode() == InputMode::LATIN ||
              state_.inputMode() == InputMode::HALF_KATAKANA)) ||
            ten_key_type == TenKeyType::HALF) {
            wide = raw;
        } else {
            wide = util::convert_to_wide(raw);
        }

        // join to previous string
        if (!exactMatch_.isEmpty()) {
            if (!exactMatch_.result(0).empty() &&
                exactMatch_.result(1).empty()) {
                result = exactMatch_.result(0);
            } else {
                retval = true; /* commit prev pending */
            }
            result += wide;
        } else {
            if (!pending_.empty()) {
                retval = true; /* commit prev pending */
            }
            result = wide;
        }

        pending_.clear();
        exactMatch_.clear();

        return retval;

    } // the key isn't keypad
    return append(raw, result, pending);
}

static int split_string_list(std::vector<std::string> &vec,
                             const std::string &str) {
    int count = 0;

    std::string temp;
    std::string::const_iterator bg;
    std::string::const_iterator ed;

    vec.clear();

    bg = str.begin();
    ed = str.begin();

    while (bg != str.end() && ed != str.end()) {
        for (; ed != str.end(); ++ed) {
            if (*ed == ',') {
                break;
            }
        }
        temp.assign(bg, ed);
        vec.push_back(temp);
        ++count;

        if (ed != str.end()) {
            bg = ++ed;
        }
    }
    return count;
}

bool CheckLayout(fcitx::Instance *instance) {
    const auto &group = instance->inputMethodManager().currentGroup();
    std::string layout = group.layoutFor("anthy");
    if (layout.empty()) {
        layout = group.defaultLayout();
    }

    return layout == "jp" || layout.starts_with("jp-");
}

bool Key2KanaConvertor::append(const std::string &str, std::string &result,
                               std::string &pending) {
    std::string matching_str = pending_ + str;
    Key2KanaRule exact_match;
    bool has_partial_match = false;
    bool retval = false;

    if (pseudoAsciiMode_ != 0 && processPseudoAsciiMode(str)) {
        pending_ += str;
        pending = pending_;
        return false;
    }
    if (!caseSensitive_) {
        std::string half = matching_str;
        for (unsigned int i = 0; i < half.length(); i++) {
            half[i] = fcitx::charutils::tolower(half[i]);
        }
        matching_str = half;
    }

    /* find matched table */
    if ((state_.typingMethod() == TypingMethod::KANA) &&
        (CheckLayout(state_.instance())) &&
        (lastKey_.sym() == FcitxKey_backslash) &&
        (lastKey_.code() != 132 && lastKey_.code() != 133) &&
        (!config().key->kanaLayoutRoKey->empty())) {
        // Special treatment for Kana "Ro" key.
        // This code is a temporary solution. It doesn't care some minor cases.
        std::vector<std::string> kana_ro_result;
        split_string_list(kana_ro_result, *config().key->kanaLayoutRoKey);
        Key2KanaRule kana_ro_rule;
        kana_ro_rule.load("\\", kana_ro_result);
        result = kana_ro_rule.result(0);
        pending_.clear();
        exactMatch_.clear();
        return matching_str != "\\";
    }
    const std::vector<Key2KanaTable *> &tables = tables_.get_tables();
    for (auto *table : tables) {
        if (!table) {
            continue;
        }

        for (const auto &rule : table->table()) {
            /* matching */
            std::string seq = rule.sequence();
            if (!caseSensitive_) {
                for (unsigned int j = 0; j < seq.length(); j++) {
                    seq[j] = fcitx::charutils::tolower(seq[j]);
                }
            }
            std::string romaji = seq;
            if (romaji.find(matching_str) == 0) {
                if (romaji.length() == matching_str.length()) {
                    /* exact match */
                    exact_match = rule;
                } else {
                    /* partial match */
                    has_partial_match = true;
                }
            }
        }
    }

    /* return result */
    if (has_partial_match) {
        exactMatch_ = exact_match;
        result.clear();
        pending_ += str;
        pending = pending_;

    } else if (!exact_match.isEmpty()) {
        if (!exact_match.result(1).empty()) {
            exactMatch_ = exact_match;
        } else {
            exactMatch_.clear();
        }
        pending_ = exact_match.result(1);
        result = exact_match.result(0);
        pending = pending_;

    } else {
        if (!exactMatch_.isEmpty()) {
            if (!exactMatch_.result(0).empty() &&
                exactMatch_.result(1).empty()) {
                result = exactMatch_.result(0);
            } else {
                retval = true; /* commit prev pending */
            }
            pending_.clear();
            exactMatch_.clear();

            std::string tmp_result;
            append(str, tmp_result, pending);
            result += tmp_result;

        } else {
            if (!pending_.empty()) {
                retval = true; /* commit prev pending */
                pending_ = str;
                pending = pending_;

            } else {
                result = str;
                pending.clear();
                pending_.clear();
            }
        }
    }

    return retval;
}

void Key2KanaConvertor::clear() {
    pending_.clear();
    exactMatch_.clear();
    lastKey_ = fcitx::Key();
    resetPseudoAsciiMode();
}

bool Key2KanaConvertor::isPending() const { return !pending_.empty(); }

std::string Key2KanaConvertor::pending() const { return pending_; }

std::string Key2KanaConvertor::flushPending() {
    std::string result;
    if (!exactMatch_.isEmpty()) {
        if (!exactMatch_.result(0).empty() && exactMatch_.result(1).empty()) {
            result = exactMatch_.result(0);
        } else if (!exactMatch_.result(1).empty()) {
            result += exactMatch_.result(1);
        } else if (!pending_.empty()) {
            result += pending_;
        }
    }
    clear();
    return result;
}

void Key2KanaConvertor::resetPending(const std::string & /*result*/,
                                     const std::string &raw) {
    lastKey_ = fcitx::Key();

    for (unsigned int i = 0; i < fcitx::utf8::length(raw); i++) {
        std::string res;
        std::string pend;
        append(util::utf8_string_substr(raw, i, 1), res, pend);
    }
}

bool Key2KanaConvertor::processPseudoAsciiMode(const std::string &wstr) {
    for (unsigned int i = 0; i < wstr.length(); i++) {
        if (fcitx::charutils::isupper(wstr[i]) ||
            fcitx::charutils::isspace(wstr[i])) {
            isInPseudoAsciiMode_ = true;
        } else if (wstr[i] & 0x80) {
            isInPseudoAsciiMode_ = false;
        }
    }

    return isInPseudoAsciiMode_;
}

void Key2KanaConvertor::resetPseudoAsciiMode() {
    if (isInPseudoAsciiMode_) {
        pending_.clear();
    }
    isInPseudoAsciiMode_ = false;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
