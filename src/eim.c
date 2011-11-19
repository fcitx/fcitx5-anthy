/***************************************************************************
 *   Copyright (C) 2010~2010 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcitx/ime.h>
#include <fcitx-config/hotkey.h>
#include <fcitx-config/xdg.h>
#include <fcitx-utils/log.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx-utils/utils.h>
#include <fcitx/instance.h>
#include <fcitx/keys.h>
#include <fcitx/ui.h>
#include <libintl.h>

#include <anthy/anthy.h>

#include "eim.h"
#include "tables.h"
#include "convert.h"

FCITX_EXPORT_API
FcitxIMClass ime = {
    FcitxAnthyCreate,
    FcitxAnthyDestroy
};
FCITX_EXPORT_API
int ABI_VERSION = FCITX_ABI_VERSION;

CONFIG_DESC_DEFINE(GetAnthyConfigDesc, "fcitx-anthy.desc")

boolean LoadAnthyConfig(FcitxAnthyConfig* fs);
static void SaveAnthyConfig(FcitxAnthyConfig* fs);
static void ConfigAnthy(FcitxAnthy* anthy);

/**
 * @brief Reset the status.
 *
 **/
__EXPORT_API
void FcitxAnthyReset(void* arg)
{
    FcitxAnthy* anthy = (FcitxAnthy*) arg;
    anthy_reset_context(anthy->context);
}

/**
 * @brief Process Key Input and return the status
 *
 * @param keycode keycode from XKeyEvent
 * @param state state from XKeyEvent
 * @param count count from XKeyEvent
 * @return INPUT_RETURN_VALUE
 **/
__EXPORT_API
INPUT_RETURN_VALUE FcitxAnthyDoInput(void* arg, FcitxKeySym sym, unsigned int state)
{
    FcitxAnthy* anthy = (FcitxAnthy*) arg;
    FcitxInputState *input = FcitxInstanceGetInputState(anthy->owner);
    Messages *msgPreedit = FcitxInputStateGetPreedit(input);

    CleanInputWindowUp(anthy->owner);

    // todo: if convert key or predict key, convert or predict

    if (IsHotKeySimple(sym, state)) {
        char *romaji_buffer = (anthy->input_state).romaji_buffer;
        int romaji_count = (anthy->input_state).romaji_count;

        romaji_buffer[romaji_count ++] = sym & 0xff;
        romaji_buffer[romaji_count] = '\0';
        (anthy->input_state).romaji_count = strlen(romaji_buffer); 

        FcitxLog(INFO,"input:%c\n",romaji_buffer[romaji_count-1]);

        FcitxAnthyConvertRomajiToKana(anthy);

        FcitxLog(INFO,"romaji_buffer:%s count:%d\n",anthy->input_state.romaji_buffer,anthy->input_state.romaji_count);
        FcitxLog(INFO,"Input Display should be:%s%s\n",anthy->input_state.input_buffer,anthy->input_state.romaji_buffer);
        AddMessageAtLast(msgPreedit, MSG_INPUT, "%s", anthy->input_state.input_buffer);
        if(romaji_buffer[0])
            AddMessageAtLast(msgPreedit, MSG_INPUT, "%s", anthy->input_state.romaji_buffer);

        return IRV_DISPLAY_MESSAGE;
    } else {

        // todo: if is not printable character
        return IRV_TO_PROCESS;
    }
}

boolean FcitxAnthyInit(void* arg)
{
    FcitxAnthy* anthy = (FcitxAnthy*)arg;
    anthy->input_state.input_buffer[0] = '\0';
    anthy->input_state.input_count = 0;
    anthy->input_state.romaji_buffer[0] = '\0';
    anthy->input_state.romaji_count = 0;

    return true;
}


/**
 * @brief function DoInput has done everything for us.
 *
 * @param searchMode
 * @return INPUT_RETURN_VALUE
 **/
