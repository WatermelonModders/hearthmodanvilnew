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

struct packet_s *net_spell_aoe(struct conn_client_s *c, struct chooseoption_s *opt, int player, struct card_s *attacker)
{
    struct packet_s *packet = NULL;
    struct powerhistory_s *ph = NULL;
    struct powerhistory_data_s *data = NULL;
    struct hs_holder_s *p;
    struct card_list_s *def;
    struct card_list_s *cl = NULL, *clitem;
    struct card_list_s *defenders;

    p = c->data;

    defenders = attacker->target.card;

    LIST_ADD(cl, clitem, attacker);

    for(def = defenders; def != NULL; def = def->next) {
        LIST_ADD(cl, clitem, def->card);
    }

    add_powerstart(&data, 7, 0, attacker->id, 0, NULL, 0);

    if(flag(&attacker->state, CARD_TRACKING, FLAG_ISSET)) {

        if(player == 2) {

            add_tagchange(&data, 418, 41, p->entity_id);
            add_tagchange(&data, 269, 1, p->entity_id);
            add_tagchange(&data, 430, 1, p->entity_id);

            struct powerhistory_entity_s *show = NULL;
            struct powerhistory_tag_s *card_tags = default_tags(attacker, attacker->entity, 1);
            add_full_entity(&show, card_tags, attacker->id, attacker->entity->name, attacker->entity->nname);
            add_ph_data_show(&data, show);

            add_tagchange(&data, 397, attacker->id, p->entity_id);
        }

        net_card_flags(&data, p->deck, p->deck_copy, p->opponent.deck, p->opponent.deck_copy, NULL, p, player, NET_SPELLAOE);

    } else {
        /*
           add_tagchange(&data, RESOURCES_USED, deck->mana_used, p->entity_id);      ///< total cost
           add_tagchange(&data, 418, 26, p->entity_id);
           add_tagchange(&data, NUM_CARDS_PLAYED_THIS_TURN, 1, p->entity_id);
           add_tagchange(&data, 430, 1, p->entity_id);

        //add_tagchange(&data, ZONE, 1, attacker->id);
        //add_tagchange(&data, ZONE_POSITION, 0, attacker->id);
        //add_tagchange(&data, JUST_PLAYED, 1, attacker->id);

        //add_tagchange(&data, 397, attacker->id, p->entity_id);

        //cards_reorder(deck, &data, attacker, opt, player);
        */
        if(player != attacker->controller) {
            hm_log(LOG_DEBUG, lg, "Spell aoe attacker [%s](%d) showing full entity player %d controller %d", attacker->entity->desc, attacker->id, player, attacker->controller);
            struct powerhistory_entity_s *show = NULL;
            struct powerhistory_tag_s *card_tags = default_tags(attacker, attacker->entity, 0);
            add_full_entity(&show, card_tags, attacker->id, attacker->entity->name, attacker->entity->nname);
            add_ph_data_show(&data, show);
        }

        // draw cards
        /*
           if(player == 1 && defenders != NULL) {
           add_powerstart(&data, 3, -1, attacker->id, 0, NULL, 0);
           add_meta(&data, 0, 0, attacker->id);
           }
           */

        net_card_flags(&data, p->deck, p->deck_copy, p->opponent.deck, p->opponent.deck_copy, NULL, p, player, NET_SPELLAOE);

        // draw cards
        /*
           if(player == 1 && defenders != NULL) {
           add_powerend(&data);
           }
           */

        add_powerend(&data);
    }

    add_ph(&ph, data);
    add_packet(&packet, ph, P_POWERHISTORY);

    powerhistory_dump(ph);

    return packet;
}
