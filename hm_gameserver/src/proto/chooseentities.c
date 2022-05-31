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

void chooseentities_free(struct chooseentities_s *c)
{
    struct chooseentities_ent_s *e, *del;

    for(e = c->entity; e != NULL; del = e, e = e->next, free(del));

    free(c);
}

void *chooseentities_deserialize(char **dst, const char *maxdst)
{
    int n, i;
    struct chooseentities_s *c;
    struct chooseentities_ent_s *e;

    c = malloc(sizeof(*c));
    memset(c, 0, sizeof(*c));

    n = read_byte(dst, maxdst);
    if(n != 8) {
        error();
    }

    c->id = read_uint64(dst, maxdst);
    c->entity = NULL;

    n = read_byte(dst, maxdst);
    if(n != 18) {
        return c;
    }

    c->nentity = read_uint(dst, maxdst);
    if(c->nentity > 5 || c->nentity < 0) {
        free(c);
        return NULL;
    }

    for(i = 0; i < c->nentity && *dst < maxdst; i++) {
        e = malloc(sizeof(*e));
        e->entity = read_uint(dst, maxdst);
        e->next = c->entity;
        c->entity = e;
    }

    return c;
}
