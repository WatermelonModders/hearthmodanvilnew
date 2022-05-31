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
#include <game.h>

static void force_next_turn(struct ev_loop *loop, ev_timer *w, int revents)
{
    struct conn_client_s *c;
    struct hs_holder_s *p;

    c = w->data;
    p = c->data;

    //c = p->opponent.client;
    //p = c->data;

    ev_timer_stop(loop, &p->game->force_next_turn);

    next_turn(c, p);
}

static void countdown(struct ev_loop *loop, ev_timer *w, int revents)
{
    struct conn_client_s *c;
    struct hs_holder_s *p;

    c = w->data;
    p = c->data;

    ev_timer_stop(loop, &p->game->turntimer);

    net_send_turntimer(c, 15, p->game->turn, 1);
}

void register_turntimer(struct conn_client_s *c)
{
    struct hs_holder_s *p;

#ifdef TURNOFF_TIMER
    return;
#endif

    p = c->data;

    // stop first
    ev_timer_stop(c->loop, &p->game->turntimer);
    ev_timer_stop(c->loop, &p->game->force_next_turn);

    p->game->turntimer.data = c;
    p->game->force_next_turn.data = c;

    ev_timer_init(&p->game->turntimer, countdown, 30, 0);
    ev_timer_start(c->loop, &p->game->turntimer);

    ev_timer_init(&p->game->force_next_turn, force_next_turn, 45, 0);
    ev_timer_start(c->loop, &p->game->force_next_turn);
}
