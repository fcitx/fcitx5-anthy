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

/* Initialize typing tables */
int init_typing_tables();

/* look up in romaji typing tables for the corresponding kana */
int lookup_kana_for_romaji(const char *romaji, char **kana, char **extra_romaji);

/* Clear typing tables */
int clear_typing_tables();

#endif
