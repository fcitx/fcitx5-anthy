/*
 * SPDX-FileCopyrightText: 2017-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_ANTHY_CONFIG_H_
#define _FCITX5_ANTHY_CONFIG_H_

#include <fcitx-config/configuration.h>
#include <fcitx-config/enum.h>
#include <fcitx-utils/i18n.h>
#include <fcitx/candidatelist.h>

namespace fcitx {
FCITX_CONFIG_ENUM_NAME_WITH_I18N(CandidateLayoutHint, N_("Not Set"),
                                 N_("Vertical"), N_("Horizontal"));
}

enum class InputMode {
    HIRAGANA,
    KATAKANA,
    HALF_KATAKANA,
    LATIN,
    WIDE_LATIN,
    LAST
};

FCITX_CONFIG_ENUM_NAME_WITH_I18N(InputMode, N_("Hiragana"), N_("Katakana"),
                                 N_("Half Katakana"), N_("Latin"),
                                 N_("Wide Latin"));

enum class SymbolStyle {
    JAPANESE,
    WIDEBRACKET_WIDESLASH,
    CORNERBRACKET_MIDDLEDOT,
    CORNERBRACKET_WIDESLASH,
    LAST
};

FCITX_CONFIG_ENUM_NAME_WITH_I18N(SymbolStyle, N_("Japanese"),
                                 N_("Wide bracket and wide slash"),
                                 N_("Corner bracket and Middle Dot"),
                                 N_("Corner bracket and wide slash"));

enum class PeriodCommaStyle { WIDELATIN, LATIN, JAPANESE, WIDELATIN_JAPANESE };

FCITX_CONFIG_ENUM_NAME_WITH_I18N(PeriodCommaStyle, N_("Wide latin"),
                                 N_("Latin"), N_("Japanese"),
                                 N_("Wide latin Japanese"));

enum class ConversionMode {
    MULTI_SEGMENT,
    SINGLE_SEGMENT,
    MULTI_SEGMENT_IMMEDIATE,
    SINGLE_SEGMENT_IMMEDIATE
};

FCITX_CONFIG_ENUM_NAME_WITH_I18N(ConversionMode, N_("Multi segment"),
                                 N_("Single segment"),
                                 N_("Multi segment immediate"),
                                 N_("Single segment immediate"));

enum class TypingMethod { ROMAJI, KANA, NICOLA };

FCITX_CONFIG_ENUM_NAME_WITH_I18N(TypingMethod, N_("Romaji"), N_("Kana"),
                                 N_("Nicola"));

enum class SpaceType { FOLLOWMODE, WIDE };

FCITX_CONFIG_ENUM_NAME_WITH_I18N(SpaceType, N_("Follow mode"), N_("Wide"));

enum class TenKeyType {
    WIDE,
    HALF,
    FOLLOWMODE,
};

FCITX_CONFIG_ENUM_NAME_WITH_I18N(TenKeyType, N_("Wide"), N_("Half"),
                                 N_("Follow mode"));

struct AnthyKeyProfile {

    fcitx::KeyList hk_CONVERT;
    fcitx::KeyList hk_PREDICT;
    // candidates keys
    fcitx::KeyList hk_CANDIDATES_PAGE_UP;
    fcitx::KeyList hk_CANDIDATES_PAGE_DOWN;
    fcitx::KeyList hk_SELECT_FIRST_CANDIDATE;
    fcitx::KeyList hk_SELECT_LAST_CANDIDATE;
    fcitx::KeyList hk_SELECT_NEXT_CANDIDATE;
    fcitx::KeyList hk_SELECT_PREV_CANDIDATE;
    fcitx::KeyList hk_SELECT_NEXT_CANDIDATE_ALTER;
    fcitx::KeyList hk_SELECT_PREV_CANDIDATE_ALTER;

    // segment keys
    fcitx::KeyList hk_SELECT_FIRST_SEGMENT;
    fcitx::KeyList hk_SELECT_LAST_SEGMENT;
    fcitx::KeyList hk_SELECT_NEXT_SEGMENT;
    fcitx::KeyList hk_SELECT_PREV_SEGMENT;
    fcitx::KeyList hk_SHRINK_SEGMENT;
    fcitx::KeyList hk_EXPAND_SEGMENT;
    fcitx::KeyList hk_COMMIT_FIRST_SEGMENT;
    fcitx::KeyList hk_COMMIT_SELECTED_SEGMENT;
    fcitx::KeyList hk_COMMIT_FIRST_SEGMENT_REVERSE_LEARN;
    fcitx::KeyList hk_COMMIT_SELECTED_SEGMENT_REVERSE_LEARN;

    // direct convert keys
    fcitx::KeyList hk_CONV_CHAR_TYPE_FORWARD;
    fcitx::KeyList hk_CONV_CHAR_TYPE_BACKWARD;
    fcitx::KeyList hk_CONV_TO_HIRAGANA;
    fcitx::KeyList hk_CONV_TO_KATAKANA;
    fcitx::KeyList hk_CONV_TO_HALF;
    fcitx::KeyList hk_CONV_TO_HALF_KATAKANA;
    fcitx::KeyList hk_CONV_TO_LATIN;
    fcitx::KeyList hk_CONV_TO_WIDE_LATIN;

    // pseudo ascii mode
    fcitx::KeyList hk_CANCEL_PSEUDO_ASCII_MODE;

    // caret keys
    fcitx::KeyList hk_MOVE_CARET_FIRST;
    fcitx::KeyList hk_MOVE_CARET_LAST;
    fcitx::KeyList hk_MOVE_CARET_FORWARD;
    fcitx::KeyList hk_MOVE_CARET_BACKWARD;

    // edit keys
    fcitx::KeyList hk_BACKSPACE;
    fcitx::KeyList hk_DELETE;
    fcitx::KeyList hk_COMMIT;
    fcitx::KeyList hk_COMMIT_REVERSE_LEARN;
    fcitx::KeyList hk_CANCEL;
    fcitx::KeyList hk_CANCEL_ALL;
    fcitx::KeyList hk_INSERT_SPACE;
    fcitx::KeyList hk_INSERT_ALT_SPACE;
    fcitx::KeyList hk_INSERT_HALF_SPACE;
    fcitx::KeyList hk_INSERT_WIDE_SPACE;

    // mode keys
    fcitx::KeyList hk_CIRCLE_INPUT_MODE;
    fcitx::KeyList hk_CIRCLE_KANA_MODE;
    fcitx::KeyList hk_CIRCLE_LATIN_HIRAGANA_MODE;
    fcitx::KeyList hk_CIRCLE_TYPING_METHOD;
    fcitx::KeyList hk_LATIN_MODE;
    fcitx::KeyList hk_WIDE_LATIN_MODE;
    fcitx::KeyList hk_HIRAGANA_MODE;
    fcitx::KeyList hk_KATAKANA_MODE;
    fcitx::KeyList hk_HALF_KATAKANA_MODE;

    // dict keys
    fcitx::KeyList hk_DICT_ADMIN;
    fcitx::KeyList hk_ADD_WORD;

    // reconvert
    fcitx::KeyList hk_RECONVERT;
};

FCITX_CONFIGURATION(
    AnthyGeneralConfig,
    fcitx::Option<int, fcitx::IntConstrain> pageSize{
        this, "PageSize", _("Page size"), 10, fcitx::IntConstrain(3, 10)};
    fcitx::OptionWithAnnotation<fcitx::CandidateLayoutHint,
                                fcitx::CandidateLayoutHintI18NAnnotation>
        candidateLayout{this, "CandidateLayout", _("Candidate List Layout"),
                        fcitx::CandidateLayoutHint::Vertical};
    fcitx::Option<int, fcitx::IntConstrain> nTriggersToShowCandWin{
        this, "NTriggersToShowCandWin",
        _("Number candidate of Triggers To Show Candidate Window"), 2,
        fcitx::IntConstrain(0, 7)};
    fcitx::Option<bool> predictOnInput{this, "PredictOnInput",
                                       _("Predict on input"), false};
    fcitx::Option<bool> romajiPseudoAsciiBlankBehavior{
        this, "RomajiPseudoAsciiBlankBehavior",
        _("Insert a blank with a blank key"), true};
    fcitx::Option<bool> romajiPseudoAsciiMode{
        this, "RomajiPseudoAsciiMode",
        _("Entering the pseudo ASCII input mode with capital letters"), true};
    fcitx::Option<bool> romajiHalfSymbol{
        this, "RomajiHalfSymbol", _("Use half-width characters for symbols"),
        true};
    fcitx::Option<bool> romajiHalfNumber{
        this, "RomajiHalfNumber", _("Use half-width characters for numbers"),
        true};
    fcitx::OptionWithAnnotation<InputMode, InputModeI18NAnnotation> inputMode{
        this, "InputMode", _("Input mode"), InputMode::HIRAGANA};
    fcitx::OptionWithAnnotation<TypingMethod, TypingMethodI18NAnnotation>
        typingMethod{this, "TypingMethod", _("Typing method"),
                     TypingMethod::ROMAJI};
    fcitx::OptionWithAnnotation<ConversionMode, ConversionModeI18NAnnotation>
        conversionMode{this, "ConversionMode", _("Conversion mode"),
                       ConversionMode::MULTI_SEGMENT};
    fcitx::OptionWithAnnotation<PeriodCommaStyle,
                                PeriodCommaStyleI18NAnnotation>
        periodCommaStyle{this, "PeriodStyle", _("Period style"),
                         PeriodCommaStyle::JAPANESE};
    fcitx::OptionWithAnnotation<SymbolStyle, SymbolStyleI18NAnnotation>
        symbolStyle{this, "SymbolStyle", _("Symbol style"),
                    SymbolStyle::JAPANESE};
    fcitx::OptionWithAnnotation<SpaceType, SpaceTypeI18NAnnotation> spaceType{
        this, "SpaceType", _("Space type"), SpaceType::FOLLOWMODE};
    fcitx::OptionWithAnnotation<TenKeyType, TenKeyTypeI18NAnnotation>
        tenKeyType{this, "TenKeyType", _("Ten key type"),
                   TenKeyType::FOLLOWMODE};
    fcitx::Option<bool> learnOnManualCommit{this, "LearnOnManualCommit",
                                            _("Learn on manual commit"), true};
    fcitx::Option<bool> learnOnAutoCommit{this, "LearnOnAutoCommit",
                                          _("Learn on auto commit"), true};
    fcitx::Option<bool> romajiAllowSplit{this, "AllowSplit", _("Allow split"),
                                         true};
    fcitx::Option<bool> useDirectKeyOnPredict{
        this, "UseDirectKeyOnPredict", _("Use direct key on predict"), true};
    fcitx::Option<bool> showCandidatesLabel{this, "ShowCandidatesLabel",
                                            _("Show candidates label"), true};);

FCITX_CONFIGURATION(
    AnthyInterfaceConfig,
    fcitx::Option<bool> showInputModeLabel{this, "ShowInputMode",
                                           _("Show input mode"), true};
    fcitx::Option<bool> showTypingMethodLabel{this, "ShowTypingMethod",
                                              _("Show typing method"), true};
    fcitx::Option<bool> showConvModeLabel{this, "ShowConversionMode",
                                          _("Show conversion mode"), true};
    fcitx::Option<bool> showPeriodStyleLabel{this, "ShowPeriodStyle",
                                             _("Show period style"), false};
    fcitx::Option<bool> showSymbolStyleLabel{
        this, "ShowSymbolStyle", _("Show candidates label"), false};);

enum class KeyBindingProfile {
    Default,
    Atok,
    Canna,
    MSIME,
    VjeDelta,
    WNN,
    Custom
};
FCITX_CONFIG_ENUM_NAME_WITH_I18N(KeyBindingProfile, N_("Default"), N_("Atok"),
                                 N_("Canna"), N_("MS IME"), N_("Vje Delta"),
                                 N_("WNN"), N_("Custom"));

enum class RomajiTable {
    Default,
    Atok,
    Azik,
    Canna,
    MSIME,
    VjeDelta,
    WNN,
    Custom
};

FCITX_CONFIG_ENUM_NAME_WITH_I18N(RomajiTable, N_("Default"), N_("Atok"),
                                 N_("Azik"), N_("Canna"), N_("MS IME"),
                                 N_("Vje Delta"), N_("WNN"), N_("Custom"));

enum class KanaTable {
    Default,
    _101Kana,
    Tsuki2_203_101,
    Tsuki2_203_106,
    QuickKana,
    Custom,
};

FCITX_CONFIG_ENUM_NAME_WITH_I18N(KanaTable, N_("Default"), N_("101kana"),
                                 N_("tsuki-2-203-101"), N_("tsuki-2-203-106"),
                                 N_("Quick Kana"), N_("Custom"));

enum class NicolaTable {
    Default,
    Nicola_A,
    Nicola_F,
    Nicola_J,
    Oasys100J,
    TronDvorak,
    TronQwertyJP,
    Custom
};

FCITX_CONFIG_ENUM_NAME_WITH_I18N(NicolaTable, N_("Default"), N_("Nicola-A"),
                                 N_("Nicola-F"), N_("Nicola-J"),
                                 N_("Oasys100J"), N_("Tron Dvorak"),
                                 N_("Tron Qwerty JP"), N_("Custom"));

FCITX_CONFIGURATION(
    AnthyKeyProfileConfig,
    fcitx::OptionWithAnnotation<KeyBindingProfile,
                                KeyBindingProfileI18NAnnotation>
        keyProfileEnum{this, "KeyBindingProfile", _("Key binding profile"),
                       KeyBindingProfile::Default};
    fcitx::OptionWithAnnotation<RomajiTable, RomajiTableI18NAnnotation>
        romajiTableEnum{this, "RomajiTable", _("Romaji Table"),
                        RomajiTable::Default};
    fcitx::OptionWithAnnotation<KanaTable, KanaTableI18NAnnotation>
        kanaTableEnum{this, "KanaTable", _("Kana Table"), KanaTable::Default};
    fcitx::OptionWithAnnotation<NicolaTable, NicolaTableI18NAnnotation>
        nicolaTableEnum{this, "NicolaTable", _("Nicola Table"),
                        NicolaTable::Default};
    fcitx::Option<std::string> keyThemeFile{this, "CustomKeyBinding",
                                            _("Custom Key Binding")};
    fcitx::Option<std::string> romajiFundamentalTable{this, "CustomRomajiTable",
                                                      _("Custom Romaji Table")};
    fcitx::Option<std::string> kanaFundamentalTable{this, "CustomKanaTable",
                                                    _("Custom Kana Table")};
    fcitx::Option<std::string> nicolaFundamentalTable{
        this, "CustomNicolaTable", _("Custom Nicola Table")};);

FCITX_CONFIGURATION(
    AnthyKeyConfig,
    fcitx::KeyListOption hk_CIRCLE_INPUT_MODE{
        this,
        "CircleInputModeKey",
        _("Circle input mode"),
        {fcitx::Key("Control+comma"), fcitx::Key("Control+less")},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CIRCLE_KANA_MODE{
        this,
        "CircleKanaModeKey",
        _("Circle kana mode"),
        {fcitx::Key("Control+period"), fcitx::Key(FcitxKey_Hiragana_Katakana)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CIRCLE_LATIN_HIRAGANA_MODE{
        this,
        "CircleLatinHiraganaModeKey",
        _("Circle latin and hiragana mode"),
        {},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CIRCLE_TYPING_METHOD{
        this,
        "CircleTypingMethodKey",
        _("Circle typing method"),
        {fcitx::Key("Alt+Romaji"), fcitx::Key("Control+slash")},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_LATIN_MODE{
        this,
        "LatinModeKey",
        _("Latin mode"),
        {},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_WIDE_LATIN_MODE{
        this,
        "WideLatinModeKey",
        _("Wide latin mode"),
        {},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_HIRAGANA_MODE{
        this,
        "HiraganaModeKey",
        _("Hiragana mode"),
        {},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_KATAKANA_MODE{
        this,
        "KatakanaModeKey",
        _("Katakana mode"),
        {},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_HALF_KATAKANA_MODE{
        this,
        "HalfKatakanaModeKey",
        _("Half Katakana mode"),
        {},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CANCEL_PSEUDO_ASCII_MODE{
        this,
        "CancelPseudoAsciiModeKey",
        _("Cancel pseudo ascii mode"),
        {fcitx::Key(FcitxKey_Escape)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_INSERT_SPACE{
        this,
        "InsertSpaceKey",
        _("Insert space"),
        {fcitx::Key(FcitxKey_space)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_INSERT_ALT_SPACE{
        this,
        "InsertAltSpaceKey",
        _("Insert Alternative space"),
        {fcitx::Key("Shift+space")},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_INSERT_HALF_SPACE{
        this,
        "InsertHalfSpaceKey",
        _("Insert Half Space"),
        {},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_INSERT_WIDE_SPACE{
        this,
        "InsertWideSpaceKey",
        _("Insert wide space"),
        {},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_BACKSPACE{
        this,
        "BackSpaceKey",
        _("BackSpace"),
        {fcitx::Key(FcitxKey_BackSpace), fcitx::Key("Control+H")},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_DELETE{
        this,
        "DeleteKey",
        _("Delete"),
        {fcitx::Key(FcitxKey_Delete), fcitx::Key("Control+D")},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_COMMIT{
        this,
        "CommitKey",
        _("Commit"),
        {fcitx::Key(FcitxKey_Return), fcitx::Key("Control+J")},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_COMMIT_REVERSE_LEARN{
        this,
        "CommitReverseLearnKey",
        _("Commit Reverse Learn"),
        {fcitx::Key("Shift+Return")},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CONVERT{
        this,
        "ConvertKey",
        _("Convert"),
        {fcitx::Key(FcitxKey_space), fcitx::Key(FcitxKey_Henkan)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_PREDICT{
        this,
        "PredictKey",
        _("Predict"),
        {fcitx::Key(FcitxKey_Tab)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CANCEL{
        this,
        "CancelKey",
        _("Cancel"),
        {fcitx::Key(FcitxKey_Escape), fcitx::Key("Control+G")},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CANCEL_ALL{
        this,
        "CancelAllKey",
        _("Cancel all"),
        {},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_MOVE_CARET_FIRST{
        this,
        "MoveCaretFirstKey",
        _("Move caret first"),
        {fcitx::Key("Control+A"), fcitx::Key(FcitxKey_Home)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_MOVE_CARET_LAST{
        this,
        "MoveCaretLastKey",
        _("Move caret last"),
        {fcitx::Key("Control+E"), fcitx::Key(FcitxKey_End)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_MOVE_CARET_FORWARD{
        this,
        "MoveCaretForwardKey",
        _("Move caret forward"),
        {fcitx::Key("Control+F"), fcitx::Key(FcitxKey_Left)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_MOVE_CARET_BACKWARD{
        this,
        "MoveCaretBackwardKey",
        _("Move caret backward"),
        {fcitx::Key("Control+B"), fcitx::Key(FcitxKey_Right)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_SELECT_FIRST_SEGMENT{
        this,
        "SelectFirstSegmentKey",
        _("Select first segment"),
        {fcitx::Key("Control+A"), fcitx::Key(FcitxKey_Home)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_SELECT_LAST_SEGMENT{
        this,
        "SelectLastSegmentKey",
        _("Select last segment"),
        {fcitx::Key("Control+E"), fcitx::Key(FcitxKey_End)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_SELECT_NEXT_SEGMENT{
        this,
        "SelectNextSegmentKey",
        _("Select next segment"),
        {fcitx::Key("Control+F"), fcitx::Key(FcitxKey_Right)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_SELECT_PREV_SEGMENT{
        this,
        "SelectPrevSegmentKey",
        _("Select prev segment"),
        {fcitx::Key("Control+B"), fcitx::Key(FcitxKey_Left)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_SHRINK_SEGMENT{
        this,
        "ShrinkSegmentKey",
        _("Shrink segment"),
        {fcitx::Key("Shift+Left"), fcitx::Key("Control+I")},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_EXPAND_SEGMENT{
        this,
        "ExpandSegmentKey",
        _("Expand segment"),
        {fcitx::Key("Shift+Right"), fcitx::Key("Control+O")},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_COMMIT_FIRST_SEGMENT{
        this,
        "CommitFirstSegmentKey",
        _("Commit first segment"),
        {fcitx::Key("Shift+Down")},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_COMMIT_SELECTED_SEGMENT{
        this,
        "CommitSelectedSegmentKey",
        _("Commit selected segment"),
        {fcitx::Key("Control+Down")},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_COMMIT_FIRST_SEGMENT_REVERSE_LEARN{
        this,
        "CommitFirstSegmentReverseLearnKey",
        _("Commit first segement reverse learn"),
        {},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_COMMIT_SELECTED_SEGMENT_REVERSE_LEARN{
        this,
        "CommitSelectedSegmentReverseLearnKey",
        _("Commit selected segement reverse learn"),
        {},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_SELECT_FIRST_CANDIDATE{
        this,
        "SelectFirstCandidateKey",
        _("Select first candidate"),
        {fcitx::Key(FcitxKey_Home)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_SELECT_LAST_CANDIDATE{
        this,
        "SelectLastCandidateKey",
        _("Select last candidate"),
        {fcitx::Key(FcitxKey_End)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_SELECT_NEXT_CANDIDATE{
        this,
        "SelectNextCandidateKey",
        _("Select next candidate"),
        {fcitx::Key(FcitxKey_space), fcitx::Key(FcitxKey_Tab),
         fcitx::Key(FcitxKey_Henkan), fcitx::Key(FcitxKey_Down)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_SELECT_PREV_CANDIDATE{
        this,
        "SelectPrevCandidateKey",
        _("Select prev candidate"),
        {fcitx::Key("Shift+Tab"), fcitx::Key(FcitxKey_Up)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CANDIDATES_PAGE_UP{
        this,
        "CandidatesPageUpKey",
        _("Candidates Page Up"),
        {fcitx::Key(FcitxKey_Page_Up)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CANDIDATES_PAGE_DOWN{
        this,
        "CandidatesPageDownKey",
        _("Candidates Page Down"),
        {fcitx::Key(FcitxKey_Page_Down)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CONV_CHAR_TYPE_FORWARD{
        this,
        "ConvertCharTypeForwardKey",
        _("Convert char type forward"),
        {fcitx::Key(FcitxKey_Muhenkan)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CONV_CHAR_TYPE_BACKWARD{
        this,
        "ConvertCharTypeBackwardKey",
        _("Convert char type backward"),
        {},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CONV_TO_HIRAGANA{
        this,
        "ConvertToHiraganaKey",
        _("Convert To Hiragana"),
        {fcitx::Key(FcitxKey_F6)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CONV_TO_KATAKANA{
        this,
        "ConvertToKatakanaKey",
        _("Convert To Katakana"),
        {fcitx::Key(FcitxKey_F7)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CONV_TO_HALF{
        this,
        "ConvertToHalfKey",
        _("Convert To Half"),
        {fcitx::Key(FcitxKey_F8)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CONV_TO_HALF_KATAKANA{
        this,
        "ConvertToHalfKatakanaKey",
        _("Convert To Half Katakana"),
        {fcitx::Key("Shift+F8")},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CONV_TO_WIDE_LATIN{
        this,
        "ConvertToWideLatinKey",
        _("Convert To Wide Latin"),
        {fcitx::Key(FcitxKey_F9)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_CONV_TO_LATIN{
        this,
        "ConvertToLatinKey",
        _("Convert To Latin"),
        {fcitx::Key(FcitxKey_F10)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_RECONVERT{
        this,
        "ReconvertKey",
        _("Reconvert"),
        {fcitx::Key("Shift+Henkan")},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_DICT_ADMIN{
        this,
        "DictAdminKey",
        _("Dict admin"),
        {fcitx::Key(FcitxKey_F11)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption hk_ADD_WORD{
        this,
        "AddWordKey",
        _("Add Word"),
        {fcitx::Key(FcitxKey_F12)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption leftThumbKeys{
        this,
        "LeftThumbKey",
        _("Left thumb key"),
        {fcitx::Key(FcitxKey_Muhenkan)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::KeyListOption rightThumbKeys{
        this,
        "RightThumbKey",
        _("Right thumb key"),
        {fcitx::Key(FcitxKey_Henkan), fcitx::Key(FcitxKey_space)},
        fcitx::KeyListConstrain(fcitx::KeyConstrainFlag::AllowModifierLess)};
    fcitx::Option<std::string> kanaLayoutRoKey{
        this, "KanaLayoutRoKey", _("Ro key for kana layout"), "ろ"};
    fcitx::Option<int> nicolaTime{this, "NicolaTime", _("Nicola time"), 200};);

FCITX_CONFIGURATION(AnthyCommnadConfig,
                    fcitx::Option<std::string> addWordCommand{
                        this, "AddWord", _("Add word"), "kasumi -a"};
                    fcitx::Option<std::string> dictAdminCommand{
                        this, "DictAdmin", _("Dict admin"), "kasumi"};);

FCITX_CONFIGURATION(
    AnthyConfig,
    fcitx::Option<AnthyGeneralConfig> general{this, "General", _("General")};
    fcitx::Option<AnthyInterfaceConfig> interface{this, "Interface",
                                                  _("Interface")};
    fcitx::Option<AnthyKeyProfileConfig> keyProfile{this, "KeyProfile",
                                                    _("Key Profile")};
    fcitx::Option<AnthyKeyConfig> key{this, "Key", _("Key")};
    fcitx::Option<AnthyCommnadConfig> command{this, "Command", _("Command")};);

#endif // _FCITX5_ANTHY_CONFIG_H_
