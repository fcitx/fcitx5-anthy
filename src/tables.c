/***************************************************************************
 *   Copyright (C) 2010~2010 by Margaret Wang                              *
 *   pipituliuliu@gmail.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include <stdlib.h>
#include <fcitx-utils/uthash.h>
#include "eim.h"
#include "tables.h"
#include <fcitx-utils/log.h>

typedef struct _FcitxAnthyTypingRule {
    struct _RomajiKanaPair* romaji_typing_rule_pairs;
} FcitxAnthyTypingRule;

typedef struct _RomajiKanaPair {
    const char *romaji_key;
    const char *kana_value;
    UT_hash_handle hh;
} RomajiKanaPair;

#define NR_ROMAJI_TYPING_RULE (sizeof(romaji_typing_rule) / sizeof(romaji_typing_rule[0]))

static const char *romaji_typing_rule[][2] = {
    {"-", "ー"},
    {"a", "あ"},
    {"i", "い"},
    {"u", "う"},
    {"e", "え"},
    {"o", "お"},
    {"xa", "ぁ"},
    {"xi", "ぃ"},
    {"xu", "ぅ"},
    {"xe", "ぇ"},
    {"xo", "ぉ"},
    {"la", "ぁ"},
    {"li", "ぃ"},
    {"lu", "ぅ"},
    {"le", "ぇ"},
    {"lo", "ぉ"},
    {"wha", "うぁ"},
    {"whi", "うぃ"},
    {"whe", "うぇ"},
    {"who", "うぉ"},
    {"va", "ヴぁ"},
    {"vi", "ヴぃ"},
    {"vu", "ヴ"},
    {"ve", "ヴぇ"},
    {"vo", "ヴぉ"},
    {"ka", "か"},
    {"ki", "き"},
    {"ku", "く"},
    {"ke", "け"},
    {"ko", "こ"},
    {"lka", "ヵ"},
    {"lke", "ヶ"},
    {"xka", "ヵ"},
    {"xke", "ヶ"},
    {"ga", "が"},
    {"gi", "ぎ"},
    {"gu", "ぐ"},
    {"ge", "げ"},
    {"go", "ご"},
    {"kya", "きゃ"},
    {"kyi", "きぃ"},
    {"kyu", "きゅ"},
    {"kye", "きぇ"},
    {"kyo", "きょ"},
    {"kwa", "くぁ"},
    {"gya", "ぎゃ"},
    {"gyi", "ぎぃ"},
    {"gyu", "ぎゅ"},
    {"gye", "ぎぇ"},
    {"gyo", "ぎょ"},
    {"gwa", "ぐぁ"},
    {"sa", "さ"},
    {"si", "し"},
    {"su", "す"},
    {"se", "せ"},
    {"so", "そ"},
    {"za", "ざ"},
    {"zi", "じ"},
    {"zu", "ず"},
    {"ze", "ぜ"},
    {"zo", "ぞ"},
    {"sya", "しゃ"},
    {"syi", "しぃ"},
    {"syu", "しゅ"},
    {"sye", "しぇ"},
    {"syo", "しょ"},
    {"sha", "しゃ"},
    {"shi", "し"},
    {"shu", "しゅ"},
    {"she", "しぇ"},
    {"sho", "しょ"},
    {"zya", "じゃ"},
    {"zyi", "じぃ"},
    {"zyu", "じゅ"},
    {"zye", "じぇ"},
    {"zyo", "じょ"},
    {"ja", "じゃ"},
    {"jya", "じゃ"},
    {"ji", "じ"},
    {"jyi", "じぃ"},
    {"ju", "じゅ"},
    {"jyu", "じゅ"},
    {"je", "じぇ"},
    {"jye", "じぇ"},
    {"jo", "じょ"},
    {"jyo", "じょ"},
    {"ta", "た"},
    {"ti", "ち"},
    {"tu", "つ"},
    {"tsu", "つ"},
    {"te", "て"},
    {"to", "と"},
    {"da", "だ"},
    {"di", "ぢ"},
    {"du", "づ"},
    {"de", "で"},
    {"do", "ど"},
    {"xtu", "っ"},
    {"xtsu", "っ"},
    {"ltu", "っ"},
    {"ltsu", "っ"},
    {"tya", "ちゃ"},
    {"tyi", "ちぃ"},
    {"tyu", "ちゅ"},
    {"tye", "ちぇ"},
    {"tyo", "ちょ"},
    {"cya", "ちゃ"},
    {"cyi", "ちぃ"},
    {"cyu", "ちゅ"},
    {"cye", "ちぇ"},
    {"cyo", "ちょ"},
    {"cha", "ちゃ"},
    {"chi", "ち"},
    {"chu", "ちゅ"},
    {"che", "ちぇ"},
    {"cho", "ちょ"},
    {"dya", "ぢゃ"},
    {"dyi", "ぢぃ"},
    {"dyu", "ぢゅ"},
    {"dye", "ぢぇ"},
    {"dyo", "ぢょ"},
    {"tsa", "つぁ"},
    {"tsi", "つぃ"},
    {"tse", "つぇ"},
    {"tso", "つぉ"},
    {"tha", "てゃ"},
    {"thi", "てぃ"},
    {"thu", "てゅ"},
    {"the", "てぇ"},
    {"tho", "てょ"},
    {"twu", "とぅ"},
    {"dha", "でゃ"},
    {"dhi", "でぃ"},
    {"dhu", "でゅ"},
    {"dhe", "でぇ"},
    {"dho", "でょ"},
    {"dwu", "どぅ"},
    {"na", "な"},
    {"ni", "に"},
    {"nu", "ぬ"},
    {"ne", "ね"},
    {"no", "の"},
    {"nya", "にゃ"},
    {"nyi", "にぃ"},
    {"nyu", "にゅ"},
    {"nye", "にぇ"},
    {"nyo", "にょ"},
    {"ha", "は"},
    {"hi", "ひ"},
    {"hu", "ふ"},
    {"he", "へ"},
    {"ho", "ほ"},
    {"ba", "ば"},
    {"bi", "び"},
    {"bu", "ぶ"},
    {"be", "べ"},
    {"bo", "ぼ"},
    {"pa", "ぱ"},
    {"pi", "ぴ"},
    {"pu", "ぷ"},
    {"pe", "ぺ"},
    {"po", "ぽ"},
    {"hya", "ひゃ"},
    {"hyi", "ひぃ"},
    {"hyu", "ひゅ"},
    {"hye", "ひぇ"},
    {"hyo", "ひょ"},
    {"bya", "びゃ"},
    {"byi", "びぃ"},
    {"byu", "びゅ"},
    {"bye", "びぇ"},
    {"byo", "びょ"},
    {"pya", "ぴゃ"},
    {"pyi", "ぴぃ"},
    {"pyu", "ぴゅ"},
    {"pye", "ぴぇ"},
    {"pyo", "ぴょ"},
    {"fa", "ふぁ"},
    {"fi", "ふぃ"},
    {"fu", "ふ"},
    {"fe", "ふぇ"},
    {"fo", "ふぉ"},
    {"fya", "ふゃ"},
    {"fyi", "ふぃ"},
    {"fyu", "ふゅ"},
    {"fye", "ふぇ"},
    {"fyo", "ふょ"},
    {"ma", "ま"},
    {"mi", "み"},
    {"mu", "む"},
    {"me", "め"},
    {"mo", "も"},
    {"mya", "みゃ"},
    {"myi", "みぃ"},
    {"myu", "みゅ"},
    {"mye", "みぇ"},
    {"myo", "みょ"},
    {"ya", "や"},
    {"yi", "い"},
    {"yu", "ゆ"},
    {"ye", "いぇ"},
    {"yo", "よ"},
    {"lya", "ゃ"},
    {"lyi", "ぃ"},
    {"lyu", "ゅ"},
    {"lye", "ぇ"},
    {"lyo", "ょ"},
    {"xya", "ゃ"},
    {"xyi", "ぃ"},
    {"xyu", "ゅ"},
    {"xye", "ぇ"},
    {"xyo", "ょ"},
    {"ra", "ら"},
    {"ri", "り"},
    {"ru", "る"},
    {"re", "れ"},
    {"ro", "ろ"},
    {"rya", "りゃ"},
    {"ryi", "りぃ"},
    {"ry", "りゅ"},
    {"rye", "りぇ"},
    {"ryo", "りょ"},
    {"wa", "わ"},
    {"wi", "うぃ"},
    {"wu", "う"},
    {"we", "うぇ"},
    {"wo", "を"},
    {"lwa", "ゎ"},
    {"xwa", "ゎ"},
    {"n\'", "ん"},
    {"nn", "ん"},
    {"wyi", "ゐ"},
    {"wye", "ゑ"}
};


int FcitxAnthyInitTypingTables(struct _FcitxAnthy* anthy)
{
    int i;

    anthy->rule = fcitx_utils_malloc0(sizeof(FcitxAnthyTypingRule));

    RomajiKanaPair *romaji_typing_rule_pairs = NULL;
    FcitxLog(INFO, "%d", NR_ROMAJI_TYPING_RULE);
    for (i = 0; i < NR_ROMAJI_TYPING_RULE; i++) {
        RomajiKanaPair *pair = fcitx_utils_malloc0(sizeof(RomajiKanaPair));
        pair->romaji_key = romaji_typing_rule[i][0];
        pair->kana_value = romaji_typing_rule[i][1];
        HASH_ADD_KEYPTR(hh, romaji_typing_rule_pairs, pair->romaji_key, strlen(pair->romaji_key), pair);
    }

    anthy->rule->romaji_typing_rule_pairs = romaji_typing_rule_pairs;

    return 0;
}

int FcitxAnthyLookupKanaForRomaji(struct _FcitxAnthy* anthy, const char *romaji, const char **kana)
{
    RomajiKanaPair *romaji_typing_rule_pairs = anthy->rule->romaji_typing_rule_pairs;
    RomajiKanaPair *pair = NULL;

    *kana = NULL;

    HASH_FIND_STR(romaji_typing_rule_pairs, romaji, pair);
    if (pair) {
        *kana = pair->kana_value;
        return 0;
    }
    return -1;
}


int FcitxAnthyClearTypingTables(struct _FcitxAnthy* anthy)
{
    RomajiKanaPair *romaji_typing_rule_pairs = anthy->rule->romaji_typing_rule_pairs;
    RomajiKanaPair *pair = NULL;
    while (romaji_typing_rule_pairs) {
        pair = romaji_typing_rule_pairs;
        HASH_DEL(romaji_typing_rule_pairs, pair);
        free(pair);
    }

    free(anthy->rule);

    return 0;
}


