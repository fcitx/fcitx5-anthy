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
#include "preedit.h"

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
static bool process_key_event_input (FcitxAnthy* anthy, FcitxKeySym sym, unsigned int state);
static bool process_key_event_lookup_keybind (FcitxAnthy* anthy, FcitxKeySym sym, unsigned int state);
static bool is_realtime_conversion (FcitxAnthy* anthy);
static bool is_single_segment (FcitxAnthy* anthy);
static bool action_revert (FcitxAnthy* anthy);
static bool action_commit (FcitxAnthy* anthy, bool learn);
static void select_candidate_no_direct(FcitxAnthy* anthy, int item);
static bool action_select_next_segment (FcitxAnthy* anthy);
static void set_preedition(FcitxAnthy* anthy);

/**
 * @brief Reset the status.
 *
 **/
__EXPORT_API
void FcitxAnthyReset(void* arg)
{
    FcitxAnthy* anthy = (FcitxAnthy*) arg;
    anthy->preedit->clear();
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

    FcitxInstanceCleanInputWindowUp(anthy->owner);
    
    // lookup user defined key bindings
    if (process_key_event_lookup_keybind (anthy, sym, state))
        return IRV_DISPLAY_MESSAGE;

    // for other mode
    if (process_key_event_input (anthy, sym, state))
        return IRV_DISPLAY_MESSAGE;

    if (anthy->preedit->is_preediting ())
        return IRV_DISPLAY_MESSAGE;
    else
        return IRV_TO_PROCESS;
}

