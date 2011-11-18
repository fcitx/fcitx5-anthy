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

#include <fcitx-util/uthash.h>

typedef struct {
    const char *romaji_key;
    const char *kana_value;
    const char *extra_romaji_value;
    UT_hash_handle hh;
} RomajiKanaPair;

#define NR_ROMAJI_TYPING_RULE 235
#define NR_ROMAJI_DOUBLE_CONSONAT_TYPING_RULE 18
#define NR_ROMAJI_CORRECTION_TYPING_RULE 21

static const char *romaji_typing_rule[NR_ROMAJI_TYPING_RULE][2] = {
    {u"-", u"ー"},
    {u"a", u"あ"},
    {u"i", u"い"},
    {u"u", u"う"},
    {u"e", u"え"},
    {u"o", u"お"},
    {u"xa", u"ぁ"},
    {u"xi", u"ぃ"},
    {u"xu", u"ぅ"},
    {u"xe", u"ぇ"},
    {u"xo", u"ぉ"},
    {u"la", u"ぁ"},
    {u"li", u"ぃ"},
    {u"lu", u"ぅ"},
    {u"le", u"ぇ"},
    {u"lo", u"ぉ"},
    {u"wha", u"うぁ"},
    {u"whi", u"うぃ"},
    {u"whe", u"うぇ"},
    {u"who", u"うぉ"},
    {u"va", u"ヴぁ"},
    {u"vi", u"ヴぃ"},
    {u"vu", u"ヴ"},
    {u"ve", u"ヴぇ"},
    {u"vo", u"ヴぉ"},
    {u"ka", u"か"},
    {u"ki", u"き"},
    {u"ku", u"く"},
    {u"ke", u"け"},
    {u"ko", u"こ"},
    {u"lka", u"ヵ"},
    {u"lke", u"ヶ"},
    {u"xka", u"ヵ"},
    {u"xke", u"ヶ"},
    {u"ga", u"が"},
    {u"gi", u"ぎ"},
    {u"gu", u"ぐ"},
    {u"ge", u"げ"},
    {u"go", u"ご"},
    {u"kya", u"きゃ"},
    {u"kyi", u"きぃ"},
    {u"kyu", u"きゅ"},
    {u"kye", u"きぇ"},
    {u"kyo", u"きょ"},
    {u"kwa", u"くぁ"},
    {u"gya", u"ぎゃ"},
    {u"gyi", u"ぎぃ"},
    {u"gyu", u"ぎゅ"},
    {u"gye", u"ぎぇ"},
    {u"gyo", u"ぎょ"},
    {u"gwa", u"ぐぁ"},
    {u"sa", u"さ"},
    {u"si", u"し"},
    {u"su", u"す"},
    {u"se", u"せ"},
    {u"so", u"そ"},
    {u"za", u"ざ"},
    {u"zi", u"じ"},
    {u"zu", u"ず"},
    {u"ze", u"ぜ"},
    {u"zo", u"ぞ"},
    {u"sya", u"しゃ"},
    {u"syi", u"しぃ"},
    {u"syu", u"しゅ"},
    {u"sye", u"しぇ"},
    {u"syo", u"しょ"},
    {u"sha", u"しゃ"},
    {u"shi", u"し"},
    {u"shu", u"しゅ"},
    {u"she", u"しぇ"},
    {u"sho", u"しょ"},
    {u"zya", u"じゃ"},
    {u"zyi", u"じぃ"},
    {u"zyu", u"じゅ"},
    {u"zye", u"じぇ"},
    {u"zyo", u"じょ"},
    {u"ja", u"じゃ"},
    {u"jya", u"じゃ"},
    {u"ji", u"じ"},
    {u"jyi", u"じぃ"},
    {u"ju", u"じゅ"},
    {u"jyu", u"じゅ"},
    {u"je", u"じぇ"},
    {u"jye", u"じぇ"},
    {u"jo", u"じょ"},
    {u"jyo", u"じょ"},
    {u"ta", u"た"},
    {u"ti", u"ち"},
    {u"tu", u"つ"},
    {u"tsu", u"つ"},
    {u"te", u"て"},
    {u"to", u"と"},
    {u"da", u"だ"},
    {u"di", u"ぢ"},
    {u"du", u"づ"},
    {u"de", u"で"},
    {u"do", u"ど"},
    {u"xtu", u"っ"},
    {u"xtsu", u"っ"},
    {u"ltu", u"っ"},
    {u"ltsu", u"っ"},
    {u"tya", u"ちゃ"},
    {u"tyi", u"ちぃ"},
    {u"tyu", u"ちゅ"},
    {u"tye", u"ちぇ"},
    {u"tyo", u"ちょ"},
    {u"cya", u"ちゃ"},
    {u"cyi", u"ちぃ"},
    {u"cyu", u"ちゅ"},
    {u"cye", u"ちぇ"},
    {u"cyo", u"ちょ"},
    {u"cha", u"ちゃ"},
    {u"chi", u"ち"},
    {u"chu", u"ちゅ"},
    {u"che", u"ちぇ"},
    {u"cho", u"ちょ"},
    {u"dya", u"ぢゃ"},
    {u"dyi", u"ぢぃ"},
    {u"dyu", u"ぢゅ"},
    {u"dye", u"ぢぇ"},
    {u"dyo", u"ぢょ"},
    {u"tsa", u"つぁ"},
    {u"tsi", u"つぃ"},
    {u"tse", u"つぇ"},
    {u"tso", u"つぉ"},
    {u"tha", u"てゃ"},
    {u"thi", u"てぃ"},
    {u"thu", u"てゅ"},
    {u"the", u"てぇ"},
    {u"tho", u"てょ"},
    {u"twu", u"とぅ"},
    {u"dha", u"でゃ"},
    {u"dhi", u"でぃ"},
    {u"dhu", u"でゅ"},
    {u"dhe", u"でぇ"},
    {u"dho", u"でょ"},
    {u"dwu", u"どぅ"},
    {u"na", u"な"},
    {u"ni", u"に"},
    {u"nu", u"ぬ"},
    {u"ne", u"ね"},
    {u"no", u"の"},
    {u"nya", u"にゃ"},
    {u"nyi", u"にぃ"},
    {u"nyu", u"にゅ"},
    {u"nye", u"にぇ"},
    {u"nyo", u"にょ"},
    {u"ha", u"は"},
    {u"hi", u"ひ"},
    {u"hu", u"ふ"},
    {u"he", u"へ"},
    {u"ho", u"ほ"},
    {u"ba", u"ば"},
    {u"bi", u"び"},
    {u"bu", u"ぶ"},
    {u"be", u"べ"},
    {u"bo", u"ぼ"},
    {u"pa", u"ぱ"},
    {u"pi", u"ぴ"},
    {u"pu", u"ぷ"},
    {u"pe", u"ぺ"},
    {u"po", u"ぽ"},
    {u"hya", u"ひゃ"},
    {u"hyi", u"ひぃ"},
    {u"hyu", u"ひゅ"},
    {u"hye", u"ひぇ"},
    {u"hyo", u"ひょ"},
    {u"bya", u"びゃ"},
    {u"byi", u"びぃ"},
    {u"byu", u"びゅ"},
    {u"bye", u"びぇ"},
    {u"byo", u"びょ"},
    {u"pya", u"ぴゃ"},
    {u"pyi", u"ぴぃ"},
    {u"pyu", u"ぴゅ"},
    {u"pye", u"ぴぇ"},
    {u"pyo", u"ぴょ"},
    {u"fa", u"ふぁ"},
    {u"fi", u"ふぃ"},
    {u"fu", u"ふ"},
    {u"fe", u"ふぇ"},
    {u"fo", u"ふぉ"},
    {u"fya", u"ふゃ"},
    {u"fyi", u"ふぃ"},
    {u"fyu", u"ふゅ"},
    {u"fye", u"ふぇ"},
    {u"fyo", u"ふょ"},
    {u"ma", u"ま"},
    {u"mi", u"み"},
    {u"mu", u"む"},
    {u"me", u"め"},
    {u"mo", u"も"},
    {u"mya", u"みゃ"},
    {u"myi", u"みぃ"},
    {u"myu", u"みゅ"},
    {u"mye", u"みぇ"},
    {u"myo", u"みょ"},
    {u"ya", u"や"},
    {u"yi", u"い"},
    {u"yu", u"ゆ"},
    {u"ye", u"いぇ"},
    {u"yo", u"よ"},
    {u"lya", u"ゃ"},
    {u"lyi", u"ぃ"},
    {u"lyu", u"ゅ"},
    {u"lye", u"ぇ"},
    {u"lyo", u"ょ"},
    {u"xya", u"ゃ"},
    {u"xyi", u"ぃ"},
    {u"xyu", u"ゅ"},
    {u"xye", u"ぇ"},
    {u"xyo", u"ょ"},
    {u"ra", u"ら"},
    {u"ri", u"り"},
    {u"ru", u"る"},
    {u"re", u"れ"},
    {u"ro", u"ろ"},
    {u"rya", u"りゃ"},
    {u"ryi", u"りぃ"},
    {u"ryu", u"りゅ"},
    {u"rye", u"りぇ"},
    {u"ryo", u"りょ"},
    {u"wa", u"わ"},
    {u"wi", u"うぃ"},
    {u"wu", u"う"},
    {u"we", u"うぇ"},
    {u"wo", u"を"},
    {u"lwa", u"ゎ"},
    {u"xwa", u"ゎ"},
    {u"n'", u"ん"},
    {u"nn", u"ん"},
    {u"wyi", u"ゐ"},
    {u"wye", u"ゑ"}
};