__EXPORT_API
INPUT_RETURN_VALUE FcitxAnthyGetCandWords(void* arg)
{
    FcitxAnthy* anthy = (FcitxAnthy*)arg;
    return IRV_DISPLAY_CANDWORDS;
}

/**
 * @brief get the candidate word by index
 *
 * @param iIndex index of candidate word
 * @return the string of canidate word
 **/
__EXPORT_API
INPUT_RETURN_VALUE FcitxAnthyGetCandWord(void* arg, CandidateWord* candWord)
{
    FcitxAnthy* anthy = (FcitxAnthy*)arg;
    return IRV_DO_NOTHING;
}

/**
 * @brief initialize the extra input method
 *
 * @param arg
 * @return successful or not
 **/
__EXPORT_API
void* FcitxAnthyCreate(FcitxInstance* instance)
{
    FcitxAnthy* anthy = (FcitxAnthy*) fcitx_malloc0(sizeof(FcitxAnthy));
    bindtextdomain("fcitx-anthy", LOCALEDIR);
    anthy->owner = instance;

    if (!LoadAnthyConfig(&anthy->fa)) {
        free(anthy);
        return NULL;
    }

    FcitxAnthyInitTypingTables(anthy);

    anthy_init();
    anthy->context = anthy_create_context();
    anthy_context_set_encoding(anthy->context, ANTHY_UTF8_ENCODING);
    anthy->input_mode = INPUT_MODE_HIRAGANA;
    anthy->typing_mode = TYPING_MODE_ROMAJI;

    ConfigAnthy(anthy);

    FcitxRegisterIM(instance,
                    anthy,
                    _("Anthy"),
                    "anthy",
                    FcitxAnthyInit,
                    FcitxAnthyReset,
                    FcitxAnthyDoInput,
                    FcitxAnthyGetCandWords,
                    NULL,
                    NULL,
                    ReloadConfigFcitxAnthy,
                    NULL,
                    anthy->fa.iAnthyPriority
                   );
    return anthy;
}

/**
 * @brief Destroy the input method while unload it.
 *
 * @return int
 **/
__EXPORT_API
void FcitxAnthyDestroy(void* arg)
{
    FcitxAnthy* anthy = (FcitxAnthy*) arg;
    FcitxAnthyClearTypingTables(anthy);
    anthy_release_context(anthy->context);
    anthy_quit();
    free(arg);
}

/**
 * @brief Load the config file for fcitx-anthy
 *
 * @param Bool is reload or not
 **/
boolean LoadAnthyConfig(FcitxAnthyConfig* fs)
{
    ConfigFileDesc *configDesc = GetAnthyConfigDesc();
    if (!configDesc)
        return false;

    FILE *fp = GetXDGFileUserWithPrefix("conf", "fcitx-anthy.config", "rt", NULL);

    if (!fp) {
        if (errno == ENOENT)
            SaveAnthyConfig(fs);
    }
    ConfigFile *cfile = ParseConfigFileFp(fp, configDesc);

    FcitxAnthyConfigConfigBind(fs, cfile, configDesc);
    ConfigBindSync(&fs->gconfig);

    if (fp)
        fclose(fp);
    return true;
}

void ConfigAnthy(FcitxAnthy* anthy)
{
}

__EXPORT_API void ReloadConfigFcitxAnthy(void* arg)
{
    FcitxAnthy* anthy = (FcitxAnthy*) arg;
    LoadAnthyConfig(&anthy->fa);
    ConfigAnthy(anthy);
}

/**
 * @brief Save the config
 *
 * @return void
 **/
void SaveAnthyConfig(FcitxAnthyConfig* fa)
{
    ConfigFileDesc *configDesc = GetAnthyConfigDesc();
    FILE *fp = GetXDGFileUserWithPrefix("conf", "fcitx-anthy.config", "wt", NULL);
    SaveConfigFileFp(fp, &fa->gconfig, configDesc);
    if (fp)
        fclose(fp);
}
