/*
 * SPDX-FileCopyrightText: 2017-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "engine.h"
#include "action.h"
#include "config.h"
#include "key2kana_table.h"
#include "state.h"
#include "style_file.h"
#include <anthy/anthy.h>
#include <array>
#include <cstdio>
#include <fcitx-config/iniparser.h>
#include <fcitx-utils/fs.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/macros.h>
#include <fcitx-utils/misc.h>
#include <fcitx-utils/standardpath.h>
#include <fcitx-utils/standardpaths.h>
#include <fcitx-utils/stringutils.h>
#include <fcitx/action.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/event.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextmanager.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/inputmethodentry.h>
#include <fcitx/statusarea.h>
#include <fcitx/userinterfacemanager.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>

FCITX_DEFINE_LOG_CATEGORY(anthy_logcategory, "anthy")

class AnthyFactory : public fcitx::AddonFactory {
    fcitx::AddonInstance *create(fcitx::AddonManager *manager) override {
        fcitx::registerDomain("fcitx5-anthy", FCITX_INSTALL_LOCALEDIR);
        return new AnthyEngine(manager->instance());
    }
};

struct AnthyStatus {
    const char *icon;
    const char *label;
    const char *description;
};

#define UTF8_BRACKET_CORNER_BEGIN "\xE3\x80\x8C"
#define UTF8_BRACKET_CORNER_END "\xE3\x80\x8D"
#define UTF8_BRACKET_WIDE_BEGIN "\xEF\xBC\xBB"
#define UTF8_BRACKET_WIDE_END "\xEF\xBC\xBD"
#define UTF8_MIDDLE_DOT "\xE3\x83\xBB"
#define UTF8_SLASH_WIDE "\xEF\xBC\x8F"

constexpr std::array input_mode_status = {
    AnthyStatus{"", "\xe3\x81\x82", N_("Hiragana")},
    AnthyStatus{"", "\xe3\x82\xa2", N_("Katakana")},
    AnthyStatus{"", "\xef\xbd\xb1", N_("Half width katakana")},
    AnthyStatus{"", "A", N_("Direct input")},
    AnthyStatus{"", "\xef\xbc\xa1", N_("Wide latin")},
};

constexpr std::array typing_method_status = {
    AnthyStatus{"", N_("Romaji"), N_("Romaji")},
    AnthyStatus{"", N_("Kana"), N_("Kana")},
    AnthyStatus{"", N_("Nicola"), N_("Thumb shift")},
};

constexpr std::array conversion_mode_status = {
    AnthyStatus{"", "\xE9\x80\xA3", N_("Multi segment")},
    AnthyStatus{"", "\xE5\x8D\x98", N_("Single segment")},
    AnthyStatus{"", "\xE9\x80\x90", N_("Convert as you type (Multi segment)")},
    AnthyStatus{"", "\xE9\x80\x90", N_("Convert as you type (Single segment)")},
};

constexpr std::array period_style_status = {
    AnthyStatus{"anthy-period-wide-latin", "\xEF\xBC\x8C\xEF\xBC\x8E",
                "\xEF\xBC\x8C\xEF\xBC\x8E"},
    AnthyStatus{"anthy-period-latin", ",.", ",."},
    AnthyStatus{"anthy-period-japanese", "\xE3\x80\x81\xE3\x80\x82",
                "\xE3\x80\x81\xE3\x80\x82"},
    AnthyStatus{"anthy-period-wide-japanese", "\xEF\xBC\x8C\xE3\x80\x82",
                "\xEF\xBC\x8C\xE3\x80\x82"},
};

constexpr std::array symbol_style_status = {
    AnthyStatus{
        "anthy-symbol",
        UTF8_BRACKET_CORNER_BEGIN UTF8_BRACKET_CORNER_END UTF8_MIDDLE_DOT,
        UTF8_BRACKET_CORNER_BEGIN UTF8_BRACKET_CORNER_END UTF8_MIDDLE_DOT},
    AnthyStatus{
        "anthy-symbol",
        UTF8_BRACKET_CORNER_BEGIN UTF8_BRACKET_CORNER_END UTF8_SLASH_WIDE,
        UTF8_BRACKET_CORNER_BEGIN UTF8_BRACKET_CORNER_END UTF8_SLASH_WIDE},
    AnthyStatus{"anthy-symbol",
                UTF8_BRACKET_WIDE_BEGIN UTF8_BRACKET_WIDE_END UTF8_MIDDLE_DOT,
                UTF8_BRACKET_WIDE_BEGIN UTF8_BRACKET_WIDE_END UTF8_MIDDLE_DOT},
    AnthyStatus{"anthy-symbol",
                UTF8_BRACKET_WIDE_BEGIN UTF8_BRACKET_WIDE_END UTF8_SLASH_WIDE,
                UTF8_BRACKET_WIDE_BEGIN UTF8_BRACKET_WIDE_END UTF8_SLASH_WIDE},
};

template <typename ModeType>
struct AnthyModeTraits;

template <typename ModeType, auto value>
struct AnthyStatusHelper {
    static const AnthyStatus *status(ModeType mode) {
        auto index = static_cast<int>(mode);
        if (index < 0 || static_cast<size_t>(index) >= std::size(*value)) {
            return nullptr;
        }
        return &(*value)[index];
    }
    static const char *icon(ModeType mode) {
        if (auto *s = status(mode)) {
            return s->icon;
        }
        return "";
    }
    static auto label(ModeType mode) {
        if (auto *s = status(mode)) {
            return s->label;
        }
        return "";
    }
    static auto description(ModeType mode) {
        if (auto *s = status(mode)) {
            return _(s->description);
        }
        return "";
    }
};

template <>
struct AnthyModeTraits<InputMode>
    : AnthyStatusHelper<InputMode, &input_mode_status> {
    static auto mode(AnthyState *state) { return state->inputMode(); }
    static void setMode(AnthyState *state, InputMode mode) {
        state->setInputMode(mode, /*propagate=*/true);
    }
    static const char *icon(InputMode mode) {
        if (const auto *s = status(mode)) {
            return s->icon;
        }
        return "";
    }
    static std::string label(InputMode mode) {
        if (const auto *s = status(mode)) {
            return fcitx::stringutils::concat(s->label, " - ",
                                              _(s->description));
        }
        return "";
    }
};

