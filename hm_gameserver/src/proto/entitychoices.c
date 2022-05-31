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

void entitychoices_free(struct entitychoices_s *ec)
{
    struct entitychoices_entities_s *e, *del;

    for(e = ec->entities; e != NULL; del = e, e = e->next, free(del));

    free(ec);
}

int entitychoices_serialize(void *ep, char **dst, const char *maxdst)
{
    int n = 0;
    struct entitychoices_entities_s *es;
    struct entitychoices_s *e = ep;
    char *start;

    start = *dst;

    write_byte(dst, maxdst, 8);
    write_uint64(dst, maxdst, e->id);

    write_byte(dst, maxdst, 16);
    write_uint64(dst, maxdst, e->type);

    write_byte(dst, maxdst, 32);
    write_uint64(dst, maxdst, e->countmin);

    write_byte(dst, maxdst, 40);
    write_uint64(dst, maxdst, e->countmax);

    if(e->entities) {
        write_byte(dst, maxdst, 50);
        for(es = e->entities; es != NULL; es = es->next) {
            n += sizeofu64(es->entity);
        }

        write_uint(dst, maxdst, n);

        for(es = e->entities; es != NULL; es = es->next) {
            write_uint64(dst, maxdst, es->entity);
        }
    }

    if(e->source > 0) {
        write_byte(dst, maxdst, 56);
        write_uint64(dst, maxdst, e->source);
    }

    write_byte(dst, maxdst, 64);
    write_uint64(dst, maxdst, e->player_id);

    return (*dst - start);
}

void entitychoices_dump(struct entitychoices_s *es)
{
    struct entitychoices_entities_s *ent;

    hm_log(LOG_DEBUG, lg, "Entity Choices dump:");
    hm_log(LOG_DEBUG, lg, "\t\tID: %lld", es->id);
    hm_log(LOG_DEBUG, lg, "\t\tType: %lld", es->type);
    hm_log(LOG_DEBUG, lg, "\t\tCountMin: %lld", es->countmin);
    hm_log(LOG_DEBUG, lg, "\t\tCountMax: %lld", es->countmax);
    hm_log(LOG_DEBUG, lg, "\t\tSource: %lld", es->source);
    hm_log(LOG_DEBUG, lg, "\t\tPlayer: %lld", es->player_id);

    for(ent = es->entities; ent != NULL; ent = ent->next) {
        hm_log(LOG_DEBUG, lg, "\t\t\tChild entity: %lld", ent->entity);
    }
}
