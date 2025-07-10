/*
 * SPDX-FileCopyrightText: 2021-2021 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "testdir.h"
#include "testfrontend_public.h"
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/eventdispatcher.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/macros.h>
#include <fcitx-utils/testing.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputmethodgroup.h>
#include <fcitx/inputmethodmanager.h>
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
    testNicola(&instance);
    instance.eventDispatcher().schedule([&instance]() { instance.exit(); });
    instance.exec();

    return 0;
}
