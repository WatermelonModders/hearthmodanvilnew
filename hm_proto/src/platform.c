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

void platform_free(struct platform_s *p)
{
    free(p->name);

    free(p);
}

int platform_size(struct platform_s *p)
{
    int num = 0;

    num += 2;
    num += sizeofu64(p->os);
    num += sizeofu64(p->screen);

    num += 1;
    num += sizeofu32(p->nname) + p->nname;

    if(p->store != 0) {
        num += 1;
        num += sizeofu64(p->store);
    }

    return num;
}

int platform_serialize(void *ao, char **dst, const char *maxdst)
{
    struct platform_s *s;
    char *start;

    start = *dst;
    s = ao;

    write_byte(dst, maxdst, 8);
    write_uint64(dst, maxdst, s->os);

    write_byte(dst, maxdst, 16);
    write_uint64(dst, maxdst, s->screen);

    write_byte(dst, maxdst, 26);
    write_bytes(dst, maxdst, s->name, s->nname);

    if(s->store != 0) {
        write_byte(dst, maxdst, 32);
        write_uint64(dst, maxdst, s->store);
    }

    return (*dst - start);
}

struct platform_s *platform_deserialize(char **dst, const char *maxdst)
{
    int n;
    struct platform_s *h;

    h = malloc(sizeof(*h));
    h->store = 0;

    n = read_byte(dst, maxdst);
    if(n != 8) {
        error();
    }
    h->os = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 16) {
        error();
    }
    h->screen = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 26) {
        error();
    }
    h->name = read_bytes(dst, maxdst, &h->nname);

    if(*dst < maxdst) {
        n = read_byte(dst, maxdst);
        if(n == 32) {
            h->store = read_uint64(dst, maxdst);
        }
    }

    return h;
}
