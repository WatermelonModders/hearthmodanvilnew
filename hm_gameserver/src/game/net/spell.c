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

static void net_defender_attacked(struct hs_holder_s *p, struct powerhistory_data_s **data, struct card_s *attacker, struct card_s *defender, struct chooseoption_s *opt)
{
    add_meta(data, 0, 0, defender->id);
    add_tagchange(data, PREDAMAGE, attacker->attack, defender->id);
    add_tagchange(data, PREDAMAGE, 0, opt->target);
    add_meta(data, 1, attacker->attack, opt->target);

    add_tagchange(data, LAST_AFFECTED_BY, attacker->id, defender->id);
    add_tagchange(data, DAMAGE, defender->total_health - defender->health, defender->id);
    add_powerend(data);

    //add_tagchange(data, ZONE, 4, attacker->id);
    //add_tagchange(data, NUM_OPTIONS_PLAYED_THIS_TURN, 1, p->entity_id);
}

struct packet_s *net_spell(struct conn_client_s *c, struct chooseoption_s *opt, int player, struct card_s *attacker, struct card_list_s *defenders, struct card_s *fake_levelup)
{
    struct packet_s *packet = NULL;
    struct powerhistory_s *ph = NULL;
    struct powerhistory_data_s *data = NULL;
    struct hs_holder_s *p;
    struct deck_s *deck;
    int i;
    struct card_list_s *cl = NULL, *clitem, *def;
    struct card_s *defender;

    p = c->data;
    deck = p->deck_copy;

    defender = defenders->card;

    add_powerstart(&data, 7, 0, attacker->id, defender->id, NULL, 0);
    //add_tagchange(&data, RESOURCES_USED, deck->mana_used, p->entity_id);      ///< total cost
    //add_tagchange(&data, 418, 26, p->entity_id);
    //add_tagchange(&data, NUM_CARDS_PLAYED_THIS_TURN, 1, p->entity_id);
    //add_tagchange(&data, 430, 1, p->entity_id);

    //add_tagchange(&data, ZONE, 1, attacker->id);
    //add_tagchange(&data, ZONE_POSITION, 0, attacker->id);
    //add_tagchange(&data, JUST_PLAYED, 1, attacker->id);

    //add_tagchange(&data, 397, attacker->id, p->entity_id);
    add_tagchange(&data, CARD_TARGET, defender->id, attacker->id);


    if(player != attacker->controller) {
        for(i = 0; i < deck->ncards; i++) {
            if(deck->cards[i]->id == p->held_card) {
                struct powerhistory_entity_s *show = NULL;
                struct powerhistory_tag_s *card_tags = default_tags(deck->cards[i], deck->cards[i]->entity, 0);
                add_full_entity(&show, card_tags, deck->cards[i]->id, deck->cards[i]->entity->name, deck->cards[i]->entity->nname);
                add_ph_data_show(&data, show);

                break;
            }
        }
    }

    if(flag(&attacker->state, CARD_NONTARGET, FLAG_ISSET)) {
        struct deck_s *opp = p->opponent.deck_copy;
        for(i = 0; i < opp->ncards; i++) {
            if(flag(&opp->cards[i]->state, CARD_BOARD, FLAG_ISSET)) {

                add_powerstart(&data, 3, -1, attacker->id, opp->cards[i]->id, NULL, 0);
                net_defender_attacked(p, &data, attacker, opp->cards[i], opt);
                add_powerend(&data);
            }
        }
    } else {
        /*
           add_powerstart(&data, 3, -1, attacker->id, defender->id, NULL, 0);
        //net_defender_attacked(p, &data, attacker, defender, opt);

        add_meta(&data, 0, 0, defender->id);
        add_tagchange(&data, PREDAMAGE, attacker->attack, defender->id);
        add_tagchange(&data, PREDAMAGE, 0, opt->target);
        add_meta(&data, 1, attacker->attack, opt->target);

        add_tagchange(&data, LAST_AFFECTED_BY, attacker->id, defender->id);
        //add_tagchange(&data, DAMAGE, defender->total_health - defender->health, defender->id);
        add_powerend(&data);
        */
        //damage_done(&data, p, attacker, defender);

        //update_zone_position_attack(&data, p);

        // pass old deck
        LIST_ADD(cl, clitem, attacker);
        for(def = defenders; def != NULL; def = def->next) {
            LIST_ADD(cl, clitem, def->card);
        }
        net_card_flags(&data, p->deck, p->deck_copy, p->opponent.deck, p->opponent.deck_copy, NULL, p, player, NET_SPELL);

        //net_card_flags(&data, p->deck, NULL, attacker, p);
        //net_card_flags(&data, p->opponent.deck, NULL, defender, p);

        add_powerend(&data);
    }

    add_ph(&ph, data);
    add_packet(&packet, ph, P_POWERHISTORY);

    powerhistory_dump(ph);

    return packet;
}
