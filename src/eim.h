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

#ifndef EIM_H
#define EIM_H

#include <fcitx/ime.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx/instance.h>
#include <fcitx/candidate.h>
#include <anthy/anthy.h>
#include "common.h"
#include "preedit.h"

#ifdef __cplusplus
#define __EXPORT_API extern "C"
#else
#define __EXPORT_API
#endif

#define _(x) gettext(x)

#define SCIM_ANTHY_CONFIG_DICT_ENCODING_DEFAULT "UTF-8"

struct _FcitxAnthyTypingRule;

typedef struct _FcitxAnthyConfig {
    FcitxGenericConfig gconfig;
    char* m_kana_layout_ro_key;
    char* m_ten_key_type;
    FcitxHotkey m_left_thumb_keys[2];
    FcitxHotkey m_right_thumb_keys[2];
    int iAnthyPriority;
    boolean m_predict_on_input;
    boolean m_learn_on_auto_commit;
    boolean m_romaji_allow_split;
    boolean m_close_cand_win_on_select;
    Key2KanaTable* m_custom_nicola_table;
    Key2KanaTable* m_custom_kana_table;
    Key2KanaTable* m_custom_romaji_table;
} FcitxAnthyConfig;

CONFIG_BINDING_DECLARE(FcitxAnthyConfig);
__EXPORT_API void* FcitxAnthyCreate(FcitxInstance* instance);
__EXPORT_API void FcitxAnthyDestroy(void* arg);
__EXPORT_API INPUT_RETURN_VALUE FcitxAnthyDoInput(void* arg, FcitxKeySym sym, unsigned int state);
__EXPORT_API INPUT_RETURN_VALUE FcitxAnthyGetCandWords(void *arg);
__EXPORT_API INPUT_RETURN_VALUE FcitxAnthyGetCandWord(void *arg, FcitxCandidateWord* candWord);
__EXPORT_API boolean FcitxAnthyInit(void*);
__EXPORT_API void ReloadConfigFcitxAnthy(void*);

#define MAX_INPUT_COUNT 1000
#define MAX_ROMAJI_COUNT 4

typedef enum {
    SCIM_ANTHY_CONVERSION_MULTI_SEGMENT,
    SCIM_ANTHY_CONVERSION_SINGLE_SEGMENT,
    SCIM_ANTHY_CONVERSION_MULTI_SEGMENT_IMMEDIATE,
    SCIM_ANTHY_CONVERSION_SINGLE_SEGMENT_IMMEDIATE,
} ConversionMode;

struct FcitxAnthy {
    FcitxAnthyConfig config;
    FcitxInstance* owner;

    InputMode input_mode;
    TypingMethod typing_method;

    struct _FcitxAnthyTypingRule* rule;
    Preedit* preedit;
    ConversionMode m_conv_mode;
    
    FcitxHotkey m_last_key[2];
};

#endif