const char *romaji_double_consonat_typing_rule[NR_ROMAJI_DOUBLE_CONSONAT_TYPING_RULE][3] = {
    {u"bb", u"っ", u"b"},
    {u"cc", u"っ", u"c"},
    {u"dd", u"っ", u"d"},
    {u"ff", u"っ", u"f"},
    {u"gg", u"っ", u"g"},
    {u"hh", u"っ", u"h"},
    {u"jj", u"っ", u"j"},
    {u"kk", u"っ", u"k"},
    {u"mm", u"っ", u"m"},
    {u"pp", u"っ", u"p"},
    {u"rr", u"っ", u"r"},
    {u"ss", u"っ", u"s"},
    {u"tt", u"っ", u"t"},
    {u"vv", u"っ", u"v"},
    {u"ww", u"っ", u"w"},
    {u"xx", u"っ", u"x"},
    {u"yy", u"っ", u"y"},
    {u"zz", u"っ", u"z"}
};

const char *romaji_correction_rule[NR_ROMAJI_CORRECTION_TYPING_RULE][3] = {
    {u"nb", u"ん", u"b"},
    {u"nc", u"ん", u"c"},
    {u"nd", u"ん", u"d"},
    {u"nf", u"ん", u"f"},
    {u"ng", u"ん", u"g"},
    {u"nh", u"ん", u"h"},
    {u"nj", u"ん", u"j"},
    {u"nk", u"ん", u"k"},
    {u"nl", u"ん", u"l"},
    {u"nm", u"ん", u"m"},
    {u"np", u"ん", u"p"},
    {u"nr", u"ん", u"r"},
    {u"ns", u"ん", u"s"},
    {u"nt", u"ん", u"t"},
    {u"nv", u"ん", u"v"},
    {u"nw", u"ん", u"w"},
    {u"nx", u"ん", u"x"},
    {u"nz", u"ん", u"z"},
    {u"n\0", u"ん", u""},
    {u"n,", u"ん", u","},
    {u"n.", u"ん", u"."}
}

