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

typedef struct _RomajiKanaPair {
    const char *romaji_key;
    const char *kana_value;
    const char *extra_romaji_value;
    UT_hash_handle hh;
} RomajiKanaPair;

#define NR_ROMAJI_TYPING_RULE 235
#define NR_ROMAJI_DOUBLE_CONSONAT_TYPING_RULE 18
#define NR_ROMAJI_CORRECTION_TYPING_RULE 21

static const char *romaji_typing_rule[NR_ROMAJI_TYPING_RULE][2] = {
    {"-", "ー"},
    {"a", "あ"},
    {"i", "い"},
    {"", "う"},
    {"e", "え"},
    {"o", "お"},
    {"xa", "ぁ"},
    {"xi", "ぃ"},
    {"x", "ぅ"},
    {"xe", "ぇ"},
    {"xo", "ぉ"},
    {"la", "ぁ"},
    {"li", "ぃ"},
    {"l", "ぅ"},
    {"le", "ぇ"},
    {"lo", "ぉ"},
    {"wha", "うぁ"},
    {"whi", "うぃ"},
    {"whe", "うぇ"},
    {"who", "うぉ"},
    {"va", "ヴぁ"},
    {"vi", "ヴぃ"},
    {"v", "ヴ"},
    {"ve", "ヴぇ"},
    {"vo", "ヴぉ"},
    {"ka", "か"},
    {"ki", "き"},
    {"k", "く"},
    {"ke", "け"},
    {"ko", "こ"},
    {"lka", "ヵ"},
    {"lke", "ヶ"},
    {"xka", "ヵ"},
    {"xke", "ヶ"},
    {"ga", "が"},
    {"gi", "ぎ"},
    {"g", "ぐ"},
    {"ge", "げ"},
    {"go", "ご"},
    {"kya", "きゃ"},
    {"kyi", "きぃ"},
    {"ky", "きゅ"},
    {"kye", "きぇ"},
    {"kyo", "きょ"},
    {"kwa", "くぁ"},
    {"gya", "ぎゃ"},
    {"gyi", "ぎぃ"},
    {"gy", "ぎゅ"},
    {"gye", "ぎぇ"},
    {"gyo", "ぎょ"},
    {"gwa", "ぐぁ"},
    {"sa", "さ"},
    {"si", "し"},
    {"s", "す"},
    {"se", "せ"},
    {"so", "そ"},
    {"za", "ざ"},
    {"zi", "じ"},
    {"z", "ず"},
    {"ze", "ぜ"},
    {"zo", "ぞ"},
    {"sya", "しゃ"},
    {"syi", "しぃ"},
    {"sy", "しゅ"},
    {"sye", "しぇ"},
    {"syo", "しょ"},
    {"sha", "しゃ"},
    {"shi", "し"},
    {"sh", "しゅ"},
    {"she", "しぇ"},
    {"sho", "しょ"},
    {"zya", "じゃ"},
    {"zyi", "じぃ"},
    {"zy", "じゅ"},
    {"zye", "じぇ"},
    {"zyo", "じょ"},
    {"ja", "じゃ"},
    {"jya", "じゃ"},
    {"ji", "じ"},
    {"jyi", "じぃ"},
    {"j", "じゅ"},
    {"jy", "じゅ"},
    {"je", "じぇ"},
    {"jye", "じぇ"},
    {"jo", "じょ"},
    {"jyo", "じょ"},
    {"ta", "た"},
    {"ti", "ち"},
    {"t", "つ"},
    {"ts", "つ"},
    {"te", "て"},
    {"to", "と"},
    {"da", "だ"},
    {"di", "ぢ"},
    {"d", "づ"},
    {"de", "で"},
    {"do", "ど"},
    {"xt", "っ"},
    {"xts", "っ"},
    {"lt", "っ"},
    {"lts", "っ"},
    {"tya", "ちゃ"},
    {"tyi", "ちぃ"},
    {"ty", "ちゅ"},
    {"tye", "ちぇ"},
    {"tyo", "ちょ"},
    {"cya", "ちゃ"},
    {"cyi", "ちぃ"},
    {"cy", "ちゅ"},
    {"cye", "ちぇ"},
    {"cyo", "ちょ"},
    {"cha", "ちゃ"},
    {"chi", "ち"},
    {"ch", "ちゅ"},
    {"che", "ちぇ"},
    {"cho", "ちょ"},
    {"dya", "ぢゃ"},
    {"dyi", "ぢぃ"},
    {"dy", "ぢゅ"},
    {"dye", "ぢぇ"},
    {"dyo", "ぢょ"},
    {"tsa", "つぁ"},
    {"tsi", "つぃ"},
    {"tse", "つぇ"},
    {"tso", "つぉ"},
    {"tha", "てゃ"},
    {"thi", "てぃ"},
    {"th", "てゅ"},
    {"the", "てぇ"},
    {"tho", "てょ"},
    {"tw", "とぅ"},
    {"dha", "でゃ"},
    {"dhi", "でぃ"},
    {"dh", "でゅ"},
    {"dhe", "でぇ"},
    {"dho", "でょ"},
    {"dw", "どぅ"},
    {"na", "な"},
    {"ni", "に"},
    {"n", "ぬ"},
    {"ne", "ね"},
    {"no", "の"},
    {"nya", "にゃ"},
    {"nyi", "にぃ"},
    {"ny", "にゅ"},
    {"nye", "にぇ"},
    {"nyo", "にょ"},
    {"ha", "は"},
    {"hi", "ひ"},
    {"h", "ふ"},
    {"he", "へ"},
    {"ho", "ほ"},
    {"ba", "ば"},
    {"bi", "び"},
    {"b", "ぶ"},
    {"be", "べ"},
    {"bo", "ぼ"},
    {"pa", "ぱ"},
    {"pi", "ぴ"},
    {"p", "ぷ"},
    {"pe", "ぺ"},
    {"po", "ぽ"},
    {"hya", "ひゃ"},
    {"hyi", "ひぃ"},
    {"hy", "ひゅ"},
    {"hye", "ひぇ"},
    {"hyo", "ひょ"},
    {"bya", "びゃ"},
    {"byi", "びぃ"},
    {"by", "びゅ"},
    {"bye", "びぇ"},
    {"byo", "びょ"},
    {"pya", "ぴゃ"},
    {"pyi", "ぴぃ"},
    {"py", "ぴゅ"},
    {"pye", "ぴぇ"},
    {"pyo", "ぴょ"},
    {"fa", "ふぁ"},
    {"fi", "ふぃ"},
    {"f", "ふ"},
    {"fe", "ふぇ"},
    {"fo", "ふぉ"},
    {"fya", "ふゃ"},
    {"fyi", "ふぃ"},
    {"fy", "ふゅ"},
    {"fye", "ふぇ"},
    {"fyo", "ふょ"},
    {"ma", "ま"},
    {"mi", "み"},
    {"m", "む"},
    {"me", "め"},
    {"mo", "も"},
    {"mya", "みゃ"},
    {"myi", "みぃ"},
    {"my", "みゅ"},
    {"mye", "みぇ"},
    {"myo", "みょ"},
    {"ya", "や"},
    {"yi", "い"},
    {"y", "ゆ"},
    {"ye", "いぇ"},
    {"yo", "よ"},
    {"lya", "ゃ"},
    {"lyi", "ぃ"},
    {"ly", "ゅ"},
    {"lye", "ぇ"},
    {"lyo", "ょ"},
    {"xya", "ゃ"},
    {"xyi", "ぃ"},
    {"xy", "ゅ"},
    {"xye", "ぇ"},
    {"xyo", "ょ"},
    {"ra", "ら"},
    {"ri", "り"},
    {"r", "る"},
    {"re", "れ"},
    {"ro", "ろ"},
    {"rya", "りゃ"},
    {"ryi", "りぃ"},
    {"ry", "りゅ"},
    {"rye", "りぇ"},
    {"ryo", "りょ"},
    {"wa", "わ"},
    {"wi", "うぃ"},
    {"w", "う"},
    {"we", "うぇ"},
    {"wo", "を"},
    {"lwa", "ゎ"},
    {"xwa", "ゎ"},
    {"n'", "ん"},
    {"nn", "ん"},
    {"wyi", "ゐ"},
    {"wye", "ゑ"}
};

