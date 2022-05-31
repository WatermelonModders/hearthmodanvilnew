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
#ifndef MISC_H_
#define MISC_H_

struct gamesetup_s {
    u64 board;
    u64 maxsecrets;
    u64 maxfriendlyminions;
    u64 keepalive;
    u64 stuckdisconnect;
};

struct handshake_s {
    u64 gamehandle;
    char *password;
    int npassword;
    u64 clienthandle;
    u64 mission;
    char *version;
    int nversion;

    struct platform_s *platform;
};

struct platform_s {
    u64 os;
    u64 screen;
    int nname;
    char *name;
    u64 store;
};

struct chooseoption_s {
    u64 id;
    u64 index;
    u64 target;
    u64 suboption;
    u64 position;
};

void gamesetup_free(struct gamesetup_s *g);
int gamesetup_serialize(void *data, char **dst, const char *maxdst);

void handshake_free(struct handshake_s *h);
void *handshake_deserialize(char **dst, const char *maxdst);

void platform_free(struct platform_s *p);
struct platform_s *platform_deserialize(char **dst, const char *maxdst);

void chooseoption_free(struct chooseoption_s *c);
void *chooseoption_deserialize(char **dst, const char *maxdst);
void chooseoption_dump(struct chooseoption_s *o, u64 local_held_card);

#endif
