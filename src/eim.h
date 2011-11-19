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

#ifdef __cplusplus
#define __EXPORT_API extern "C"
#else
#define __EXPORT_API
#endif

#define _(x) gettext(x)

struct _FcitxAnthyTypingRule;

typedef struct _FcitxAnthyConfig {
    GenericConfig gconfig;
    int iAnthyPriority;
} FcitxAnthyConfig;

CONFIG_BINDING_DECLARE(FcitxAnthyConfig);
__EXPORT_API void* FcitxAnthyCreate(FcitxInstance* instance);
__EXPORT_API void FcitxAnthyDestroy(void* arg);
__EXPORT_API INPUT_RETURN_VALUE FcitxAnthyDoInput(void* arg, FcitxKeySym sym, unsigned int state);
__EXPORT_API INPUT_RETURN_VALUE FcitxAnthyGetCandWords(void *arg);
__EXPORT_API INPUT_RETURN_VALUE FcitxAnthyGetCandWord(void *arg, CandidateWord* candWord);
__EXPORT_API boolean FcitxAnthyInit(void*);
__EXPORT_API void ReloadConfigFcitxAnthy(void*);

/* Input Mode */
typedef enum _InputMode {
    INPUT_MODE_HIRAGANA,
    INPUT_MODE_KATAKANA,
    INPUT_MODE_HALF_WIDTH_KATAKANA,
    INPUT_MODE_LATIN,
    INPUT_MODE_WIDE_LATIN
} InputMode;

/* Typing Mode */
typedef enum _TypingMode {
    TYPING_MODE_ROMAJI,
    TYPING_MODE_KANA
} TypingMode;

#define MAX_INPUT_COUNT 1000
#define MAX_ROMAJI_COUNT 3

typedef struct _FcitxAnthyInputState {
    char input_buffer[MAX_INPUT_COUNT];
    char romaji_buffer[MAX_ROMAJI_COUNT + 1];
    int input_count;
    int romaji_count;
} FcitxAnthyInputState;

typedef struct _FcitxAnthy {
    FcitxAnthyConfig fa;
    FcitxInstance* owner;
    anthy_context_t context;

    InputMode input_mode;
    TypingMode typing_mode;

    FcitxAnthyInputState input_state;

    struct _FcitxAnthyTypingRule* rule;
} FcitxAnthy;

#endif