const char *romaji_double_consonat_typing_rule[NR_ROMAJI_DOUBLE_CONSONAT_TYPING_RULE][3] = {
    {"bb", "っ", "b"},
    {"cc", "っ", "c"},
    {"dd", "っ", "d"},
    {"ff", "っ", "f"},
    {"gg", "っ", "g"},
    {"hh", "っ", "h"},
    {"jj", "っ", "j"},
    {"kk", "っ", "k"},
    {"mm", "っ", "m"},
    {"pp", "っ", "p"},
    {"rr", "っ", "r"},
    {"ss", "っ", "s"},
    {"tt", "っ", "t"},
    {"vv", "っ", "v"},
    {"ww", "っ", "w"},
    {"xx", "っ", "x"},
    {"yy", "っ", "y"},
    {"zz", "っ", "z"}
};

const char *romaji_correction_typing_rule[NR_ROMAJI_CORRECTION_TYPING_RULE][3] = {
    {"nb", "ん", "b"},
    {"nc", "ん", "c"},
    {"nd", "ん", "d"},
    {"nf", "ん", "f"},
    {"ng", "ん", "g"},
    {"nh", "ん", "h"},
    {"nj", "ん", "j"},
    {"nk", "ん", "k"},
    {"nl", "ん", "l"},
    {"nm", "ん", "m"},
    {"np", "ん", "p"},
    {"nr", "ん", "r"},
    {"ns", "ん", "s"},
    {"nt", "ん", "t"},
    {"nv", "ん", "v"},
    {"nw", "ん", "w"},
    {"nx", "ん", "x"},
    {"nz", "ん", "z"},
    {"n\0", "ん", ""},
    {"n,", "ん", ","},
    {"n.", "ん", "."}
};

