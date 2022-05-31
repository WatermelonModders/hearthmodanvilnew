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

void tagchange_free(struct powerhistory_tagchange_s *t)
{
    free(t);
}

struct powerhistory_tagchange_s *tagchange_deserialize(char **dst, const char *maxdst)
{
    int n;
    struct powerhistory_tagchange_s *t;

    t = malloc(sizeof(*t));

    n = read_byte(dst, maxdst);
    if(n != 8) {
        error();
    }
    t->entity = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 16) {
        error();
    }
    t->tag = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 24) {
        error();
    }
    t->value = read_uint64(dst, maxdst);

    return t;
}

int tagchange_serialize(struct powerhistory_tagchange_s *ent, char **dst, const char *maxdst)
{
    write_byte(dst, maxdst, 8);
    write_uint64(dst, maxdst, ent->entity);

    write_byte(dst, maxdst, 16);
    write_uint64(dst, maxdst, ent->tag);

    write_byte(dst, maxdst, 24);
    write_uint64(dst, maxdst, ent->value);

    return 0;
}

int tagchange_size(struct powerhistory_tagchange_s *t)
{
    int size = 0;

    if(t) {
        size += 3;
        size += sizeofu64(t->entity);
        size += sizeofu64(t->tag);
        size += sizeofu64(t->value);
    }

    return size;
}
