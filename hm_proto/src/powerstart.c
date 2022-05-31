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

void powerstart_free(struct powerhistory_powerstart_s *p)
{
    free(p->card_id);

    free(p);
}

struct powerhistory_powerstart_s *powerstart_deserialize(char **dst, const char *maxdst)
{
    int n;
    struct powerhistory_powerstart_s *t;

    t = malloc(sizeof(*t));

    n = read_byte(dst, maxdst);
    if(n != 8) {
        error();
    }
    t->type = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 16) {
        error();
    }
    t->index = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 24) {
        error();
    }
    t->source = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 32) {
        error();
    }
    t->target = read_uint64(dst, maxdst);

    t->ncard_id = 0;
    t->card_id = NULL;
    // optional
    n = read_byte(dst, maxdst);
    if(n != 42) {
        return t;
    }

    t->card_id = read_bytes(dst, maxdst, &t->ncard_id);

    return t;
}

int powerstart_serialize(struct powerhistory_powerstart_s *p, char **dst, const char *maxdst)
{
    write_byte(dst, maxdst, 8);
    write_uint64(dst, maxdst, p->type);

    write_byte(dst, maxdst, 16);
    write_uint64(dst, maxdst, p->index);

    write_byte(dst, maxdst, 24);
    write_uint64(dst, maxdst, p->source);

    write_byte(dst, maxdst, 32);
    write_uint64(dst, maxdst, p->target);

    if(p->ncard_id > 0) {
        write_byte(dst, maxdst, 42);
        write_bytes(dst, maxdst, p->card_id, p->ncard_id);
    }

    return 0;
}

int powerstart_size(struct powerhistory_powerstart_s *p)
{
    int size = 0;
    int n;

    if(p) {
        size += 4;
        size += sizeofu64(p->type);
        size += sizeofu64(p->index);
        size += sizeofu64(p->source);
        size += sizeofu64(p->target);
        if(p->ncard_id > 0) {
            size += 1;
            n = sizeofu64(p->ncard_id);
            size += sizeofu32(n) + n;
        }
    }

    return size;
}
