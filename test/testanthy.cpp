/*
 * SPDX-FileCopyrightText: 2021-2021 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "testdir.h"
#include "testfrontend_public.h"
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/capabilityflags.h>
#include <fcitx-utils/eventdispatcher.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/macros.h>
#include <fcitx-utils/testing.h>
#include <fcitx/addonmanager.h>
#include <fcitx/candidatelist.h>
#include <fcitx/inputmethodgroup.h>
#include <fcitx/inputmethodmanager.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>

using namespace fcitx;

void setup(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *anthy = instance->addonManager().addon("anthy", true);
        FCITX_ASSERT(anthy);

        auto defaultGroup = instance->inputMethodManager().currentGroup();
        defaultGroup.inputMethodList().clear();
        defaultGroup.inputMethodList().push_back(InputMethodGroupItem("anthy"));
        defaultGroup.setDefaultInputMethod("");
        instance->inputMethodManager().setGroup(defaultGroup);
    });
}

void testSpace(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *anthy = instance->addonManager().addon("anthy", true);
        FCITX_ASSERT(anthy);
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);

        RawConfig config;
        config.setValueByPath("General/SpaceType", "Wide");
        anthy->setConfig(config);

        testfrontend->call<ITestFrontend::pushCommitExpectation>("　");
        FCITX_ASSERT(testfrontend->call<ITestFrontend::sendKeyEvent>(
            uuid, Key("space"), false));

        config.setValueByPath("General/SpaceType", "Half");
        anthy->setConfig(config);

        FCITX_ASSERT(!testfrontend->call<ITestFrontend::sendKeyEvent>(
            uuid, Key("space"), false));
    });
}

void testNicola(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *anthy = instance->addonManager().addon("anthy", true);
        FCITX_ASSERT(anthy);
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);

        RawConfig config;
        config.setValueByPath("General/TypingMethod", "Nicola");
        anthy->setConfig(config);

        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("Muhenkan"),
                                                    false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("a"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("a"), true);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("Muhenkan"),
                                                    true);
    });
}

void testVirtualConversion(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *anthy = instance->addonManager().addon("anthy", true);
        auto *frontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            frontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        ic->setCapabilityFlags(CapabilityFlag::Preedit);

        RawConfig config;
        config.setValueByPath("General/TypingMethod", "Romaji");
        config.setValueByPath("General/ConversionMode", "Multi segment");
        config.setValueByPath("General/PredictOnInput", "False");
        config.setValueByPath("General/NTriggersToShowCandWin", "2");
        anthy->setConfig(config);

        auto send = [&](KeySym sym, bool virtualKey = true,
                        bool release = false) {
            return frontend->call<ITestFrontend::sendKeyEvent>(
                uuid,
                Key(sym, virtualKey ? KeyState::Virtual : KeyState::NoState),
                release);
        };
        auto type = [&](const std::string &text, bool virtualKey = true) {
            for (unsigned char c : text) {
                FCITX_ASSERT(send(static_cast<KeySym>(c), virtualKey));
            }
        };
        auto reading = [&]() {
            return ic->inputPanel().clientPreedit().toString();
        };
        auto hasCandidates = [&]() {
            auto list = ic->inputPanel().candidateList();
            return list && list->size() > 0;
        };
        auto selectFirst = [&]() {
            auto list = ic->inputPanel().candidateList();
            FCITX_ASSERT(list && list->size() > 0);
            frontend->call<ITestFrontend::pushCommitExpectation>(
                list->candidate(0).text().toString());
            list->candidate(0).select(ic);
        };

        // Press-only virtual keyboards get candidates without a space or
        // release.
        type("ka");
        FCITX_ASSERT(reading() == "か");
        FCITX_ASSERT(hasCandidates());
        send(FcitxKey_a, true, true);
        FCITX_ASSERT(reading() == "か");
        type("na");
        FCITX_ASSERT(reading() == "かな");
        FCITX_ASSERT(hasCandidates());
        selectFirst();
        FCITX_ASSERT(reading().empty());
        FCITX_ASSERT(!hasCandidates());

        // A final 'n' is converted on selection without requiring a space.
        type("nihon");
        FCITX_ASSERT(reading() == "にほn");
        auto list = ic->inputPanel().candidateList();
        FCITX_ASSERT(list && list->toBulk());
        bool found = false;
        for (int i = 0; i < list->toBulk()->totalSize(); i++) {
            const auto &candidate = list->toBulk()->candidateFromAll(i);
            if (candidate.text().toString() == "日本") {
                frontend->call<ITestFrontend::pushCommitExpectation>("日本");
                candidate.select(ic);
                found = true;
                break;
            }
        }
        FCITX_ASSERT(found);
        FCITX_ASSERT(reading().empty());

        // Previewing must not finish pending romaji (in particular, 'n').
        type("kan");
        FCITX_ASSERT(reading() == "かn");
        type("a");
        FCITX_ASSERT(reading() == "かな");
        FCITX_ASSERT(send(FcitxKey_BackSpace));
        FCITX_ASSERT(reading() == "か");
        FCITX_ASSERT(hasCandidates());
        FCITX_ASSERT(send(FcitxKey_BackSpace));
        FCITX_ASSERT(reading().empty());
        FCITX_ASSERT(!hasCandidates());

        // Digits continue the reading instead of selecting a preview candidate.
        type("ka1");
        FCITX_ASSERT(reading() == "か1");
        ic->reset();

        // Switching to physical keys keeps the reading and restores manual
        // conversion.
        type("ka");
        type("na", false);
        FCITX_ASSERT(reading() == "かな");
        FCITX_ASSERT(!hasCandidates());
        ic->reset();
        type("kana", false);
        FCITX_ASSERT(!hasCandidates());
        FCITX_ASSERT(send(FcitxKey_space, false));
        FCITX_ASSERT(!hasCandidates());
        FCITX_ASSERT(send(FcitxKey_space, false));
        FCITX_ASSERT(hasCandidates());
        ic->reset();

        // A tap commits one segment and exposes candidates for the remaining
        // text.
        type("watashihanihongowohanashimasu");
        selectFirst();
        FCITX_ASSERT(!reading().empty());
        FCITX_ASSERT(hasCandidates());
        while (hasCandidates()) {
            selectFirst();
        }
        FCITX_ASSERT(reading().empty());

        config.setValueByPath("General/ConversionMode", "Single segment");
        anthy->setConfig(config);
        type("watashihanihongowohanashimasu");
        selectFirst();
        FCITX_ASSERT(reading().empty());
        FCITX_ASSERT(!hasCandidates());

        config.setValueByPath("General/ConversionMode",
                              "Multi segment immediate");
        anthy->setConfig(config);
        type("kana");
        FCITX_ASSERT(hasCandidates());
        FCITX_ASSERT(send(FcitxKey_BackSpace));
        FCITX_ASSERT(hasCandidates());
        ic->reset();

        config.setValueByPath("General/ConversionMode", "Multi segment");
        anthy->setConfig(config);
    });
}

int main() {
    setupTestingEnvironment(TESTING_BINARY_DIR, {"bin"},
                            {TESTING_BINARY_DIR "/test"});
    // fcitx::Log::setLogRule("default=5,table=5,libime-table=5");
    char arg0[] = "testanthy";
    char arg1[] = "--disable=all";
    char arg2[] = "--enable=testim,testfrontend,anthy,testui";
    char *argv[] = {arg0, arg1, arg2};
    fcitx::Log::setLogRule("default=5,anthy=5");
    Instance instance(FCITX_ARRAY_SIZE(argv), argv);
    instance.addonManager().registerDefaultLoader(nullptr);
    setup(&instance);
    testSpace(&instance);
    testVirtualConversion(&instance);
    testNicola(&instance);
    instance.eventDispatcher().schedule([&instance]() { instance.exit(); });
    instance.exec();

    return 0;
}
