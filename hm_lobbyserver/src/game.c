/*
   hm_lobbyserver - HearthStone HearthMod lobbyserver
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
#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <malloc.h>
#include <ev.h>

#include <hmbase.h>

#include <client.h>

struct playerlist_s {
    struct conn_client_s *player;
    struct playerlist_s *next;
};

static struct playerlist_s *queue = NULL;
static int gamehandle = 0;

void mm_push(struct conn_client_s *c)
{
    struct playerlist_s *p;

    p = malloc(sizeof(*p));

    p->player = c;
    p->next = queue;

    queue = p;
}

struct conn_client_s *mm_pop()
{
    struct conn_client_s *c;
    struct playerlist_s *p;

    p = queue;
    c = queue->player;

    queue = queue->next;
    free(p);

    return c;
}

int matchmaking(struct conn_client_s *player1)
{
    struct conn_client_s *player2;	

    // first player
    if(queue == NULL) {
        mm_push(player1);
        return 0;
    }

    player2 = mm_pop();

    if(player1 && player2) {
        start_game(player1, player2, gamehandle++);
        return 0;
    }

    return -1;
}
