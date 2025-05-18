/*
 *  SPDX-FileCopyrightText: 2023 CSSlayer <wengxt@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "style_file.h"
#include "testdir.h"
#include <cstddef>
#include <fcitx-utils/log.h>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

int main() {
    StyleFile style;
    style.load(TESTING_SOURCE_DIR "/profile/101kana.sty");
    FCITX_ASSERT(style.title() == "101英語キーボード用かな配列");
    constexpr std::string_view section = "KanaTable/FundamentalTable";
    auto keys = style.getKeyList(section);
    FCITX_ASSERT(keys);
    std::vector<std::tuple<std::string, std::vector<std::string>>>
        expectedValues = {
            {"~", {"ろ"}},     {"1", {"ぬ"}},     {"2", {"", "ふ"}},
            {"3", {"あ"}},     {"4", {"う"}},     {"5", {"え"}},
            {"6", {"お"}},     {"7", {"や"}},     {"8", {"ゆ"}},
            {"9", {"よ"}},     {"0", {"わ"}},     {"-", {"", "ほ"}},
            {"=", {"", "へ"}}, {"q", {"", "た"}}, {"w", {"", "て"}},
            {"e", {"い"}},     {"r", {"", "す"}}, {"t", {"", "か"}},
            {"y", {"ん"}},     {"u", {"な"}},     {"i", {"に"}},
            {"o", {"ら"}},     {"p", {"", "せ"}}, {"[", {"゛"}},
            {"]", {"゜"}},     {"\\", {"む"}},    {"a", {"", "ち"}},
            {"s", {"", "と"}}, {"d", {"", "し"}}, {"f", {"", "は"}},
            {"g", {"", "き"}}, {"h", {"", "く"}}, {"j", {"ま"}},
            {"k", {"の"}},     {"l", {"り"}},     {";", {"れ"}},
            {"'", {"", "け"}}, {"z", {"", "つ"}}, {"x", {"", "さ"}},
            {"c", {"", "そ"}}, {"v", {"", "ひ"}}, {"b", {"", "こ"}},
            {"n", {"み"}},     {"m", {"も"}},     {",", {"ね"}},
            {".", {"る"}},     {"/", {"め"}},     {"`", {"ろ"}},
            {"!", {"ぬ"}},     {"@", {"", "ふ"}}, {"#", {"ぁ"}},
            {"$", {"ぅ"}},     {"%", {"ぇ"}},     {"^", {"ぉ"}},
            {"&", {"ゃ"}},     {"*", {"ゅ"}},     {"(", {"ょ"}},
            {")", {"を"}},     {"_", {"ー"}},     {"+", {"ゑ"}},
            {"Q", {"", "た"}}, {"W", {"", "て"}}, {"E", {"ぃ"}},
            {"R", {"", "す"}}, {"T", {"ヵ"}},     {"Y", {"ん"}},
            {"U", {"な"}},     {"I", {"に"}},     {"O", {"ら"}},
            {"P", {"", "せ"}}, {"{", {"「"}},     {"}", {"」"}},
            {"|", {"む"}},     {"A", {"", "ち"}}, {"S", {"", "と"}},
            {"D", {"", "し"}}, {"F", {"", "ゎ"}}, {"G", {"", "き"}},
            {"H", {"", "く"}}, {"J", {"ま"}},     {"K", {"の"}},
            {"L", {"り"}},     {":", {"れ"}},     {"\"", {"ヶ"}},
            {"Z", {"っ"}},     {"X", {"", "さ"}}, {"C", {"", "そ"}},
            {"V", {"", "ゐ"}}, {"B", {"", "こ"}}, {"N", {"み"}},
            {"M", {"も"}},     {"?", {"・"}},

        };
    FCITX_ASSERT(expectedValues.size() == keys->size());
    for (size_t i = 0; i < keys->size(); i++) {
        FCITX_ASSERT((*keys)[i] == std::get<0>(expectedValues[i]))
            << (*keys)[i] << " " << expectedValues[i];
        auto values = style.getStringArray(section, (*keys)[i]);
        FCITX_ASSERT(values);
        FCITX_ASSERT(*values == std::get<1>(expectedValues[i]));
    }

    return 0;
}
