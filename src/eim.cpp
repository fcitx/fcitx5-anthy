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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
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
#include <string>
#include <libintl.h>

#include <anthy/anthy.h>

#include "eim.h"

#ifdef __cplusplus
extern "C" {
#endif
FCITX_EXPORT_API
FcitxIMClass ime = {
    FcitxAnthyCreate,
    FcitxAnthyDestroy
};
#ifdef __cplusplus
}
#endif

CONFIG_DESC_DEFINE(GetAnthyConfigDesc, "fcitx-anthy.desc")

boolean LoadAnthyConfig(FcitxAnthyConfig* fs);
static void SaveAnthyConfig(FcitxAnthyConfig* fs);
static void ConfigAnthy(FcitxAnthy* anthy);

/**
 * @brief Reset the status.
 *
 **/
__EXPORT_API
void FcitxAnthyReset (void* arg)
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

    return IRV_TO_PROCESS;
}

boolean FcitxAnthyInit(void* arg)
{
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
    FcitxAnthy* anthy = (FcitxAnthy* )arg;
    return IRV_DISPLAY_CANDWORDS;
}

/**
 * @brief get the candidate word by index
 *
 * @param iIndex index of candidate word
 * @return the string of canidate word
 **/
__EXPORT_API
INPUT_RETURN_VALUE FcitxAnthyGetCandWord (void* arg, CandidateWord* candWord)
{
    FcitxAnthy* anthy = (FcitxAnthy* )arg;    
    return IRV_DO_NOTHING;
}

/**
 * @brief initialize the extra input method
 *
 * @param arg
 * @return successful or not
 **/
__EXPORT_API
void* FcitxAnthyCreate (FcitxInstance* instance)
{
    FcitxAnthy* anthy = (FcitxAnthy*) fcitx_malloc0(sizeof(FcitxAnthy));
    bindtextdomain("fcitx-anthy", LOCALEDIR);
    anthy->owner = instance;
    
    if (LoadAnthyConfig(&anthy->fa))
    {
        free(anthy);
        return NULL;
    }
    ConfigAnthy(anthy);
    
    anthy_init();
    anthy->context = anthy_create_context();
    anthy_context_set_encoding(anthy->context, ANTHY_UTF8_ENCODING);
    
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
void FcitxAnthyDestroy (void* arg)
{
    FcitxAnthy* anthy = (FcitxAnthy*) arg;
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

    if (!fp)
    {
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
