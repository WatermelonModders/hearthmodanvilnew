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

struct packet_s *net_attack(struct conn_client_s *c, struct chooseoption_s *opt, struct card_s *attacker, struct card_s *defender, int target_player)
{
    struct packet_s *packet = NULL;
    struct powerhistory_s *ph = NULL;
    struct powerhistory_data_s *data = NULL;
    struct hs_holder_s *p;
    //struct card_s *attacker, *defender;
    //struct deck_s *deck;

    p = c->data;
    //deck = p->deck;
    //attacker = card_get(p->deck, NULL, p->arroworigin);
    //defender = card_get(p->opponent.deck, p->deck, opt->target);
    //assert(attacker && defender);

    //game_attack(p, attacker, defender);

    add_powerstart(&data, 1, -1, attacker->id, defender->id, NULL, 0);
    //add_tagchange(&data, 417, 1, p->entity_id == 2 ? 3 : 2);
    add_tagchange(&data, PROPOSED_ATTACKER, attacker->id, 1);
    add_tagchange(&data, PROPOSED_DEFENDER, opt->target, 1);
    add_tagchange(&data, ATTACKING, 1, p->arroworigin);
    add_tagchange(&data, NEXT_STEP, 10, 1);
    add_tagchange(&data, STEP, 11, 1);
    add_tagchange(&data, NUM_OPTIONS_PLAYED_THIS_TURN, 1, p->entity_id == 2 ? 3 : 2);

    add_tagchange(&data, DEFENDING, 1, opt->target);
    add_tagchange(&data, PREDAMAGE, defender->receive.damage, opt->target);
    add_tagchange(&data, PREDAMAGE, 0, opt->target);
    add_meta(&data, 1, defender->receive.damage, opt->target);

    add_tagchange(&data, LAST_AFFECTED_BY, p->arroworigin, opt->target);
    //add_tagchange(&data, DAMAGE, defender->total_health - defender->health, opt->target); // total damaged caused from beginning
    add_tagchange(&data, PREDAMAGE, attacker->receive.damage, p->arroworigin);
    add_tagchange(&data, PREDAMAGE, 0, p->arroworigin);

    add_meta(&data, 1, attacker->receive.damage, p->arroworigin);

    add_tagchange(&data, NUM_ATTACKS_THIS_TURN, 1, p->arroworigin);

    /*
       if(--(attacker->actions_number) == 0) {
       hm_log(LOG_DEBUG, lg, "setting card %d to EXHAUSTED", attacker->id);
       add_tagchange(&data, EXHAUSTED, 1, p->arroworigin);
       }
       */

    add_tagchange(&data, PROPOSED_ATTACKER, 0, 1);
    add_tagchange(&data, PROPOSED_DEFENDER, 0, 1);
    add_tagchange(&data, ATTACKING, 0, p->arroworigin);
    add_tagchange(&data, DEFENDING, 0, opt->target);

    // pass old deck
    //LIST_ADD(cl, clitem, attacker);
    //LIST_ADD(cl, clitem, defender);
    net_card_flags(&data, p->deck, p->deck_copy, p->opponent.deck, p->opponent.deck_copy, NULL, p, target_player, NET_ATTACK);

    add_powerend(&data);

    //add_powerstart(&data, 6, 0, 1, 0, NULL, 0);
    //add_tagchange(&data, NUM_MINIONS_PLAYER_KILLED_THIS_TURN, 1, p->entity_id);		// 368
    //add_tagchange(&data, NUM_MINIONS_KILLED_THIS_TURN, 1, 1);				// 369
    //add_tagchange(&data, 398, 1, p->entity_id);
    //add_tagchange(&data, 412, 1, p->entity_id);

    //damage_done(&data, p, attacker, defender);

    //update_zone_position_attack(&data, p);

    /*
       if(attacker->health == 0) {
       cards_board_destroyed_reorder(&data, p->deck, attacker->zone_position);

       add_tagchange(&data, EXHAUSTED, 0, attacker->id);						// 43
       add_tagchange(&data, DAMAGE, 0, attacker->id);
       add_tagchange(&data, ZONE, 4, attacker->id);								// 49
       add_tagchange(&data, ZONE_POSITION, 0, attacker->id);
       } else {
       add_tagchange(&data, DAMAGE, attacker->total_health - attacker->health, attacker->id); // total damaged caused from beginning
       card_set_exhausted(attacker);
       }

       add_tagchange(&data, ARMOR, defender->armor, defender->id);
       add_tagchange(&data, ARMOR, attacker->armor, attacker->id);

       if(defender->health == 0) {
       add_tagchange(&data, EXHAUSTED, 0, defender->id);						// 43
       add_tagchange(&data, DAMAGE, 0, defender->id);
       add_tagchange(&data, ZONE, 4, defender->id);								// 49
       add_tagchange(&data, ZONE_POSITION, 0, defender->id);

       if(EQFLAG(defender->state, CARD_HERO)) {
       add_tagchange(&data, PLAYSTATE, 3, p->entity_id == 2 ? 3 : 2);
       add_tagchange(&data, PLAYSTATE, 5, p->entity_id == 2 ? 3 : 2);
       add_tagchange(&data, PLAYSTATE, 4, p->entity_id);
       } else {
       cards_board_destroyed_reorder(&data, p->opponent.deck, defender->zone_position);
       }
       }
       */

    /** destroy both */
    /*
       add_tagchange(&data, ZONE, 4, attacker->id);								// 49
       add_tagchange(&data, ZONE_POSITION, 0, attacker->id);
       add_tagchange(&data, ZONE, 4, defender->id);								// 49
       add_tagchange(&data, ZONE_POSITION, 0, defender->id);
       attacker->state = CARD_DESTROYED;
       defender->state = CARD_DESTROYED;
       */

    //add_powerend(&data);

    //add_tagchange(&data, STEP, 10, 1);
    //add_powerstart(&data, 5, 2, p->entity_id, 0, NULL, 0);

    /*
       if(flag(&attacker->state, CARD_HERO, FLAG_ISSET) && p->deck->heroweapon) {

       p->deck->heroweapon->durability--;

       add_meta(&data, 1, p->deck->heroweapon->attack, p->deck->heroweapon->id);
       add_tagchange(&data, LAST_AFFECTED_BY, p->deck->hero->id, p->deck->heroweapon->id);

       add_tagchange(&data, DAMAGE, p->deck->heroweapon->maxdurability - p->deck->heroweapon->durability, p->deck->heroweapon->id);
       add_tagchange(&data, EXHAUSTED, 1, p->deck->heroweapon->id);

       if(p->deck->heroweapon->durability == 0) {
       add_tagchange(&data, ZONE, 4, p->deck->heroweapon->id);
       add_tagchange(&data, ATK, 0, attacker->id);

    //p->deck->heroweapon->state |= CARD_DESTROYED;
    flag(&p->deck->heroweapon->state, CARD_DESTROYED, FLAG_SET);
    p->deck->hero->attack = 0;

    hm_log(LOG_DEBUG, lg, "heroweapon destroyed %d", p->deck->heroweapon->id);

    p->deck->heroweapon = NULL;
    }

    }
    */

    //add_tagchange(&data, NEXT_STEP, 12, 1);
    //add_powerend(&data);

    add_ph(&ph, data);
    add_packet(&packet, ph, P_POWERHISTORY);

    powerhistory_dump(ph);


    return packet;
}