template <>
struct AnthyModeTraits<TypingMethod>
    : AnthyStatusHelper<TypingMethod, &typing_method_status> {
    static auto mode(AnthyState *state) { return state->typingMethod(); }
    static void setMode(AnthyState *state, TypingMethod mode) {
        state->engine()->setTypingMethod(mode);
    }
    static auto label(TypingMethod mode) {
        if (const auto *s = status(mode)) {
            return _(s->label);
        }
        return "";
    }
};

template <>
struct AnthyModeTraits<ConversionMode>
    : AnthyStatusHelper<ConversionMode, &conversion_mode_status> {
    static auto mode(AnthyState *state) {
        return state->engine()->conversionMode();
    }
    static void setMode(AnthyState *state, ConversionMode mode) {
        state->engine()->setConversionMode(mode);
    }
    static std::string label(ConversionMode mode) {
        if (const auto *s = status(mode)) {
            return fcitx::stringutils::concat(s->label, " - ",
                                              _(s->description));
        }
        return "";
    }
};

template <>
struct AnthyModeTraits<PeriodCommaStyle>
    : AnthyStatusHelper<PeriodCommaStyle, &period_style_status> {
    static auto mode(AnthyState *state) {
        return state->engine()->periodCommaStyle();
    }
    static void setMode(AnthyState *state, PeriodCommaStyle mode) {
        state->engine()->setPeriodCommaStyle(mode);
    }
};

template <>
struct AnthyModeTraits<SymbolStyle>
    : AnthyStatusHelper<SymbolStyle, &symbol_style_status> {
    static auto mode(AnthyState *state) {
        return state->engine()->symbolStyle();
    }
    static void setMode(AnthyState *state, SymbolStyle mode) {
        state->engine()->setSymbolStyle(mode);
    }
};

