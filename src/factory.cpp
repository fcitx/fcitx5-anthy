/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) Hiroyuki Ikezoe <poincare@ikezoe.net>
 *  Copyright (C) 2004 Takuro Ashie <ashie@homa.ne.jp>
 *  Copyright (C) 2012 CSSlayer
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*
 * The original code is scim_uim_imengine.cpp in scim-uim-0.1.3.
 * Copyright (C) 2004 James Su <suzhe@tsinghua.org.cn>
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcitx/instance.h>
#include <libintl.h>
#include <fcitx-config/xdg.h>
#include <fcitx/hook.h>
#include <fcitx-utils/log.h>
#include <errno.h>

#include "factory.h"
#include "imengine.h"

static void* FcitxAnthyCreate(FcitxInstance* instance);
static void  FcitxAnthyDestory(void* arg);
static boolean FcitxAnthyInit(void* arg);
static INPUT_RETURN_VALUE  FcitxAnthyDoInput(void* arg, FcitxKeySym sym, unsigned int state);
static INPUT_RETURN_VALUE FcitxAnthyDoReleaseInput(void* arg, FcitxKeySym sym, unsigned int state);
static void  FcitxAnthyReloadConfig(void* arg);
static void  FcitxAnthySave(void* arg);
static void  FcitxAnthyReset(void* arg);
static void  FcitxAnthyResetIM(void* arg);
static void FcitxAnthyOnClose(void* arg, FcitxIMCloseEventType event);

FCITX_DEFINE_PLUGIN(fcitx_anthy, ime2, FcitxIMClass2) = {
    FcitxAnthyCreate,
    FcitxAnthyDestory,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

void* FcitxAnthyCreate(FcitxInstance* instance)
{
    if (anthy_init())
        return NULL;

    AnthyInstance* anthy = new AnthyInstance(instance);
    if (!anthy->load_config()) {
        anthy_quit();
        delete anthy;
        return NULL;
    }

    bindtextdomain("fcitx-anthy", LOCALEDIR);

    FcitxIMIFace iface;
    memset(&iface, 0, sizeof(FcitxIMIFace));
    iface.Init = FcitxAnthyInit;
    iface.ResetIM = FcitxAnthyResetIM;
    iface.DoInput = FcitxAnthyDoInput;
    iface.DoReleaseInput = FcitxAnthyDoReleaseInput;
    iface.ReloadConfig = FcitxAnthyReloadConfig;
    iface.Save = FcitxAnthySave;
    iface.OnClose = FcitxAnthyOnClose;

    FcitxInstanceRegisterIMv2(
        instance,
        anthy,
        "anthy",
        _("Anthy"),
        "anthy",
        iface,
        1,
        "ja"
    );

    FcitxIMEventHook hk;
    hk.arg = anthy;
    hk.func = FcitxAnthyReset;
    FcitxInstanceRegisterResetInputHook(instance, hk);

    return anthy;
}

void FcitxAnthyDestory(void* arg)
{
    AnthyInstance* anthy = (AnthyInstance*) arg;
    delete anthy;

    anthy_quit();
}

boolean FcitxAnthyInit(void* arg)
{
    AnthyInstance* anthy = (AnthyInstance*) arg;
    anthy->init();
    anthy->update_ui();
    return true;
}

void FcitxAnthyResetIM(void* arg)
{
    AnthyInstance* anthy = (AnthyInstance*) arg;
    anthy->reset_im();
    anthy->update_ui();
}

void FcitxAnthyReset(void* arg)
{
    AnthyInstance* anthy = (AnthyInstance*) arg;
    anthy->reset();
    anthy->update_ui();
}

void FcitxAnthySave(void* arg)
{
}

INPUT_RETURN_VALUE FcitxAnthyDoInput(void* arg, FcitxKeySym sym, unsigned int state)
{
    AnthyInstance* anthy = (AnthyInstance*) arg;
    KeyEvent event;
    FcitxInputState* input = FcitxInstanceGetInputState(anthy->get_owner());
    event.sym = (FcitxKeySym) FcitxInputStateGetKeySym(input);
    event.is_release = false;
    event.state = FcitxInputStateGetKeyState(input) & FcitxKeyState_SimpleMask;
    bool result = anthy->process_key_event(event);
    anthy->update_ui();
    if (result)
        return IRV_DO_NOTHING;
    else
        return IRV_TO_PROCESS;
}

INPUT_RETURN_VALUE FcitxAnthyDoReleaseInput(void* arg, FcitxKeySym sym, unsigned int state)
{
    AnthyInstance* anthy = (AnthyInstance*) arg;
    KeyEvent event;
    FcitxInputState* input = FcitxInstanceGetInputState(anthy->get_owner());
    event.sym = (FcitxKeySym) FcitxInputStateGetKeySym(input);
    event.is_release = true;
    event.state = FcitxInputStateGetKeyState(input) & FcitxKeyState_SimpleMask;
    bool result = anthy->process_key_event(event);
    anthy->update_ui();
    if (result)
        return IRV_DO_NOTHING;
    else
        return IRV_TO_PROCESS;
}

void FcitxAnthyReloadConfig(void* arg)
{
    AnthyInstance* anthy = (AnthyInstance*) arg;
    anthy->load_config();
    anthy->configure();
    anthy->update_ui();
}

void FcitxAnthyOnClose(void* arg, FcitxIMCloseEventType event)
{
    AnthyInstance* anthy = (AnthyInstance*) arg;
    anthy->auto_commit(event);
}


void
SaveAnthyConfig(AnthyInstance* anthy)
{
    anthy->save_config();
}

boolean
LoadAnthyConfig(AnthyInstance* anthy)
{
    anthy->load_config();
    return true;
}

void ConfigAnthy(AnthyInstance* anthy) {
    anthy->configure();
    anthy->update_ui();
}

