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

void next_turn(struct conn_client_s *c, struct hs_holder_s *p/*, struct chooseoption_s *opt*/)
{
    struct packet_s *p1;
    struct packet_s *p2;
    struct card_s *drawn_card = NULL;
    int zone = 0;

    decks_copy(p);

    // refresh resources
    update_resources(p->opponent.deck_copy, NULL, 1);

    mechanics_next_turn(c, p);

    hm_log(LOG_INFO, c->log, "Next turn: %d", p->game->turn);
    p1 = net_next_turn(c, &drawn_card, &zone);
    p2 = net_next_turn(c, &drawn_card, &zone);

    decks_swap(p);

    net_send(c, p1);

    net_send(p->opponent.client, p2);
    net_send_options(p->opponent.client, p->opponent.deck, p->deck, p->game->turn);

    packet_free(p1);
    packet_free(p2);

    /*

       ptr = output;
       n = serialize(p1, &ptr, ptr + sizeof(output));
       hm_send(c, output, n);

       ptr = output;
       n = serialize(p2, &ptr, ptr + sizeof(output));

       int so = set_options(p->opponent.deck, p->deck, output + n, output + sizeof(output) - n, p->game->turn);

    //memcpy(output + n + so, opt1, sizeof(opt1));

    hm_send(p->opponent.client, output, n + so);
    */
}