template <typename ModeType>
class AnthyAction : public fcitx::Action {
public:
    explicit AnthyAction(AnthyEngine *engine) : engine_(engine) {}
    std::string shortText(fcitx::InputContext *ic) const override {
        auto *state = engine_->state(ic);
        auto mode = AnthyModeTraits<ModeType>::mode(state);
        return AnthyModeTraits<ModeType>::label(mode);
    }
    std::string longText(fcitx::InputContext *ic) const override {
        auto *state = engine_->state(ic);
        auto mode = AnthyModeTraits<ModeType>::mode(state);
        return AnthyModeTraits<ModeType>::description(mode);
    }
    std::string icon(fcitx::InputContext *ic) const override {
        auto *state = engine_->state(ic);
        auto mode = AnthyModeTraits<ModeType>::mode(state);
        return AnthyModeTraits<ModeType>::icon(mode);
    }

private:
    AnthyEngine *engine_;
};

template <typename ModeType>
class AnthySubAction : public fcitx::SimpleAction {
public:
    AnthySubAction(AnthyEngine *engine, ModeType mode)
        : engine_(engine), mode_(mode) {
        setShortText(AnthyModeTraits<ModeType>::label(mode));
        setLongText(AnthyModeTraits<ModeType>::description(mode));
        setIcon(AnthyModeTraits<ModeType>::icon(mode));
        setCheckable(true);
    }
    bool isChecked(fcitx::InputContext *ic) const override {
        auto *state = engine_->state(ic);
        return mode_ == AnthyModeTraits<ModeType>::mode(state);
    }
    void activate(fcitx::InputContext *ic) override {
        auto *state = engine_->state(ic);
        AnthyModeTraits<ModeType>::setMode(state, mode_);
    }

private:
    AnthyEngine *engine_;
    const ModeType mode_;
};

using InputModeAction = AnthyAction<InputMode>;
using InputModeSubAction = AnthySubAction<InputMode>;

using TypingMethodAction = AnthyAction<TypingMethod>;
using TypingMethodSubAction = AnthySubAction<TypingMethod>;

using ConversionModeAction = AnthyAction<ConversionMode>;
using ConversionModeSubAction = AnthySubAction<ConversionMode>;

using PeriodCommaStyleAction = AnthyAction<PeriodCommaStyle>;
using PeriodCommaStyleSubAction = AnthySubAction<PeriodCommaStyle>;

using SymbolStyleAction = AnthyAction<SymbolStyle>;
using SymbolStyleSubAction = AnthySubAction<SymbolStyle>;

