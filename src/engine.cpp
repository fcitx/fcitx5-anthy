//
// Copyright (C) 2017~2017 by CSSlayer
// wengxt@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "engine.h"
#include "action.h"
#include "state.h"
#include <errno.h>
#include <fcitx-config/iniparser.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/standardpath.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextmanager.h>
#include <fcitx/userinterfacemanager.h>
#include <libintl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

FCITX_DEFINE_LOG_CATEGORY(anthy_logcategory, "anthy");

class AnthyFactory : public fcitx::AddonFactory {
    fcitx::AddonInstance *create(fcitx::AddonManager *manager) override {
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

const static AnthyStatus input_mode_status[] = {
    {"", "\xe3\x81\x82", N_("Hiragana")},
    {"", "\xe3\x82\xa2", N_("Katakana")},
    {"", "\xef\xbd\xb1", N_("Half width katakana")},
    {"", "A", N_("Direct input")},
    {"", "\xef\xbc\xa1", N_("Wide latin")},
};

AnthyStatus typing_method_status[] = {
    {"", N_("Romaji"), N_("Romaji")},
    {"", N_("Kana"), N_("Kana")},
    {"", N_("Nicola"), N_("Thumb shift")},
};

AnthyStatus conversion_mode_status[] = {
    {"", "\xE9\x80\xA3", N_("Multi segment")},
    {"", "\xE5\x8D\x98", N_("Single segment")},
    {"", "\xE9\x80\x90", N_("Convert as you type (Multi segment)")},
    {"", "\xE9\x80\x90", N_("Convert as you type (Single segment)")},
};

AnthyStatus period_style_status[] = {
    {"anthy-period-wide-latin", "\xEF\xBC\x8C\xEF\xBC\x8E",
     "\xEF\xBC\x8C\xEF\xBC\x8E"},
    {"anthy-period-latin", ",.", ",."},
    {"anthy-period-japanese", "\xE3\x80\x81\xE3\x80\x82",
     "\xE3\x80\x81\xE3\x80\x82"},
    {"anthy-period-wide-japanese", "\xEF\xBC\x8C\xE3\x80\x82",
     "\xEF\xBC\x8C\xE3\x80\x82"},
};

AnthyStatus symbol_style_status[] = {
    {"anthy-symbol",
     UTF8_BRACKET_CORNER_BEGIN UTF8_BRACKET_CORNER_END UTF8_MIDDLE_DOT,
     UTF8_BRACKET_CORNER_BEGIN UTF8_BRACKET_CORNER_END UTF8_MIDDLE_DOT},
    {"anthy-symbol",
     UTF8_BRACKET_CORNER_BEGIN UTF8_BRACKET_CORNER_END UTF8_SLASH_WIDE,
     UTF8_BRACKET_CORNER_BEGIN UTF8_BRACKET_CORNER_END UTF8_SLASH_WIDE},
    {"anthy-symbol",
     UTF8_BRACKET_WIDE_BEGIN UTF8_BRACKET_WIDE_END UTF8_MIDDLE_DOT,
     UTF8_BRACKET_WIDE_BEGIN UTF8_BRACKET_WIDE_END UTF8_MIDDLE_DOT},
    {"anthy-symbol",
     UTF8_BRACKET_WIDE_BEGIN UTF8_BRACKET_WIDE_END UTF8_SLASH_WIDE,
     UTF8_BRACKET_WIDE_BEGIN UTF8_BRACKET_WIDE_END UTF8_SLASH_WIDE},
};

#define DEFINE_MENU(NAME, name, array)                                         \
    auto name##Status(AnthyEngine *engine, fcitx::InputContext *ic) {          \
        auto state = engine->state(ic);                                        \
        auto mode = static_cast<int>(state->name());                           \
        return (mode >= 0 &&                                                   \
                static_cast<size_t>(mode) < FCITX_ARRAY_SIZE(array))           \
                   ? &array[mode]                                              \
                   : nullptr;                                                  \
    }                                                                          \
    class NAME##Action : public fcitx::Action {                                \
    public:                                                                    \
        NAME##Action(AnthyEngine *engine) : engine_(engine) {}                 \
        std::string shortText(fcitx::InputContext *ic) const override {        \
            if (auto status = name##Status(engine_, ic)) {                     \
                return _(status->label);                                       \
            }                                                                  \
            return "";                                                         \
        }                                                                      \
        std::string longText(fcitx::InputContext *ic) const override {         \
            if (auto status = name##Status(engine_, ic)) {                     \
                return _(status->description);                                 \
            }                                                                  \
            return "";                                                         \
        }                                                                      \
        std::string icon(fcitx::InputContext *ic) const override {             \
            if (auto status = name##Status(engine_, ic)) {                     \
                return status->icon;                                           \
            }                                                                  \
            return "";                                                         \
        }                                                                      \
                                                                               \
    private:                                                                   \
        AnthyEngine *engine_;                                                  \
    };                                                                         \
    class NAME##SubAction : public fcitx::SimpleAction {                       \
    public:                                                                    \
        NAME##SubAction(AnthyEngine *engine, NAME mode)                        \
            : engine_(engine), mode_(mode) {                                   \
            setShortText(array[static_cast<int>(mode)].label);                 \
            setLongText(_(array[static_cast<int>(mode)].description));         \
            setIcon(array[static_cast<int>(mode)].icon);                       \
            setCheckable(true);                                                \
        }                                                                      \
        bool isChecked(fcitx::InputContext *ic) const override {               \
            auto state = engine_->state(ic);                                   \
            return mode_ == state->name();                                     \
        }                                                                      \
        void activate(fcitx::InputContext *ic) override {                      \
            auto state = engine_->state(ic);                                   \
            state->set##NAME(mode_);                                           \
        }                                                                      \
                                                                               \
    private:                                                                   \
        AnthyEngine *engine_;                                                  \
        NAME mode_;                                                            \
    };