boolean FcitxAnthyInit(void* arg)
{
    FcitxAnthy* anthy = (FcitxAnthy*)arg;

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
INPUT_RETURN_VALUE FcitxAnthyGetCandWord(void* arg, FcitxCandidateWord* candWord)
{
    FcitxAnthy* anthy = (FcitxAnthy*)arg;
    FcitxInputState* input = FcitxInstanceGetInputState(anthy->owner);
    struct _FcitxCandidateWordList* candList = FcitxInputStateGetCandidateList(input);
    
    FcitxAnthyCandWord* priv = (FcitxAnthyCandWord*) candWord->priv;

    select_candidate_no_direct (anthy, priv->item);

    if (anthy->config.m_close_cand_win_on_select) {
        FcitxCandidateWordReset(candList);
        action_select_next_segment(anthy);
    }
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
    FcitxAnthy* anthy = (FcitxAnthy*) fcitx_utils_malloc0(sizeof(FcitxAnthy));
    bindtextdomain("fcitx-anthy", LOCALEDIR);
    anthy->owner = instance;

    if (!LoadAnthyConfig(&anthy->config)) {
        free(anthy);
        return NULL;
    }

    anthy_init();
    anthy->preedit = new Preedit(anthy);
    anthy->input_mode = SCIM_ANTHY_MODE_HIRAGANA;
    anthy->typing_method = SCIM_ANTHY_TYPING_METHOD_ROMAJI;

    ConfigAnthy(anthy);

    FcitxInstanceRegisterIM(
        instance,
        anthy,
        "anthy",
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
        anthy->config.iAnthyPriority,
        "ja_JP"
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
    delete(anthy->preedit);
    free(arg);
}

/**
 * @brief Load the config file for fcitx-anthy
 *
 * @param Bool is reload or not
 **/
boolean LoadAnthyConfig(FcitxAnthyConfig* fs)
{
    FcitxConfigFileDesc *configDesc = GetAnthyConfigDesc();
    if (!configDesc)
        return false;

    FILE *fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-anthy.config", "rt", NULL);

    if (!fp) {
        if (errno == ENOENT)
            SaveAnthyConfig(fs);
    }
    FcitxConfigFile *cfile = FcitxConfigParseConfigFileFp(fp, configDesc);

    FcitxAnthyConfigConfigBind(fs, cfile, configDesc);
    FcitxConfigBindSync(&fs->gconfig);

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
    LoadAnthyConfig(&anthy->config);
    ConfigAnthy(anthy);
}

/**
 * @brief Save the config
 *
 * @return void
 **/
void SaveAnthyConfig(FcitxAnthyConfig* fa)
{
    FcitxConfigFileDesc *configDesc = GetAnthyConfigDesc();
    FILE *fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-anthy.config", "wt", NULL);
    FcitxConfigSaveConfigFileFp(fp, &fa->gconfig, configDesc);
    if (fp)
        fclose(fp);
}

bool
process_key_event_input (FcitxAnthy* anthy, FcitxKeySym sym, unsigned int state)
{
    FcitxInputState* input = FcitxInstanceGetInputState(anthy->owner);
    // prediction while typing
    if (anthy->config.m_predict_on_input && 
        anthy->preedit->is_preediting () && !anthy->preedit->is_converting ())
    {
        struct _FcitxCandidateWordList* candList = FcitxInputStateGetCandidateList(input);
        anthy->preedit->predict ();
        anthy->preedit->get_candidates (candList);
    }

    if (!anthy->preedit->can_process_key_event (sym, state)) {
        return false;
    }

    if (anthy->preedit->is_converting ()) {
        if (is_realtime_conversion (anthy)) {
            action_revert (anthy);
        } else {
            action_commit (anthy, anthy->config.m_learn_on_auto_commit);
        }
    }

    bool need_commit = anthy->preedit->process_key_event (sym, state);

    if (need_commit) {
        if (is_realtime_conversion (anthy))
        {
            anthy->preedit->convert (SCIM_ANTHY_CANDIDATE_DEFAULT,
                               is_single_segment (anthy));
        }
        action_commit (anthy, anthy->config.m_learn_on_auto_commit);
    } else {
        if (is_realtime_conversion (anthy)) {
            anthy->preedit->convert (SCIM_ANTHY_CANDIDATE_DEFAULT,
                               is_single_segment (anthy));
            anthy->preedit->select_segment (-1);
        }
        set_preedition (anthy);
    }

    return true;
}


bool
process_key_event_lookup_keybind (FcitxAnthy* anthy, FcitxKeySym sym, unsigned int state)
{
#if 0
    std::vector<Action>::iterator it;

    anthy->m_last_key[0].sym = sym;
    anthy->m_last_key[0].state = state;

    /* try to find a "insert a blank" action to be not stolen a blank key
     * when entering the pseudo  mode.
     */
    if (get_pseudo_ascii_mode () != 0 &&
        m_factory->m_romaji_pseudo_ascii_blank_behavior &&
        anthy->preedit->is_pseudo_ascii_mode ()) {
        for (it  = m_factory->m_actions.begin();
             it != m_factory->m_actions.end();
             it++) {
            if (it->match_action_name ("INSERT_SPACE") &&
                it->perform (this, key)) {
                return true;
            }
        }
    }
    for (it  = m_factory->m_actions.begin();
         it != m_factory->m_actions.end();
         it++)
    {
        if (it->perform (this, key)) {
            m_last_key = KeyEvent ();
            return true;
        }
    }

    m_last_key = KeyEvent ();
#endif

    return false;
}

bool
is_realtime_conversion (FcitxAnthy* anthy)
{
    if (anthy->m_conv_mode == SCIM_ANTHY_CONVERSION_MULTI_SEGMENT_IMMEDIATE ||
        anthy->m_conv_mode == SCIM_ANTHY_CONVERSION_SINGLE_SEGMENT_IMMEDIATE)
        return true;
    else
        return false;
}


bool
is_single_segment (FcitxAnthy* anthy)
{
    if (anthy->m_conv_mode == SCIM_ANTHY_CONVERSION_SINGLE_SEGMENT ||
        anthy->m_conv_mode == SCIM_ANTHY_CONVERSION_SINGLE_SEGMENT_IMMEDIATE)
        return true;
    else
        return false;
}



bool
action_revert (FcitxAnthy* anthy)
{
    FcitxInputState *input = FcitxInstanceGetInputState(anthy->owner);
    struct _FcitxCandidateWordList* candList = FcitxInputStateGetCandidateList(input);
    if (anthy->preedit->is_reconverting ()) {
        anthy->preedit->revert ();
        FcitxInstanceCommitString (anthy->owner, FcitxInstanceGetCurrentIC(anthy->owner), (char*) anthy->preedit->get_string ().c_str());
        FcitxAnthyReset(anthy);
        return true;
    }

    if (!anthy->preedit->is_preediting ())
        return false;

    if (!anthy->preedit->is_converting ()) {
        FcitxAnthyReset(anthy);
        return true;
    }

    if (FcitxCandidateWordGetListSize (candList)) {
        FcitxCandidateWordReset(candList);
    }

    anthy->preedit->revert ();

    return true;
}


bool
action_commit (FcitxAnthy* anthy, bool learn)
{
    if (!anthy->preedit->is_preediting ())
        return false;

    if (anthy->preedit->is_converting ()) {
        FcitxInstanceCommitString (anthy->owner, FcitxInstanceGetCurrentIC(anthy->owner), (char*) anthy->preedit->get_string ().c_str());
        if (learn)
            anthy->preedit->commit ();
    } else {
        anthy->preedit->finish ();
        FcitxInstanceCommitString (anthy->owner, FcitxInstanceGetCurrentIC(anthy->owner), (char*) anthy->preedit->get_string ().c_str());
    }

    FcitxAnthyReset(anthy);

    return true;
}


bool
action_predict (FcitxAnthy* anthy)
{
    if (!anthy->preedit->is_preediting ())
        return false;

    if (anthy->preedit->is_converting ())
        return false;

    if (!anthy->preedit->is_predicting ())
        anthy->preedit->predict ();

    anthy->preedit->select_candidate (0);
    set_preedition (anthy);
    select_candidate_no_direct (anthy, 0);

    return true;
}

void select_candidate_no_direct(FcitxAnthy* anthy, int item)
{
    FcitxInputState* input = FcitxInstanceGetInputState(anthy->owner);
    struct _FcitxCandidateWordList* candList = FcitxInputStateGetCandidateList(input);

    if (anthy->preedit->is_predicting () && !anthy->preedit->is_converting ())
        action_predict (anthy);

    if (!FcitxCandidateWordGetListSize (candList))
        return;

    // update preedit
    anthy->preedit->select_candidate (item);
}

void set_preedition(FcitxAnthy* anthy)
{
    FcitxInputState* input = FcitxInstanceGetInputState(anthy->owner);
    FcitxMessages *msgPreedit = FcitxInputStateGetPreedit(input);
    FcitxMessages *clientPreedit = FcitxInputStateGetClientPreedit(input);
    FcitxMessagesSetMessageCount(msgPreedit, 0);
    FcitxMessagesAddMessageAtLast(msgPreedit, MSG_INPUT,"%s", anthy->preedit->get_string().c_str());
    FcitxMessagesSetMessageCount(clientPreedit, 0);
    FcitxMessagesAddMessageAtLast(clientPreedit, MSG_INPUT, "%s", anthy->preedit->get_string().c_str());
}


bool
action_select_next_segment (FcitxAnthy* anthy)
{
    if (!anthy->preedit->is_converting ())
        return false;
    FcitxInputState* input = FcitxInstanceGetInputState(anthy->owner);
    struct _FcitxCandidateWordList* candList = FcitxInputStateGetCandidateList(input);
    
    FcitxCandidateWordReset(candList);
    int idx = anthy->preedit->get_selected_segment ();
    if (idx < 0) {
        anthy->preedit->select_segment(0);
    } else {
        int n = anthy->preedit->get_nr_segments ();
        if (n <= 0)
            return false;
        if (idx + 1 >= n)
            anthy->preedit->select_segment(0);
        else
            anthy->preedit->select_segment(idx + 1);
    }
    set_preedition (anthy);

    return true;
}