static RomajiKanaPair *romaji_typing_rule_pairs = NULL;
static RomajiKanaPair *romaji_double_consonat_typing_rule_pairs = NULL;
static RomajiKanaPair *romaji_correction_typing_rule_pairs = NULL;

int init_typing_tables()
{
    int i;
    for (i = 0; i < NR_ROMAJI_TYPING_RULE; i++) {
        RomajiKanaPair *pair = malloc(sizeof(RomajiKanaPair));
        pair->romaji_key = romaji_typing_rule[i][0];
        pair->kana_value = romaji_typing_rule[i][1];
        pair->extra_romaji_value = NULL;
        HASH_ADD_KEYPTR(romaji_typing_rule_pairs, romaji_key, pair);
    }
    for (i = 0; i < NR_ROMAJI_DOUBLE_CONSONAT_TYPING_RULE; i++) {
        RomajiKanaPair *pair = malloc(sizeof(RomajiKanaPair));
        pair->romaji_key = romaji_double_consonat_typing_rule[i][0];
        pair->kana_value = romaji_double_consonat_typing_rule[i][1];
        pair->extra_romaji_value = romaji_double_consonat_typing_rule[i][2];
        HASH_ADD_KEYPTR(romaji_double_consonat_typing_rule_pairs, romaji_key, pair);
    }
    for (i = 0; i < NR_ROMAJI_CORRECTION_TYPING_RULE; i++) {
        RomajiKanaPair *pair = malloc(sizeof(RomajiKanaPair));
        pair->romaji_key = romaji_correction_typing_rule[i][0];
        pair->kana_value = romaji_correction_typing_rule[i][1];
        pair->extra_romaji_value = romaji_correction_typing_rule[i][2];
        HASH_ADD_KEYPTR(romaji_correction_typing_rule_pairs, romaji_key, pair);
    }
    return 0;
}

int lookup_kana_for_romaji(const char *romaji, char **kana, char **extra_romaji)
{
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


int clear_typing_tables()
{
    struct RomajiKanaPair *pair, *tmp;
    HASH_ITER(hh, romaji_typing_rule_pairs, pair, tmp) {
        HASH_DEL(romaji_typing_rule_pairs, pair);
        free(pair);
    }
    HASH_ITER(hh, romaji_double_consonat_typing_rule_pairs, pair, tmp) {
        HASH_DEL(romaji_double_consonat_typing_rule_pairs, pair);
        free(pair);
    }
    HASH_ITER(hh, romaji_correction_typing_rule_pairs, pair, tmp) {
        HASH_DEL(romaji_correction_typing_rule_pairs, pair);
        free(pair);
    }
    return 0;
}