DEFINE_MENU(InputMode, inputMode, input_mode_status);
DEFINE_MENU(TypingMethod, typingMethod, typing_method_status);
DEFINE_MENU(ConversionMode, conversionMode, conversion_mode_status);
DEFINE_MENU(PeriodCommaStyle, periodCommaStyle, period_style_status);
DEFINE_MENU(SymbolStyle, symbolStyle, symbol_style_status);

AnthyEngine::AnthyEngine(fcitx::Instance *instance)
    : instance_(instance), factory_([this](fcitx::InputContext &ic) {
          return new AnthyState(&ic, this, instance_);
      }) {
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
                "anthy-input-mode-hiragana");
    _ADD_ACTION(InputMode::KATAKANA, InputMode, inputMode,
                "anthy-input-mode-katakana");
    _ADD_ACTION(InputMode::HALF_KATAKANA, InputMode, inputMode,
                "anthy-input-mode-half-katakana");
    _ADD_ACTION(InputMode::LATIN, InputMode, inputMode,
                "anthy-input-mode-latin");
    _ADD_ACTION(InputMode::WIDE_LATIN, InputMode, inputMode,
                "anthy-input-mode-wide-latin");

    _ADD_ACTION(TypingMethod::ROMAJI, TypingMethod, typingMethod,
                "anthy-typing-method-romaji");
    _ADD_ACTION(TypingMethod::KANA, TypingMethod, typingMethod,
                "anthy-typing-method-kana");
    _ADD_ACTION(TypingMethod::NICOLA, TypingMethod, typingMethod,
                "anthy-typing-method-nicola");

    _ADD_ACTION(ConversionMode::MULTI_SEGMENT, ConversionMode, conversionMode,
                "anthy-conversion-mode-multi");
    _ADD_ACTION(ConversionMode::SINGLE_SEGMENT, ConversionMode, conversionMode,
                "anthy-conversion-mode-single");
    _ADD_ACTION(ConversionMode::MULTI_SEGMENT_IMMEDIATE, ConversionMode,
                conversionMode, "anthy-conversion-mode-multi-imm");
    _ADD_ACTION(ConversionMode::SINGLE_SEGMENT_IMMEDIATE, ConversionMode,
                conversionMode, "anthy-conversion-mode-single-imm");

    _ADD_ACTION(PeriodCommaStyle::WIDELATIN, PeriodCommaStyle, periodStyle,
                "anthy-period-widelatin");
    _ADD_ACTION(PeriodCommaStyle::LATIN, PeriodCommaStyle, periodStyle,
                "anthy-period-latin");
    _ADD_ACTION(PeriodCommaStyle::JAPANESE, PeriodCommaStyle, periodStyle,
                "anthy-period-japanese");
    _ADD_ACTION(PeriodCommaStyle::WIDELATIN_JAPANESE, PeriodCommaStyle,
                periodStyle, "anthy-period-widelatin-japanese");

    _ADD_ACTION(SymbolStyle::JAPANESE, SymbolStyle, symbolStyle,
                "anthy-symbol-japanese");
    _ADD_ACTION(SymbolStyle::WIDEBRACKET_WIDESLASH, SymbolStyle, symbolStyle,
                "anthy-symbol-widebracket-wideslash");
    _ADD_ACTION(SymbolStyle::CORNERBRACKET_MIDDLEDOT, SymbolStyle, symbolStyle,
                "anthy-symbol-cornerbracket-middledot");
    _ADD_ACTION(SymbolStyle::CORNERBRACKET_WIDESLASH, SymbolStyle, symbolStyle,
                "anthy-symbol-cornerbracket-wideslash");

    instance_->inputContextManager().registerProperty("anthyState", &factory_);

    constructed_ = true;
}

