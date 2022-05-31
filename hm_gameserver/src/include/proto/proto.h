/*
   hm_gameserver -  hearthmod gameserver
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
#ifndef PROTO_H_
#define PROTO_H_

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>
#include <sys/time.h>
#include <time.h>

#include <ev.h>

#include <hmbase.h>

typedef unsigned long long u64;

int sizeofu64(u64 v);
int sizeofu32(int v);
int stream_write(const char **dst, const char *end, const void *src, const int nsrc);
int write_byte(char **dst, const char *end, const char src);
int write_bytes(char **dst, const char *end, const char *src, const int nsrc);
void write_uint(char **dst, const char *end, int src);
void write_uint64(char **dst, const char *end, u64 src);
int write_mem_int(char **dst, const char *end, const int src);

char read_byte(char **dst, const char *end);
int read_uint(char **dst, const char *end);
u64 read_uint64(char **dst, const char *end);
int read_mem_int(char **dst, const char *end);
char *read_bytes(char **dst, const char *end, int *ndst);

int skip(char **dst, const char *end, const int jump);

#include <ent_gen.h>
#include <flag_def.h>
#include <flags.h>

#include <zone.h>
#include <misc.h>
#include <powerhistory.h>
#include <packet.h>
#include <gametag.h>
#include <ent.h>
#include <entitychoices.h>
#include <chooseentities.h>
#include <options.h>
#include <mouseinfo.h>
#include <userui.h>
#include <turntimer.h>

extern struct hm_log_s *lg;

#endif