int FcitxAnthyInitTypingTables(struct _FcitxAnthy* anthy)
{
    int i;
    RomajiKanaPair *romaji_typing_rule_pairs = NULL;
    RomajiKanaPair *romaji_double_consonat_typing_rule_pairs = NULL;
    RomajiKanaPair *romaji_correction_typing_rule_pairs = NULL;
    for (i = 0; i < NR_ROMAJI_TYPING_RULE; i++) {
        RomajiKanaPair *pair = malloc(sizeof(RomajiKanaPair));
        pair->romaji_key = romaji_typing_rule[i][0];
        pair->kana_value = romaji_typing_rule[i][1];
        pair->extra_romaji_value = NULL;
        HASH_ADD_KEYPTR(hh, romaji_typing_rule_pairs, pair->romaji_key, strlen(pair->romaji_key), pair);
    }
    for (i = 0; i < NR_ROMAJI_DOUBLE_CONSONAT_TYPING_RULE; i++) {
        RomajiKanaPair *pair = malloc(sizeof(RomajiKanaPair));
        pair->romaji_key = romaji_double_consonat_typing_rule[i][0];
        pair->kana_value = romaji_double_consonat_typing_rule[i][1];
        pair->extra_romaji_value = romaji_double_consonat_typing_rule[i][2];
        HASH_ADD_KEYPTR(hh, romaji_double_consonat_typing_rule_pairs, pair->romaji_key, strlen(pair->romaji_key), pair);
    }
    for (i = 0; i < NR_ROMAJI_CORRECTION_TYPING_RULE; i++) {
        RomajiKanaPair *pair = malloc(sizeof(RomajiKanaPair));
        pair->romaji_key = romaji_correction_typing_rule[i][0];
        pair->kana_value = romaji_correction_typing_rule[i][1];
        pair->extra_romaji_value = romaji_correction_typing_rule[i][2];
        HASH_ADD_KEYPTR(hh, romaji_correction_typing_rule_pairs, pair->romaji_key, strlen(pair->romaji_key), pair);
    }

    anthy->romaji_typing_rule_pairs = romaji_typing_rule_pairs;
    anthy->romaji_double_consonat_typing_rule_pairs = romaji_double_consonat_typing_rule_pairs;
    anthy->romaji_correction_typing_rule_pairs = romaji_correction_typing_rule_pairs;

    return 0;
}

int FcitxAnthyLookupKanaForRomaji(struct _FcitxAnthy* anthy, const char *romaji, const char **kana, const char **extra_romaji)
{
    RomajiKanaPair *romaji_typing_rule_pairs = anthy->romaji_typing_rule_pairs;
    RomajiKanaPair *romaji_double_consonat_typing_rule_pairs = anthy->romaji_double_consonat_typing_rule_pairs;
    RomajiKanaPair *romaji_correction_typing_rule_pairs = anthy->romaji_correction_typing_rule_pairs;
    RomajiKanaPair *pair = NULL;
    HASH_FIND_STR(romaji_typing_rule_pairs, romaji, pair);
    if (pair) {
        *kana = pair->kana_value;
        *extra_romaji = NULL;
        return 0;
    }

    HASH_FIND_STR(romaji_double_consonat_typing_rule_pairs, romaji, pair);
    if (!pair)
        HASH_FIND_STR(romaji_correction_typing_rule_pairs, romaji, pair);
    if (!pair)
        return -1;
    else {
        *kana = pair->kana_value;
        *extra_romaji = pair->extra_romaji_value;
        return 0;
    }
}


int FcitxAnthyClearTypingTables(struct _FcitxAnthy* anthy)
{
    RomajiKanaPair *romaji_typing_rule_pairs = anthy->romaji_typing_rule_pairs;
    RomajiKanaPair *romaji_double_consonat_typing_rule_pairs = anthy->romaji_double_consonat_typing_rule_pairs;
    RomajiKanaPair *romaji_correction_typing_rule_pairs = anthy->romaji_correction_typing_rule_pairs;
    RomajiKanaPair *pair = NULL;
    while (romaji_typing_rule_pairs) {
        pair = romaji_typing_rule_pairs;
        HASH_DEL(romaji_typing_rule_pairs, pair);
        free(pair);
    }

    while (romaji_double_consonat_typing_rule_pairs) {
        pair = romaji_double_consonat_typing_rule_pairs;
        HASH_DEL(romaji_double_consonat_typing_rule_pairs, pair);
        free(pair);
    }

    while (romaji_correction_typing_rule_pairs) {
        pair = romaji_correction_typing_rule_pairs;
        HASH_DEL(romaji_correction_typing_rule_pairs, pair);
        free(pair);
    }
    return 0;
}