AnthyEngine::~AnthyEngine() { anthy_quit(); }

void AnthyEngine::keyEvent(const fcitx::InputMethodEntry &,
                           fcitx::KeyEvent &keyEvent) {
    auto anthy = keyEvent.inputContext()->propertyFor(&factory_);
    auto result = anthy->processKeyEvent(keyEvent);
    anthy->updateUI();
    if (result) {
        keyEvent.filterAndAccept();
    }
}

std::string AnthyEngine::keyProfileName() {
    const std::string key_profile[] = {
        "",
        "atok.sty",
        "canna.sty",
        "msime.sty",
        "vje-delta.sty",
        "wnn.sty",
        *config().m_key_profile->m_key_theme_file};

    auto profile =
        static_cast<int>(*config().m_key_profile->m_key_profile_enum);
    return key_profile[profile];
}

std::string AnthyEngine::romajiTableName() {
    const std::string key_profile[] = {
        "",          "atok.sty",
        "azik.sty",  "canna.sty",
        "msime.sty", "vje-delta.sty",
        "wnn.sty",   *config().m_key_profile->m_romaji_fundamental_table};

    auto profile =
        static_cast<int>(*config().m_key_profile->m_romaji_table_enum);

    return key_profile[profile];
}

std::string AnthyEngine::kanaTableName() {
    const std::string key_profile[] = {
        "",
        "101kana.sty",
        "tsuki-2-203-101.sty",
        "tsuki-2-203-106.sty",
        "qkana.sty",
        *config().m_key_profile->m_kana_fundamental_table};

    auto profile = static_cast<int>(*config().m_key_profile->m_kana_table_enum);

    return key_profile[profile];
}

std::string AnthyEngine::nicolaTableName() {
    const std::string key_profile[] = {
        "",
        "nicola-a.sty",
        "nicola-f.sty",
        "nicola-j.sty",
        "oasys100j.sty"
        "tron-dvorak.sty",
        "tron-qwerty-jp.sty",
        *config().m_key_profile->m_nicola_fundamental_table};

    auto profile =
        static_cast<int>(*config().m_key_profile->m_nicola_table_enum);

    return key_profile[profile];
}

