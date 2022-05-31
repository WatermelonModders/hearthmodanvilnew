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

void tag_free(struct powerhistory_tag_s *t)
{
    free(t);
}

struct powerhistory_tag_s *tag_deserialize(char **dst, const char *maxdst)
{
    int n;
    struct powerhistory_tag_s *t;

    t = malloc(sizeof(*t));


    n = read_byte(dst, maxdst);

    if(n != 8) {
        error();
    }
    t->name = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);

    if(n != 16) {
        error();
    }
    t->value = read_uint64(dst, maxdst);

    return t;
}


int tag_size(struct powerhistory_tag_s *tag)
{
    int size = 0;

    if(tag) {
        size += 2;
        size += sizeofu64(tag->name);
        size += sizeofu64(tag->value);
    }

    return size;
}

int tag_serialize(struct powerhistory_tag_s *t, char **dst, const char *maxdst)
{
    write_byte(dst, maxdst, 8);
    write_uint64(dst, maxdst, t->name);

    write_byte(dst, maxdst, 16);
    write_uint64(dst, maxdst, t->value);

    return 0;
}
