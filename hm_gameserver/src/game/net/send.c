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

void net_send_turntimer(struct conn_client_s *c, int seconds, int turn, int show)
{
    int n;
    char output[8192];
    char *ptr = output;
    struct turntimer_s *all;
    struct packet_s *packet;

    struct hs_holder_s *p;

    p = c->data;

    all = malloc(sizeof(*all));

    all->seconds = seconds;
    all->turn = turn;
    all->show = show;

    add_packet(&packet, all, P_TURNTIMER);

    ptr = output;
    n = serialize(packet, &ptr, output + sizeof(output));

    turntimer_dump(all);

    hm_send(c, output, n);
    hm_send(p->opponent.client, output, n);
}

int net_send(struct conn_client_s *c, struct packet_s *packet)
{
    int n;
    char output[8192];
    char *ptr = output;
    n = serialize(packet, &ptr, ptr + sizeof(output));

    hm_send(c, output, n);

    return 0;
}

int net_send_options(struct conn_client_s *c, struct deck_s *deck, struct deck_s *opponent, int turn)
{
    int n;
    char output[8192];

    n = set_options(deck, opponent, output, output + sizeof(output), turn);

    hm_send(c, output, n);

    return 0;
}

int net_send_cl_options(struct conn_client_s *c, struct card_list_s *cl, int turn)
{
    int n;
    char output[8192];

    n = set_options_cards(cl, output, output + sizeof(output), turn);
    hm_send(c, output, n);

    return 0;
}