std::string AnthyEngine::fullFileName(const std::string &name) {
    if (name.empty()) {
        return {};
    }
    return fcitx::StandardPath::global().locate(
        fcitx::StandardPath::Type::Config,
        fcitx::stringutils::joinPath("anthy", name));
}

void AnthyEngine::reloadConfig() {
    fcitx::readAsIni(config_, "conf/anthy.conf");

    std::string file;

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
        style.getString(keystr, "KeyBindings", str);                           \
        keyProfile_.m_hk_##key = fcitx::Key::keyListFromString(keystr);        \
    } while (0)
#include "action_defs.h"
#undef FOREACH_ACTION
    }

    customRomajiTableLoaded_ = false;
    customRomajiTable_.clear();
    const char *section_romaji = "RomajiTable/FundamentalTable";
    filename = fullFileName(romajiTableName());
    FCITX_ANTHY_DEBUG() << "Romaji: " << filename;
    if (!filename.empty() && style.load(filename)) {
        customRomajiTableLoaded_ = true;
        customRomajiTable_ = style.key2kanaTable(section_romaji);
    }

    // load custom kana table
    customKanaTableLoaded_ = false;
    customKanaTable_.clear();
    const char *section_kana = "KanaTable/FundamentalTable";
    filename = fullFileName(kanaTableName());
    if (!filename.empty() && style.load(filename)) {
        customKanaTableLoaded_ = true;
        customRomajiTable_ = style.key2kanaTable(section_kana);
    }

    customNicolaTableLoaded_ = false;
    customNicolaTable_.clear();
    const char *section_nicola = "NICOLATable/FundamentalTable";
    filename = fullFileName(nicolaTableName());
    if (!filename.empty() && style.load(filename)) {
        customNicolaTableLoaded_ = true;
        customNicolaTable_ = style.key2kanaTable(section_nicola);
    }

    if (factory_.registered()) {
        instance_->inputContextManager().foreach(
            [this](fcitx::InputContext *ic) {
                auto state = this->state(ic);
                state->configure();
                return true;
            });
    };
}

std::string AnthyEngine::subMode(const fcitx::InputMethodEntry &,
                                 fcitx::InputContext &ic) {
    if (auto status = inputModeStatus(this, &ic)) {
        return _(status->description);
    }
    return "";
}

void AnthyEngine::activate(const fcitx::InputMethodEntry &,
                           fcitx::InputContextEvent &event) {
    // Setup action.
    if (*config_.m_interface->m_show_input_mode_label) {
        event.inputContext()->statusArea().addAction(
            fcitx::StatusGroup::InputMethod, inputModeAction_.get());
    }
    if (*config_.m_interface->m_show_typing_method_label) {
        event.inputContext()->statusArea().addAction(
            fcitx::StatusGroup::InputMethod, typingMethodAction_.get());
    }
    if (*config_.m_interface->m_show_conv_mode_label) {
        event.inputContext()->statusArea().addAction(
            fcitx::StatusGroup::InputMethod, conversionModeAction_.get());
    }
    if (*config_.m_interface->m_show_period_style_label) {
        event.inputContext()->statusArea().addAction(
            fcitx::StatusGroup::InputMethod, periodStyleAction_.get());
    }
    if (*config_.m_interface->m_show_symbol_style_label) {
        event.inputContext()->statusArea().addAction(
            fcitx::StatusGroup::InputMethod, symbolStyleAction_.get());
    }
}

void AnthyEngine::deactivate(const fcitx::InputMethodEntry &,
                             fcitx::InputContextEvent &event) {
    auto anthy = event.inputContext()->propertyFor(&factory_);
    anthy->autoCommit(event);
    event.inputContext()->statusArea().clearGroup(
        fcitx::StatusGroup::InputMethod);
}

FCITX_ADDON_FACTORY(AnthyFactory);
