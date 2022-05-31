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

void creategame_free(struct powerhistory_creategame_s *c)
{
    struct powerhistory_player_s *player, *del;

    for(player = c->player; player != NULL; del = player, player = player->next, player_free(del));

    free(c);
}

struct powerhistory_creategame_s *creategame_deserialize(char **dst, const char *maxdst)
{
    int n, len;
    struct powerhistory_creategame_s *t;
    struct powerhistory_player_s *player;

    t = malloc(sizeof(*t));
    t->player = t->player_tail = NULL;

    n = read_byte(dst, maxdst);

    if(n != 10) {
        error();
    }
    len = read_uint(dst, maxdst);

    t->game_entity = game_entity_deserialize(dst, *dst + len);

    while(*dst < maxdst) {
        n = read_byte(dst, maxdst);
        if(n != 18) {
            error();
        }

        len = read_uint(dst, maxdst);

        player = player_deserialize(dst, *dst + len);
        player->next = NULL;

        if(t->player == NULL && t->player_tail == NULL) {
            t->player = player;
            t->player_tail = player;
        } else {
            t->player_tail->next = player;
            t->player_tail = player;
        }
    }

    return t;
}

int creategame_size(struct powerhistory_creategame_s *cg)
{
    struct powerhistory_player_s *p;
    int num = 0;
    int n;

    if(cg) {
        num += 1;
        n = game_entity_size(cg->game_entity);
        num += sizeofu32(n) + n;

        for(p = cg->player; p != NULL; p = p->next) {
            num += 1;
            n = player_size(p);
            num += sizeofu32(n) + n;
        }
    }

    return num;
}

int creategame_serialize(struct powerhistory_creategame_s *cg, char **dst, const char *maxdst)
{
    int n;
    struct powerhistory_player_s *p;

    write_byte(dst, maxdst, 10);
    n = game_entity_size(cg->game_entity);
    write_uint(dst, maxdst, n);
    game_entity_serialize(cg->game_entity, dst, maxdst);

    for(p = cg->player; p != NULL; p = p->next) {
        write_byte(dst, maxdst, 18);
        n = player_size(p);
        write_uint(dst, maxdst, n);
        player_serialize(p, dst, maxdst);
    }

    return 0;
}
