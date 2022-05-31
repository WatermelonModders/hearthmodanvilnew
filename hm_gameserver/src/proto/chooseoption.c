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

void chooseoption_free(struct chooseoption_s *c)
{
    free(c);
}

void *chooseoption_deserialize(char **dst, const char *maxdst)
{
    int n;
    struct chooseoption_s *o;

    o = malloc(sizeof(*o));
    memset(o, 0, sizeof(*o));

    n = read_byte(dst, maxdst);
    if(n != 8) {
        error();
    }
    o->id = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 16) {
        error();
    }
    o->index = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 24) {
        error();
    }
    o->target = read_uint64(dst, maxdst);

    while(*dst < maxdst) {
        n = read_byte(dst, maxdst);
        if(n == 32) {
            o->suboption = read_uint64(dst, maxdst);
        } else if(n == 40) {
            o->position = read_uint64(dst, maxdst);
        } else {
            error();
        }
    }

    return o;
}

void chooseoption_dump(struct chooseoption_s *o, u64 local_held_card)
{
    hm_log(LOG_ALERT, lg, "Choose options dump: option id: %lld index: %lld target: %lld suboption: %lld position: %lld local held card: %lld", o->id, o->index, o->target, o->suboption, o->position, local_held_card);

}
