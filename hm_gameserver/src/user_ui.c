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

enum ct_e {
    C_HELD = 0,
    C_ARROWORIGIN = 1,
};

static void set_card(struct conn_client_s *c, struct userui_s *u, enum ct_e type)
{
    struct hs_holder_s *p;

    p = c->data;

    if(u && u->mouseinfo) {
        if(type == C_HELD) {
            if(u->mouseinfo->heldcard > 0) p->held_card = u->mouseinfo->heldcard;
            else if(u->mouseinfo->overcard > 0) p->held_card = u->mouseinfo->overcard;
            else p->held_card = 0;

            hm_log(LOG_INFO, lg, "Setting heldcard %d based on held %lld over %lld", p->held_card, u->mouseinfo->heldcard, u->mouseinfo->overcard);
        }
        if(type == C_ARROWORIGIN) {
            p->arroworigin = u->mouseinfo->arroworigin;
        }
    }
}

void user_ui(struct conn_client_s *c, struct userui_s *u)
{
    struct packet_s *packet = NULL;
    struct hs_holder_s *p;
    char output[1024];
    char *ptr;

    p = c->data;

    // update arroworigin
    set_card(c, u, C_ARROWORIGIN);

    // update held card
    set_card(c, u, C_HELD);

    // add player id
    u->player_id = p->id;

    add_packet(&packet, u, P_USERUI);
    ptr = output;
    int n = serialize(packet, &ptr, ptr + sizeof(output));

    userui_dump(u);

    packet_free(packet);

    // let the opponent know what player is thinking
    hm_send(p->opponent.client, output, n);
}
