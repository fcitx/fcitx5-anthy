/*
 * SPDX-FileCopyrightText: 2017-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_ANTHY_ENGINE_H_
#define _FCITX5_ANTHY_ENGINE_H_

#include "action.h"
#include "config.h"
#include "key2kana_table.h"
#include <anthy/anthy.h>
#include <fcitx-config/enum.h>
#include <fcitx-config/iniparser.h>
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/log.h>
#include <fcitx/action.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/candidatelist.h>
#include <fcitx/event.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>
#include <fcitx/menu.h>
#include <memory>
#include <string>
#include <vector>

FCITX_DECLARE_LOG_CATEGORY(anthy_logcategory);

#define FCITX_ANTHY_DEBUG() FCITX_LOGC(anthy_logcategory, Debug)
#define FCITX_ANTHY_INFO() FCITX_LOGC(anthy_logcategory, Info)

class AnthyEngine : public fcitx::InputMethodEngineV3 {
public:
    AnthyEngine(fcitx::Instance *instance);
    ~AnthyEngine();
    void keyEvent(const fcitx::InputMethodEntry &entry,
                  fcitx::KeyEvent &keyEvent) override;
    void reloadConfig() override;
    std::string subMode(const fcitx::InputMethodEntry & /*entry*/,
                        fcitx::InputContext & /*inputContext*/) override;
    void activate(const fcitx::InputMethodEntry & /*entry*/,
                  fcitx::InputContextEvent & /*event*/) override;
    void deactivate(const fcitx::InputMethodEntry &entry,
                    fcitx::InputContextEvent &event) override;
    void reset(const fcitx::InputMethodEntry & /*entry*/,
               fcitx::InputContextEvent & /*event*/) override;
    void invokeActionImpl(const fcitx::InputMethodEntry &entry,
                          fcitx::InvokeActionEvent &event) override;

    const fcitx::Configuration *getConfig() const override { return &config_; }
    void setConfig(const fcitx::RawConfig &config) override {
        config_.load(config, true);
        saveConfig();
        populateConfig();
    }

    void saveConfig() { fcitx::safeSaveAsIni(config_, "conf/anthy.conf"); }

    auto state(fcitx::InputContext *ic) { return ic->propertyFor(&factory_); }
    auto &config() const { return config_; }
    auto keyProfile() { return keyProfileFileLoaded_ ? &keyProfile_ : nullptr; }
    auto customRomajiTable() {
        return customRomajiTableLoaded_ ? &customRomajiTable_ : nullptr;
    }
    auto customKanaTable() {
        return customKanaTableLoaded_ ? &customKanaTable_ : nullptr;
    }
    auto customNicolaTable() {
        return customNicolaTableLoaded_ ? &customNicolaTable_ : nullptr;
    }

    FCITX_ADDON_DEPENDENCY_LOADER(clipboard, instance_->addonManager());

    bool constructed() const { return constructed_; }
    auto inputModeAction() { return inputModeAction_.get(); }
    auto typingMethodAction() { return typingMethodAction_.get(); }
    auto conversionModeAction() { return conversionModeAction_.get(); }
    auto periodStyleAction() { return periodStyleAction_.get(); }
    auto symbolStyleAction() { return symbolStyleAction_.get(); }

    std::string subModeLabelImpl(const fcitx::InputMethodEntry & /*unused*/,
                                 fcitx::InputContext & /*unused*/) override;

    const auto &propertyFactory() const { return factory_; }

    ConversionMode conversionMode() const {
        return *config_.general->conversionMode;
    }
    PeriodCommaStyle periodCommaStyle() const {
        return *config_.general->periodCommaStyle;
    }
    SymbolStyle symbolStyle() const { return *config_.general->symbolStyle; }
    TypingMethod typingMethod() const { return *config_.general->typingMethod; }

    void setPeriodCommaStyle(PeriodCommaStyle period) {
        setAndPopulateOption(config_.general.mutableValue()->periodCommaStyle,
                             period);
    }

    void setSymbolStyle(SymbolStyle symbol) {
        setAndPopulateOption(config_.general.mutableValue()->symbolStyle,
                             symbol);
    }

    void setConversionMode(ConversionMode mode) {
        setAndPopulateOption(config_.general.mutableValue()->conversionMode,
                             mode);
    }

    void setTypingMethod(TypingMethod method) {
        setAndPopulateOption(config_.general.mutableValue()->typingMethod,
                             method);
    }

private:
    void populateConfig();
    void populateOptionToState();

    template <typename OptionT, typename ValueT>
    void setAndPopulateOption(OptionT &option, const ValueT &value) {
        option.setValue(value);
        saveConfig();
        populateOptionToState();
    }

    std::string keyProfileName() const;
    std::string romajiTableName() const;
    std::string kanaTableName() const;
    std::string nicolaTableName() const;
    std::string fullFileName(const std::string &name);

    bool constructed_ = false;
    fcitx::Instance *instance_;
    fcitx::FactoryFor<AnthyState> factory_;
    AnthyConfig config_;

    bool keyProfileFileLoaded_ = false;
    AnthyKeyProfile keyProfile_;

    bool customRomajiTableLoaded_ = false;
    Key2KanaTable customRomajiTable_{"custom-romaji"};

    bool customKanaTableLoaded_ = false;
    Key2KanaTable customKanaTable_{"custom-kana"};

    bool customNicolaTableLoaded_ = false;
    Key2KanaTable customNicolaTable_{"custom-nicola"};

    std::unique_ptr<fcitx::Action> inputModeAction_;
    std::unique_ptr<fcitx::Action> typingMethodAction_;
    std::unique_ptr<fcitx::Action> conversionModeAction_;
    std::unique_ptr<fcitx::Action> periodStyleAction_;
    std::unique_ptr<fcitx::Action> symbolStyleAction_;
    std::vector<std::unique_ptr<fcitx::Action>> subModeActions_;

    fcitx::Menu inputModeMenu_;
    fcitx::Menu typingMethodMenu_;
    fcitx::Menu conversionModeMenu_;
    fcitx::Menu periodStyleMenu_;
    fcitx::Menu symbolStyleMenu_;
};

#endif // _FCITX5_ANTHY_ENGINE_H_
