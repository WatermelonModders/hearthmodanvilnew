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
