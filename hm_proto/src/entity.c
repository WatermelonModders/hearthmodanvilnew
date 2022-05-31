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

void entity_free(struct powerhistory_entity_s *e)
{
    struct powerhistory_tag_s *tag, *del;

    for(tag = e->tag; tag != NULL; del = tag, tag = tag->next, tag_free(del));

    //free(e->name);

    free(e);
}

struct powerhistory_entity_s *entity_deserialize(char **dst, const char *maxdst)
{
    int n, len;
    struct powerhistory_entity_s *p;
    struct powerhistory_tag_s *t;

    p = malloc(sizeof(*p));

    p->tag = p->tag_tail = NULL;

    n = read_byte(dst, maxdst);
    if(n != 8) {
        error();
    }

    p->entity = read_uint64(dst, maxdst);
    n = read_byte(dst, maxdst);
    if(n != 18) {
        error();
    }

    p->name = read_bytes(dst, maxdst, &p->nname);

    while(*dst < maxdst) {
        n = read_byte(dst, maxdst);
        len = read_uint(dst, maxdst);

        if(n == 26) {
            t = tag_deserialize(dst, *dst + len);
            t->next = NULL;

            if(p->tag == NULL && p->tag_tail == NULL) {
                p->tag = t;
                p->tag_tail = t;
            } else {
                p->tag_tail->next = t;
                p->tag_tail = t;
            }
        } else {
            error();
            return NULL;
        }
    }

    return p;
}


int entity_size(struct powerhistory_entity_s *ent)
{
    int num = 0;
    int ts;
    struct powerhistory_tag_s *tag;

    num += 2;
    num += sizeofu64(ent->entity);
    num += sizeofu32(ent->nname) + ent->nname;

    for(tag = ent->tag; tag != NULL; tag = tag->next) {
        num += 1;
        ts = tag_size(tag);
        num += sizeofu32(ts) + ts;
    }

    return num;
}

int entity_serialize(struct powerhistory_entity_s *ent, char **dst, const char *maxdst)
{
    int ts;
    struct powerhistory_tag_s *t;

    write_byte(dst, maxdst, 8);
    write_uint64(dst, maxdst, ent->entity);

    write_byte(dst, maxdst, 18);
    write_bytes(dst, maxdst, ent->name, ent->nname);

    if(ent->tag) {
        for(t = ent->tag; t != NULL; t = t->next) {
            write_byte(dst, maxdst, 26);
            ts = tag_size(t);
            write_uint(dst, maxdst, ts);
            tag_serialize(t, dst, maxdst);
        }
    }

    return 0;
}
