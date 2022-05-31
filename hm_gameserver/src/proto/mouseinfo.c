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

void mouseinfo_free(struct mouseinfo_s *m)
{
    free(m);
}

void *mouseinfo_deserialize(char **dst, const char *maxdst)
{
    int n;
    struct mouseinfo_s *c;

    c = malloc(sizeof(*c));
    memset(c, 0, sizeof(*c));

    n = read_byte(dst, maxdst);
    if(n != 8) {
        error();
    }
    c->arroworigin = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 16) {
        error();
    }
    c->heldcard = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 24) {
        error();
    }
    c->overcard = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 32) {
        error();
    }
    c->x = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 40) {
        error();
    }
    c->y = read_uint64(dst, maxdst);

    return c;
}

int mouseinfo_serialize(void *ao, char **dst, const char *maxdst)
{
    struct mouseinfo_s *s;
    char *start;

    start = *dst;
    s = ao;

    write_byte(dst, maxdst, 8);
    write_uint64(dst, maxdst, s->arroworigin);

    write_byte(dst, maxdst, 16);
    write_uint64(dst, maxdst, s->heldcard);

    write_byte(dst, maxdst, 24);
    write_uint64(dst, maxdst, s->overcard);

    write_byte(dst, maxdst, 32);
    write_uint64(dst, maxdst, s->x);

    write_byte(dst, maxdst, 40);
    write_uint64(dst, maxdst, s->y);

    return (*dst - start);
}

int mouseinfo_size(struct mouseinfo_s *p)
{
    int num = 0;

    num += 5;
    num += sizeofu64(p->arroworigin);
    num += sizeofu64(p->heldcard);
    num += sizeofu64(p->overcard);
    num += sizeofu64(p->x);
    num += sizeofu64(p->y);

    return num;
}
