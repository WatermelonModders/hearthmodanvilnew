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
#include <proto.h>

void gamesetup_free(struct gamesetup_s *g)
{
    free(g);
}

struct gamesetup_s *gamesetup_deserialize(char **dst, const char *maxdst)
{
    abort();
    return NULL;
}

int gamesetup_serialize(void *data, char **dst, const char *maxdst)
{
    char *start;
    struct gamesetup_s *g = data;

    start = *dst;

    write_byte(dst, maxdst, 8);
    write_uint64(dst, maxdst, g->board);

    write_byte(dst, maxdst, 16);
    write_uint64(dst, maxdst, g->maxsecrets);

    write_byte(dst, maxdst, 24);
    write_uint64(dst, maxdst, g->maxfriendlyminions);


    if(g->keepalive > 0) {
        write_byte(dst, maxdst, 32);
        write_uint64(dst, maxdst, g->keepalive);
    }

    if(g->stuckdisconnect > 0) {
        write_byte(dst, maxdst, 40);
        write_uint64(dst, maxdst, g->stuckdisconnect);
    }

    return (*dst - start);
}

int gamesetup_size(struct gamesetup_s *g)
{
    int num = 0;

    num += 3;
    num += sizeofu64(g->board);
    num += sizeofu64(g->maxsecrets);
    num += sizeofu64(g->maxfriendlyminions);

    if(g->keepalive > 0) {
        num += 1;
        num += sizeofu64(g->keepalive);
    }

    if(g->stuckdisconnect > 0) {
        num += 1;
        num += sizeofu64(g->stuckdisconnect);
    }

    return num;
}