AnthyEngine::AnthyEngine(fcitx::Instance *instance)
    : instance_(instance), factory_([this](fcitx::InputContext &ic) {
          return new AnthyState(&ic, this, instance_);
      }) {
    anthy_set_logger(
        [](int level, const char *msg) {
            FCITX_UNUSED(level);
            FCITX_ANTHY_INFO() << "Anthy: " << msg;
        },
        0);
    if constexpr (fcitx::isAndroid() || fcitx::isApple() ||
                  fcitx::isEmscripten()) {
        const auto &sp = fcitx::StandardPath::global();
        std::string anthy_conf = sp.locate(fcitx::StandardPath::Type::Data,
                                           "anthy/anthy-unicode.conf");
        std::string anthy_prefix = fcitx::fs::dirName(anthy_conf);
        // "CONFFILE" must be overridden first to change main config file path
        anthy_conf_override("CONFFILE", anthy_conf.c_str());
        // "prefix" must be set before using "${prefix}" in other values
        anthy_conf_override("prefix", anthy_prefix.c_str());
        anthy_conf_override("ANTHYDIR", "${prefix}/");
        anthy_conf_override("DIC_FILE", "${prefix}/anthy.dic");
        // save anthy data (primarily input history) to
        // /sdcard/Android/data/<pkg>/files/data/anthy
        anthy_conf_override(
            "XDG_CONFIG_HOME",
            sp.userDirectory(fcitx::StandardPath::Type::Data).c_str());
    }
    if (anthy_init()) {
        throw std::runtime_error("Failed to init anthy library.");
    }

    reloadConfig();
    inputModeAction_ = std::make_unique<InputModeAction>(this);
    inputModeAction_->setMenu(&inputModeMenu_);
    instance_->userInterfaceManager().registerAction("anthy-input-mode",
                                                     inputModeAction_.get());

    typingMethodAction_ = std::make_unique<TypingMethodAction>(this);
    typingMethodAction_->setMenu(&typingMethodMenu_);
    instance_->userInterfaceManager().registerAction("anthy-typing-method",
                                                     typingMethodAction_.get());

    conversionModeAction_ = std::make_unique<ConversionModeAction>(this);
    conversionModeAction_->setMenu(&conversionModeMenu_);
    instance_->userInterfaceManager().registerAction(
        "anthy-conversion-mode", conversionModeAction_.get());

    periodStyleAction_ = std::make_unique<PeriodCommaStyleAction>(this);
    periodStyleAction_->setMenu(&periodStyleMenu_);
    instance_->userInterfaceManager().registerAction("anthy-period-style",
                                                     periodStyleAction_.get());

    symbolStyleAction_ = std::make_unique<SymbolStyleAction>(this);
    symbolStyleAction_->setMenu(&symbolStyleMenu_);
    instance_->userInterfaceManager().registerAction("anthy-symbol-style",
                                                     symbolStyleAction_.get());

#define _ADD_ACTION(MODE, TYPE, type, NAME)                                    \
    subModeActions_.emplace_back(                                              \
        std::make_unique<TYPE##SubAction>(this, MODE));                        \
    instance_->userInterfaceManager().registerAction(                          \
        NAME, subModeActions_.back().get());                                   \
    type##Menu_.addAction(subModeActions_.back().get());

    _ADD_ACTION(InputMode::HIRAGANA, InputMode, inputMode,
                "anthy-input-mode-hiragana")
    _ADD_ACTION(InputMode::KATAKANA, InputMode, inputMode,
                "anthy-input-mode-katakana")
    _ADD_ACTION(InputMode::HALF_KATAKANA, InputMode, inputMode,
                "anthy-input-mode-half-katakana")
    _ADD_ACTION(InputMode::LATIN, InputMode, inputMode,
                "anthy-input-mode-latin")
    _ADD_ACTION(InputMode::WIDE_LATIN, InputMode, inputMode,
                "anthy-input-mode-wide-latin")

    _ADD_ACTION(TypingMethod::ROMAJI, TypingMethod, typingMethod,
                "anthy-typing-method-romaji")
    _ADD_ACTION(TypingMethod::KANA, TypingMethod, typingMethod,
                "anthy-typing-method-kana")
    _ADD_ACTION(TypingMethod::NICOLA, TypingMethod, typingMethod,
                "anthy-typing-method-nicola")

    _ADD_ACTION(ConversionMode::MULTI_SEGMENT, ConversionMode, conversionMode,
                "anthy-conversion-mode-multi")
    _ADD_ACTION(ConversionMode::SINGLE_SEGMENT, ConversionMode, conversionMode,
                "anthy-conversion-mode-single")
    _ADD_ACTION(ConversionMode::MULTI_SEGMENT_IMMEDIATE, ConversionMode,
                conversionMode, "anthy-conversion-mode-multi-imm")
    _ADD_ACTION(ConversionMode::SINGLE_SEGMENT_IMMEDIATE, ConversionMode,
                conversionMode, "anthy-conversion-mode-single-imm")

    _ADD_ACTION(PeriodCommaStyle::WIDELATIN, PeriodCommaStyle, periodStyle,
                "anthy-period-widelatin")
    _ADD_ACTION(PeriodCommaStyle::LATIN, PeriodCommaStyle, periodStyle,
                "anthy-period-latin")
    _ADD_ACTION(PeriodCommaStyle::JAPANESE, PeriodCommaStyle, periodStyle,
                "anthy-period-japanese")
    _ADD_ACTION(PeriodCommaStyle::WIDELATIN_JAPANESE, PeriodCommaStyle,
                periodStyle, "anthy-period-widelatin-japanese")

    _ADD_ACTION(SymbolStyle::JAPANESE, SymbolStyle, symbolStyle,
                "anthy-symbol-japanese")
    _ADD_ACTION(SymbolStyle::WIDEBRACKET_WIDESLASH, SymbolStyle, symbolStyle,
                "anthy-symbol-widebracket-wideslash")
    _ADD_ACTION(SymbolStyle::CORNERBRACKET_MIDDLEDOT, SymbolStyle, symbolStyle,
                "anthy-symbol-cornerbracket-middledot")
    _ADD_ACTION(SymbolStyle::CORNERBRACKET_WIDESLASH, SymbolStyle, symbolStyle,
                "anthy-symbol-cornerbracket-wideslash")

    instance_->inputContextManager().registerProperty("anthyState", &factory_);

    constructed_ = true;
}

AnthyEngine::~AnthyEngine() { anthy_quit(); }

void AnthyEngine::keyEvent(const fcitx::InputMethodEntry & /*entry*/,
                           fcitx::KeyEvent &keyEvent) {
    auto *anthy = keyEvent.inputContext()->propertyFor(&factory_);
    auto result = anthy->processKeyEvent(keyEvent);
    anthy->updateUI();
    if (result) {
        keyEvent.filterAndAccept();
    }
}

std::string AnthyEngine::keyProfileName() const {
    const std::string key_profile[] = {"",
                                       "atok.sty",
                                       "canna.sty",
                                       "msime.sty",
                                       "vje-delta.sty",
                                       "wnn.sty",
                                       *config().keyProfile->keyThemeFile};

    auto profile = static_cast<int>(*config().keyProfile->keyProfileEnum);
    return key_profile[profile];
}

std::string AnthyEngine::romajiTableName() const {
    const std::string key_profile[] = {
        "",          "atok.sty",
        "azik.sty",  "canna.sty",
        "msime.sty", "vje-delta.sty",
        "wnn.sty",   *config().keyProfile->romajiFundamentalTable};

    static_assert(FCITX_ARRAY_SIZE(key_profile) ==
                  static_cast<int>(RomajiTable::Custom) + 1);
    auto profile = static_cast<int>(*config().keyProfile->romajiTableEnum);

    return key_profile[profile];
}

std::string AnthyEngine::kanaTableName() const {
    const std::string key_profile[] = {
        "",
        "101kana.sty",
        "tsuki-2-203-101.sty",
        "tsuki-2-203-106.sty",
        "qkana.sty",
        *config().keyProfile->kanaFundamentalTable};

    static_assert(FCITX_ARRAY_SIZE(key_profile) ==
                  static_cast<int>(KanaTable::Custom) + 1);
    auto profile = static_cast<int>(*config().keyProfile->kanaTableEnum);

    return key_profile[profile];
}

std::string AnthyEngine::nicolaTableName() const {
    const std::string key_profile[] = {
        "",
        "nicola-a.sty",
        "nicola-f.sty",
        "nicola-j.sty",
        "oasys100j.sty",
        "tron-dvorak.sty",
        "tron-qwerty-jp.sty",
        *config().keyProfile->nicolaFundamentalTable};

    static_assert(FCITX_ARRAY_SIZE(key_profile) ==
                  static_cast<int>(NicolaTable::Custom) + 1);
    auto profile = static_cast<int>(*config().keyProfile->nicolaTableEnum);

    return key_profile[profile];
}

std::string AnthyEngine::fullFileName(const std::string &name) {
    if (name.empty()) {
        return {};
    }
    return fcitx::StandardPath::global().locate(
        fcitx::StandardPath::Type::PkgData,
        fcitx::stringutils::joinPath("anthy", name));
}

void AnthyEngine::reloadConfig() {
    fcitx::readAsIni(config_, "conf/anthy.conf");
    populateConfig();
}

void AnthyEngine::populateConfig() {
    StyleFile style;
    // load key bindings
    keyProfileFileLoaded_ = false;
    auto filename = fullFileName(keyProfileName());
    if (!filename.empty()) {
        keyProfileFileLoaded_ = style.load(filename);
#define FOREACH_ACTION(key, _)                                                 \
    do {                                                                       \
        std::string str = (ACTION_CONFIG_##key##_KEY);                         \
        std::string keystr;                                                    \
        if (auto value = style.getString("KeyBindings", str)) {                \
            keystr = std::move(*value);                                        \
        }                                                                      \
        keyProfile_.hk_##key = fcitx::Key::keyListFromString(keystr);          \
    } while (0)
#include "action_defs.h"
#undef FOREACH_ACTION
    }

    customRomajiTableLoaded_ = false;
    customRomajiTable_.clear();
    const char *section_romaji = "RomajiTable/FundamentalTable";
    filename = fullFileName(romajiTableName());
    if (!filename.empty()) {
        FCITX_ANTHY_DEBUG() << "Try loading romaji table: " << filename;
        if (style.load(filename)) {
            customRomajiTableLoaded_ = true;
            customRomajiTable_ = Key2KanaTable(section_romaji, style);
        }
    }

    // load custom kana table
    customKanaTableLoaded_ = false;
    customKanaTable_.clear();
    const char *section_kana = "KanaTable/FundamentalTable";
    filename = fullFileName(kanaTableName());
    if (!filename.empty() && style.load(filename)) {
        FCITX_ANTHY_DEBUG() << "Try loading kana table: " << filename;
        if (style.load(filename)) {
            customKanaTableLoaded_ = true;
            customKanaTable_ = Key2KanaTable(section_kana, style);
        }
    }

    customNicolaTableLoaded_ = false;
    customNicolaTable_.clear();
    const char *section_nicola = "NICOLATable/FundamentalTable";
    filename = fullFileName(nicolaTableName());
    if (!filename.empty()) {
        FCITX_ANTHY_DEBUG() << "Try loading nicola table: " << filename;
        if (style.load(filename)) {
            customNicolaTableLoaded_ = true;
            customNicolaTable_ = Key2KanaTable(section_nicola, style);
        }
    }

    populateOptionToState();
}

void AnthyEngine::populateOptionToState() {
    if (factory_.registered()) {
        instance_->inputContextManager().foreach(
            [this](fcitx::InputContext *ic) {
                auto *state = this->state(ic);
                state->configure();
                return true;
            });
    }
}

std::string AnthyEngine::subMode(const fcitx::InputMethodEntry & /*entry*/,
                                 fcitx::InputContext &ic) {
    auto *state = this->state(&ic);
    return AnthyModeTraits<InputMode>::description(state->inputMode());
}

std::string
AnthyEngine::subModeLabelImpl(const fcitx::InputMethodEntry & /*unused*/,
                              fcitx::InputContext &ic) {
    auto *state = this->state(&ic);

    if (const auto *s =
            AnthyStatusHelper<InputMode, &input_mode_status>::status(
                state->inputMode())) {
        return s->label;
    }
    return "";
}

void AnthyEngine::activate(const fcitx::InputMethodEntry & /*entry*/,
                           fcitx::InputContextEvent &event) {
    // Setup action.
    if (*config_.interface->showInputModeLabel) {
        event.inputContext()->statusArea().addAction(
            fcitx::StatusGroup::InputMethod, inputModeAction_.get());
    }
    if (*config_.interface->showTypingMethodLabel) {
        event.inputContext()->statusArea().addAction(
            fcitx::StatusGroup::InputMethod, typingMethodAction_.get());
    }
    if (*config_.interface->showConvModeLabel) {
        event.inputContext()->statusArea().addAction(
            fcitx::StatusGroup::InputMethod, conversionModeAction_.get());
    }
    if (*config_.interface->showPeriodStyleLabel) {
        event.inputContext()->statusArea().addAction(
            fcitx::StatusGroup::InputMethod, periodStyleAction_.get());
    }
    if (*config_.interface->showSymbolStyleLabel) {
        event.inputContext()->statusArea().addAction(
            fcitx::StatusGroup::InputMethod, symbolStyleAction_.get());
    }
}

void AnthyEngine::deactivate(const fcitx::InputMethodEntry & /*entry*/,
                             fcitx::InputContextEvent &event) {
    auto *anthy = event.inputContext()->propertyFor(&factory_);
    anthy->autoCommit(event);
    anthy->updateUI();
}

void AnthyEngine::reset(const fcitx::InputMethodEntry & /*entry*/,
                        fcitx::InputContextEvent &event) {
    auto *anthy = event.inputContext()->propertyFor(&factory_);
    anthy->reset();
    anthy->updateUI();
}

void AnthyEngine::invokeActionImpl(const fcitx::InputMethodEntry &entry,
                                   fcitx::InvokeActionEvent &event) {
    const int cursor = event.cursor();
    if (event.action() != fcitx::InvokeActionEvent::Action::LeftClick ||
        cursor < 0) {
        InputMethodEngineV3::invokeActionImpl(entry, event);
        return;
    }
    event.filter();
    auto *anthy = event.inputContext()->propertyFor(&factory_);
    anthy->movePreeditCaret(event.cursor());
    anthy->updateUI();
}

FCITX_ADDON_FACTORY_V2(anthy, AnthyFactory)
