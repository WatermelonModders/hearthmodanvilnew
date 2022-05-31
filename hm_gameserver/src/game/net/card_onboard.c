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

struct packet_s *net_card_onboard(struct hs_holder_s *p, struct chooseoption_s *opt, struct card_s *attacker, int player)
{
    //struct deck_s *deck;
    struct packet_s *packet = NULL;
    struct powerhistory_s *ph = NULL;
    struct powerhistory_data_s *data = NULL;

    assert(p);

    add_powerstart(&data, 7, 0, p->held_card, opt->target, NULL, 0);
    //add_tagchange(&data, TEMP_RESOURCES, 0, p->entity_id);
    //add_tagchange(&data, RESOURCES_USED, p->deck_copy->mana_used, p->entity_id);

    if(!flag(&attacker->state, CARD_CHARGE, FLAG_ISSET)) {
        add_tagchange(&data, EXHAUSTED, 1, p->held_card);
    }

    if(player != attacker->controller) {
        struct powerhistory_entity_s *show = NULL;
        struct powerhistory_tag_s *card_tags = default_tags(attacker, attacker->entity, 0);
        add_full_entity(&show, card_tags, attacker->id, attacker->entity->name, attacker->entity->nname);
        add_ph_data_show(&data, show);
        /*
           if(flag(&attacker->state, CARD_BATTLECRY_DRAW1_CARD, FLAG_ISSET)) {
           add_powerstart(&data, 3, -1, attacker->id, 0, NULL, 0);
           add_meta(&data, 0, 0, attacker->id);
           add_powerend(&data);
           }
           */
    }

    net_card_flags(&data, p->deck, p->deck_copy, p->opponent.deck, p->opponent.deck_copy, NULL, p, player, NET_PUTONBOARD);

    hm_log(LOG_DEBUG, lg, "board zone position %lld for card: %d", opt->position, p->held_card);

    add_powerend(&data);

    add_ph(&ph, data);
    add_packet(&packet, ph, P_POWERHISTORY);

    hm_log(LOG_DEBUG, lg, "Card onboard for player: %d", player);
    powerhistory_dump(ph);

    return packet;
}
