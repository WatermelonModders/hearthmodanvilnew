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

void put_card_onboard(struct conn_client_s *c, struct hs_holder_s *p, struct chooseoption_s *opt)
{
    struct packet_s *p1;
    struct packet_s *p2;
    struct card_s *attacker;

    decks_copy(p);

    ///< get attacker from copied deck
    attacker = card_get(p->deck_copy, NULL, p->held_card);
    assert(attacker);

    update_resources(p->deck_copy, attacker, 0);

    hm_log(LOG_INFO, lg, "Putting card %d on board", attacker->id);

    mechanics_put_card_onboard(c, opt->position, attacker, 0, NULL, opt, p->deck_copy);

    p1 = net_card_onboard(p, opt, attacker, p->deck_copy->controller);
    p2 = net_card_onboard(p, opt, attacker, p->opponent.deck_copy->controller);

    decks_swap(p);

    net_send(c, p1);
    net_send_options(c, p->deck, p->opponent.deck, p->game->turn);

    net_send(p->opponent.client, p2);

    packet_free(p1);
    packet_free(p2);

    /*
       int n;
       char output[8192];
       char *ptr = output;
       n = serialize(p1, &ptr, ptr + sizeof(output));

       int so = set_options(p->deck, p->opponent.deck, output + n, output + sizeof(output) - n, p->game->turn);

    //hm_log(LOG_DEBUG, lg, "sending %d bytes to initiator", n + so);
    hm_send(c, output, (int)(n + so));

    char output2[2048];
    ptr = output2;
    int n2 = serialize(p2, &ptr, ptr + sizeof(output2));

    //hm_log(LOG_DEBUG, lg, "sending %d bytes to initiator's opponent", n);
    hm_send(p->opponent.client, output2, n2);
    */
}
