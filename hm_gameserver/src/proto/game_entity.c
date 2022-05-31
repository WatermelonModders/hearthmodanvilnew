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

void game_entity_free(struct powerhistory_game_entity_s *g)
{
    struct powerhistory_tag_s *tag, *del;

    for(tag = g->tag; tag != NULL; del = tag, tag = tag->next, tag_free(del));

    free(g);
}

struct powerhistory_game_entity_s *game_entity_deserialize(char **dst, const char *maxdst)
{
    int n, len;
    struct powerhistory_game_entity_s *t;
    struct powerhistory_tag_s *tag;

    t = malloc(sizeof(*t));
    memset(t, 0, sizeof(*t));

    n = read_byte(dst, maxdst);
    if(n != 8) {
        error();
    }

    t->id = read_uint64(dst, maxdst);

    while(*dst < maxdst) {

        n = read_byte(dst, maxdst);
        if(n != 18) {
            error();
        }

        len = read_uint(dst, maxdst);

        tag = tag_deserialize(dst, *dst + len);
        tag->next = NULL;

        if(t->tag == NULL && t->tag_tail == NULL) {
            t->tag = tag;
            t->tag_tail = tag;
        } else {
            t->tag_tail->next = tag;
            t->tag_tail = tag;
        }
    }

    return t;
}

int game_entity_serialize(struct powerhistory_game_entity_s *ent, char **dst, const char *maxdst)
{
    int ts;
    struct powerhistory_tag_s *t;

    write_byte(dst, maxdst, 8);
    write_uint64(dst, maxdst, ent->id);

    if(ent->tag) {
        for(t = ent->tag; t != NULL; t = t->next) {
            write_byte(dst, maxdst, 18);
            ts = tag_size(t);
            write_uint(dst, maxdst, ts);
            tag_serialize(t, dst, maxdst);
        }
    }

    return 0;
}

int game_entity_size(struct powerhistory_game_entity_s *ent)
{
    int ts;
    struct powerhistory_tag_s *tag;
    int num = 0;

    num += 1;
    num += sizeofu64(ent->id);

    for(tag = ent->tag; tag != NULL; tag = tag->next) {
        num += 1;
        ts = tag_size(tag);
        num += sizeofu32(ts) + ts;
    }

    return num;
}
