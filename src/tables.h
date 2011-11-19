/***************************************************************************
 *   Copyright (C) 2010~2010 by Margaret Wang                              *
 *   pipituliuliu@gmail.com                                                *
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

#ifndef TABLES_H
#define TABLES_H

struct _FcitxAnthy;

#define MAX_NR_ROMAJI_MATCH 4

/* Initialize typing tables */
int FcitxAnthyInitTypingTables(struct _FcitxAnthy* anthy);

/* look up in romaji typing tables for the corresponding kana */
int FcitxAnthyLookupKanaForRomaji(struct _FcitxAnthy* anthy, const char *romaji, const char **kana);

/* Clear typing tables */
int FcitxAnthyClearTypingTables(struct _FcitxAnthy* anthy);

#endif
