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

struct packet_s *net_spell_hp_damage(struct conn_client_s *c, struct card_list_s *defenders, struct card_s *attacker)
{
    struct packet_s *packet = NULL;
    struct powerhistory_s *ph = NULL;
    struct powerhistory_data_s *data = NULL;
    struct hs_holder_s *p;
    struct card_list_s *cl = NULL, *clitem, *def;
    struct card_s *defender;

    p = c->data;
    defender = defenders->card;

    add_powerstart(&data, 7, 0, attacker->id, defender->id, NULL, 0);
    //add_tagchange(&data, RESOURCES_USED, deck->mana_used, p->entity_id);
    //add_tagchange(&data, 418, 36, p->entity_id);
    /*
       add_tagchange(&data, CARD_TARGET, defender->id, attacker->id);

       add_powerstart(&data, 3, -1, attacker->id, defender->id, NULL, 0);
       add_meta(&data, 0, 0, defender->id);
       add_tagchange(&data, PREDAMAGE, attacker->attack, defender->id);
       add_tagchange(&data, PREDAMAGE, 0, defender->id);

       add_meta(&data, 1, attacker->attack, defender->id);
       add_tagchange(&data, LAST_AFFECTED_BY, attacker->id, defender->id);
       */
    // pass old deck
    LIST_ADD(cl, clitem, attacker);
    for(def = defenders; def != NULL; def = def->next) {
        LIST_ADD(cl, clitem, def->card);
    }
    net_card_flags(&data, p->deck, p->deck_copy, p->opponent.deck, p->opponent.deck_copy, cl, p, p->id, NET_SPELLHP_DAMAGE);

    add_powerend(&data);

    //add_tagchange(&data, 406, 1, p->entity_id);

    add_powerend(&data);

    add_ph(&ph, data);
    add_packet(&packet, ph, P_POWERHISTORY);

    powerhistory_dump(ph);

    return packet;
}
