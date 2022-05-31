/*
   hm_lobbyserver - HearthStone HearthMod lobbyserver
   Copyright (C) 2016 Filip Pancik

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>

int step12_0(char *dst, int ndst) {const char src[13] = { 0x00, 0x0B, 0x08, 0xFE, 0x01, 0x18, 0x1A, 0x20, 0x00, 0x28, 0x00, 0x30, 0x00 };
if(ndst >= 13) { memcpy(dst, src, 13); } else { abort(); }
return 13;}

int step12_1(char *dst, int ndst) {const char src[65] = { 0x00, 0x0C, 0x08, 0x05, 0x10, 0x06, 0x18, 0x3A, 0x20, 0x02, 0x28, 0x33, 0x30, 0x00, 0x12, 0x31, 0xAA, 0x06, 0x2E, 0x0A, 0x12, 0x09, 0x47, 0x43, 0x54, 0x57, 0x02, 0x00, 0x00, 0x02, 0x11, 0x67, 0x39, 0xAB, 0x04, 0x00, 0x00, 0x00, 0x00, 0x12, 0x18, 0x0A, 0x16, 0x0A, 0x0A, 0x08, 0xC7, 0x86, 0xD1, 0xBA, 0x05, 0x10, 0x02, 0x18, 0x12, 0x12, 0x08, 0x32, 0x06, 0x19, 0x00, 0x00, 0x19, 0x00, 0x00, };
if(ndst >= 65) { memcpy(dst, src, 65); } else { abort(); }
return 65;}
