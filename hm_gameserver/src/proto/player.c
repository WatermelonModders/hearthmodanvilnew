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

void player_free(struct powerhistory_player_s *p)
{
    game_entity_free(p->entity);

    free(p);
}

struct powerhistory_player_s *player_deserialize(char **dst, const char *maxdst)
{
    int n, len;
    struct powerhistory_player_s *player;

    player = malloc(sizeof(*player));

    n = read_byte(dst, maxdst);
    if(n != 8) {
        error();
    }
    player->id = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 18) {
        error();
    }
    len = read_uint(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 8) {
        error();
    }
    player->bnet_hi = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 16) {
        error();
    }
    player->bnet_lo = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 24) {
        error();
    }
    player->cardback = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 34) {
        error();
    }
    len = read_uint(dst, maxdst);
    player->entity = game_entity_deserialize(dst, *dst + len);

    return player;
}

int player_serialize(struct powerhistory_player_s *player, char **dst, const char *maxdst)
{
    int n;

    write_byte(dst, maxdst, 8);
    write_uint64(dst, maxdst, player->id);

    // bnet
    write_byte(dst, maxdst, 18);
    n = bnet_size(player);
    write_uint(dst, maxdst, n);
    write_byte(dst, maxdst, 8);
    write_uint64(dst, maxdst, player->bnet_hi);
    write_byte(dst, maxdst, 16);
    write_uint64(dst, maxdst, player->bnet_lo);

    write_byte(dst, maxdst, 24);
    write_uint64(dst, maxdst, player->cardback);

    write_byte(dst, maxdst, 34);
    n = game_entity_size(player->entity);
    write_uint(dst, maxdst, n);
    game_entity_serialize(player->entity, dst, maxdst);

    return 0;
}

int player_size(struct powerhistory_player_s *p)
{
    int size = 0, ts;

    if(p) {
        size += 4;
        size += sizeofu64(p->id);
        ts = bnet_size(p);
        size += sizeofu32(ts) + ts;
        size += sizeofu64(p->cardback);
        ts = game_entity_size(p->entity);
        size += sizeofu32(ts) + ts;
    }

    return size;
}
