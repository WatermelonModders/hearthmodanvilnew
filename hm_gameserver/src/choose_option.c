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

enum metatype_e {
    METATYPE_DAMAGE = 1,
    METATYPE_HEAL = 2,
};

void cards_reorder(struct hs_holder_s *p, struct deck_s *deck, struct card_s *attacker, u64 position, int summoned)
{
    if(summoned == 0) {
        cards_reorder_hand_owner(deck, attacker->id);
    }

    // spell card is never placed on board
    if(!flag(&attacker->state, CARD_SPELL, FLAG_ISSET)) {
        cards_reorder_board_owner(deck, attacker->id, position);
    }
}

struct packet_s *choose_option_player1(struct conn_client_s *c, struct chooseoption_s *opt, int player, struct card_s *attacker)
{
    struct packet_s *packet = NULL;
    struct powerhistory_s *ph = NULL;
    struct powerhistory_data_s *data = NULL;
    struct hs_holder_s *p;
    //struct deck_s *deck;

    p = c->data;
    //deck = p->deck;

    add_powerstart(&data, 7, 0, p->held_card, opt->target, NULL, 0);
    add_tagchange(&data, TEMP_RESOURCES, 0, p->entity_id);
    add_tagchange(&data, RESOURCES_USED, p->deck->mana_used, p->entity_id);
    add_tagchange(&data, 418, 3, p->entity_id);
    add_tagchange(&data, NUM_CARDS_PLAYED_THIS_TURN, 1, p->entity_id);
    add_tagchange(&data, NUM_MINIONS_PLAYED_THIS_TURN, 1, p->entity_id);

    //cards_reorder(deck, &data, attacker, opt, player);
    /*
    // set new zone positions for owner
    if(player == 1) {
    cards_reorder_hand_owner(p->deck, p->held_card);
    cards_reorder_board_owner(&data, p->deck, p->held_card, opt->position);

    for(i = 0; i < deck->ncards; i++) {
    if(EQFLAG(deck->cards[i]->state, CARD_HAND)) {
    hm_log(LOG_DEBUG, lg, "new position %d for card: %d", deck->cards[i]->zone_position, deck->cards[i]->id);
    add_tagchange(&data, ZONE_POSITION, deck->cards[i]->zone_position, deck->cards[i]->id);
    }
    }
    }

    if(player == 2) {
    cards_reorder_hand_opponent(&data, p->deck);
    cards_reorder_board_opponent(&data, p->deck);

    for(i = 0; i < deck->ncards; i++) {
    if(deck->cards[i]->id == p->held_card) {
    struct powerhistory_entity_s *show = NULL;
    struct powerhistory_tag_s *card_tags = default_tags(deck->cards[i]->entity);
    add_full_entity(&show, card_tags, deck->cards[i]->id, deck->cards[i]->entity->name, deck->cards[i]->entity->nname);
    add_ph_data_show(&data, show);

    break;
    }
    }
    }
    */

    add_tagchange(&data, ZONE, 1, p->held_card);

    add_tagchange(&data, JUST_PLAYED, 1, p->held_card);
    add_tagchange(&data, ZONE_POSITION, opt->position, p->held_card);
    add_tagchange(&data, 397, p->held_card, p->entity_id);

    hm_log(LOG_DEBUG, lg, "board zone position %lld for card: %d", opt->position, p->held_card);

    add_powerstart(&data, 3, -1, p->held_card, opt->target, NULL, 0);
    add_powerend(&data);

    add_tagchange(&data, NUM_OPTIONS_PLAYED_THIS_TURN, 1, p->entity_id);
    add_powerend(&data);

    add_ph(&ph, data);
    add_packet(&packet, ph, P_POWERHISTORY);

    //powerhistory_dump(ph);

    return packet;
}

struct packet_s *nontarget(struct conn_client_s *c, struct card_s *attacker)
{
    struct packet_s *packet = NULL;
    struct powerhistory_s *ph = NULL;
    struct powerhistory_data_s *data = NULL;
    struct hs_holder_s *p;

    p = c->data;

    add_powerstart(&data, 7, 0, attacker->id, 0, NULL, 0);
    add_tagchange(&data, TEMP_RESOURCES, 0, p->entity_id);
    add_tagchange(&data, RESOURCES_USED, 1, p->entity_id);
    add_tagchange(&data, 418, 2, p->entity_id);

    add_powerstart(&data, 3, -1, attacker->id, 0, NULL, 0);
    struct powerhistory_entity_s *full = NULL;
    struct card_s *card = NULL;

    if(flag(&attacker->state, CARD_HP_REINFORCE, FLAG_ISSET)) {
        card = add_new_card(p->game, p->deck, -1, "CS2_101t", -1);
        struct powerhistory_tag_s *card_tags = default_tags(card, card->entity, 0);
        int zone_position = zone_position_last(p->deck);
        add_tag(&card_tags, ZONE_POSITION, zone_position);
        add_tag(&card_tags, CONTROLLER, p->id);
        add_tag(&card_tags, ENTITY_ID, card->id);
        add_tag(&card_tags, CREATOR, attacker->id);
        add_full_entity(&full, card_tags, card->id, card->entity->name, card->entity->nname);
        add_ph_data_full(&data, full);

        card->zone_position = zone_position;
        //card->state |= (CARD_BOARD|CARD_EXHAUSTED);
        flag(&card->state, CARD_BOARD, FLAG_SET);
        flag(&card->state, CARD_EXHAUSTED, FLAG_SET);

    } else if(flag(&attacker->state, CARD_HP_DAGGERMASTERY, FLAG_ISSET)) {
        card = add_new_card(p->game, p->deck, -1, "CS2_082", -1);
        struct powerhistory_tag_s *card_tags = default_tags(card, card->entity, 0);
        add_tag(&card_tags, CONTROLLER, p->id);
        add_tag(&card_tags, ENTITY_ID, card->id);
        add_tag(&card_tags, CREATOR, attacker->id);
        add_full_entity(&full, card_tags, card->id, card->entity->name, card->entity->nname);
        add_ph_data_full(&data, full);

        add_tagchange(&data, 334, attacker->id, p->entity_id);

        p->deck->heroweapon = card;
        //card->state |= CARD_EXHAUSTED;
        flag(&card->state, CARD_EXHAUSTED, FLAG_SET);
    } else if(flag(&attacker->state, CARD_HP_SHAPESHIFT, FLAG_ISSET)) {
        p->deck->hero->armor += attacker->attack;
        p->deck->hero->attack = attacker->attack;

        add_tagchange(&data, ARMOR, p->deck->hero->armor, p->deck->hero->id);
        add_tagchange(&data, ATK, attacker->attack, p->deck->hero->id);
    } else if(flag(&attacker->state, CARD_HP_ARMORUP, FLAG_ISSET)) {
        p->deck->hero->armor += attacker->attack;
        add_tagchange(&data, ARMOR, p->deck->hero->armor, p->deck->hero->id);
    } else if(flag(&attacker->state, CARD_HP_LIFETAP, FLAG_ISSET)) {
        // FIXME: don't let enemy know what card we have
        struct powerhistory_entity_s *show = NULL;
        struct card_s *drawn_card = draw_random_card(p->deck, p->entity_id, -1);
        struct powerhistory_tag_s *card_tags = default_tags(drawn_card, drawn_card->entity, 0);

        p->deck->hero->health -= attacker->attack;

        add_meta(&data, 1, attacker->attack, p->deck->hero->id);
        add_tagchange(&data, LAST_AFFECTED_BY, attacker->id, p->deck->hero->id);
        add_tagchange(&data, DAMAGE, p->deck->hero->total_health - p->deck->hero->health, p->deck->hero->id);

        hm_log(LOG_DEBUG, lg, "drawn card zone position %d for card: %d", drawn_card->zone_position, drawn_card->id);
        add_full_entity(&show, card_tags, drawn_card->id, drawn_card->entity->name, drawn_card->entity->nname);

        add_tagchange(&data, ZONE_POSITION, drawn_card->zone_position, drawn_card->id);
        add_ph_data_show(&data, show);
    }

    add_powerend(&data);

    add_tagchange(&data, 406, 1, p->entity_id);
    add_tagchange(&data, EXHAUSTED, 1, attacker->id);

    if(flag(&attacker->state, CARD_HP_DAGGERMASTERY, FLAG_ISSET)) {
        add_tagchange(&data, ATK, card->attack, p->deck->hero->id);
        p->deck->hero->attack = card->attack;
    }

    add_powerstart(&data, 5, -1, p->entity_id, 0, NULL, 0);

    add_tagchange(&data, 394, 1, p->entity_id);

    add_powerend(&data);

    add_tagchange(&data, 358, 2, p->entity_id);

    add_powerend(&data);

    add_ph(&ph, data);
    add_packet(&packet, ph, P_POWERHISTORY);

    //card_set_exhausted(attacker);

    powerhistory_dump(ph);

    return packet;
}

struct packet_s *spell_hp_damage(struct conn_client_s *c, struct card_s *defender, struct card_s *attacker)
{

    /*
       struct packet_s *packet = NULL;
       struct powerhistory_s *ph = NULL;
       struct powerhistory_data_s *data = NULL;
       struct hs_holder_s *p;

       p = c->data;

       game_attack(p, attacker, defender);

       add_powerstart(&data, 7, 0, attacker->id, defender->id, NULL, 0);
       add_tagchange(&data, RESOURCES_USED, p->deck->mana_used, p->entity_id);
       add_tagchange(&data, 418, 36, p->entity_id);
       add_tagchange(&data, CARD_TARGET, defender->id, attacker->id);

       add_powerstart(&data, 3, -1, attacker->id, defender->id, NULL, 0);
       add_meta(&data, 0, 0, defender->id);
       add_tagchange(&data, PREDAMAGE, attacker->attack, defender->id);
       add_tagchange(&data, PREDAMAGE, 0, defender->id);

       add_meta(&data, METATYPE_DAMAGE, attacker->attack, defender->id);
       add_tagchange(&data, LAST_AFFECTED_BY, attacker->id, defender->id);
       add_tagchange(&data, DAMAGE, defender->total_health - defender->health, defender->id);
       add_powerend(&data);

       add_tagchange(&data, 406, 1, p->entity_id);
       add_tagchange(&data, EXHAUSTED, 1, attacker->id);

       check_remove_divine_shield(&data, defender);

       if(defender->health == 0) {
       add_tagchange(&data, EXHAUSTED, 0, defender->id);						// 43
       add_tagchange(&data, DAMAGE, 0, defender->id);
       add_tagchange(&data, ZONE, 4, defender->id);								// 49
       add_tagchange(&data, ZONE_POSITION, 0, defender->id);

       if(flag(&defender->state, CARD_HERO, FLAG_ISSET)) {
       add_tagchange(&data, PLAYSTATE, 3, p->entity_id == 2 ? 3 : 2);
       add_tagchange(&data, PLAYSTATE, 5, p->entity_id == 2 ? 3 : 2);
       add_tagchange(&data, PLAYSTATE, 4, p->entity_id);
       } else {
       cards_board_destroyed_reorder(&data, p->opponent.deck, defender->zone_position);
       }
       }

       add_powerend(&data);

       add_ph(&ph, data);
       add_packet(&packet, ph, P_POWERHISTORY);

       powerhistory_dump(ph);

       card_set_exhausted(attacker);

       return packet;
       */
    return NULL;
}

struct packet_s *heal(struct conn_client_s *c, struct card_s *defender, struct card_s *attacker)
{
    struct packet_s *packet = NULL;
    struct powerhistory_s *ph = NULL;
    struct powerhistory_data_s *data = NULL;
    struct hs_holder_s *p;

    if(defender->total_health - defender->health > attacker->attack) {
        hm_log(LOG_DEBUG, lg, "card: %d healed for %d", defender->id, attacker->attack);
        defender->health += attacker->attack;
    } else {
        hm_log(LOG_DEBUG, lg, "card: %d hp fully restored", defender->id);
        defender->health = defender->total_health;
    }

    p = c->data;

    add_powerstart(&data, 7, 0, attacker->id, defender->id, NULL, 0);
    add_tagchange(&data, RESOURCES_USED, p->deck->mana_used, p->entity_id);
    add_tagchange(&data, CARD_TARGET, defender->id, attacker->id);

    add_powerstart(&data, 3, -1, attacker->id, defender->id, NULL, 0);
    add_meta(&data, 0, 0, defender->id);

    add_tagchange(&data, 425, attacker->attack, defender->id);
    add_tagchange(&data, 425, 0, defender->id);

    add_meta(&data, 0, attacker->attack, defender->id);
    add_tagchange(&data, DAMAGE, defender->total_health - defender->health, defender->id);

    add_powerend(&data);

    add_tagchange(&data, 406, 1, defender->id);
    add_tagchange(&data, EXHAUSTED, 1, attacker->id);

    add_powerend(&data);

    add_ph(&ph, data);
    add_packet(&packet, ph, P_POWERHISTORY);

    powerhistory_dump(ph);

    //card_set_exhausted(attacker);

    return packet;
}

struct packet_s *attack(struct conn_client_s *c, struct chooseoption_s *opt)
{
    struct packet_s *packet = NULL;
    struct powerhistory_s *ph = NULL;
    struct powerhistory_data_s *data = NULL;
    struct hs_holder_s *p;
    struct card_s *attacker, *defender;
    //struct deck_s *deck;

    p = c->data;
    //deck = p->deck;

    attacker = card_get(p->deck, NULL, p->arroworigin);
    defender = card_get(p->opponent.deck, p->deck, opt->target);

    assert(attacker && defender);

    //game_attack(p, attacker, defender);

    add_powerstart(&data, 1, -1, attacker->id, defender->id, NULL, 0);
    add_tagchange(&data, 417, 1, p->entity_id == 2 ? 3 : 2);
    add_tagchange(&data, PROPOSED_ATTACKER, attacker->id, 1);
    add_tagchange(&data, PROPOSED_DEFENDER, opt->target, 1);
    add_tagchange(&data, ATTACKING, 1, p->arroworigin);
    add_tagchange(&data, NEXT_STEP, 10, 1);
    add_tagchange(&data, STEP, 11, 1);
    add_tagchange(&data, NUM_OPTIONS_PLAYED_THIS_TURN, 1, p->entity_id == 2 ? 3 : 2);

    add_tagchange(&data, DEFENDING, 1, opt->target);
    add_tagchange(&data, PREDAMAGE, attacker->attack, opt->target);
    add_tagchange(&data, PREDAMAGE, 0, opt->target);
    add_meta(&data, 1, attacker->attack, opt->target);

    add_tagchange(&data, LAST_AFFECTED_BY, p->arroworigin, opt->target);
    add_tagchange(&data, DAMAGE, defender->total_health - defender->health, opt->target); // total damaged caused from beginning
    add_tagchange(&data, PREDAMAGE, 1, p->arroworigin);
    add_tagchange(&data, PREDAMAGE, 0, p->arroworigin);

    add_meta(&data, 1, defender->attack, p->arroworigin);

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

    //check_remove_divine_shield(&data, defender);

    add_powerend(&data);

    add_powerstart(&data, 6, 0, 1, 0, NULL, 0);
    //add_tagchange(&data, NUM_MINIONS_PLAYER_KILLED_THIS_TURN, 1, p->entity_id);		// 368
    //add_tagchange(&data, NUM_MINIONS_KILLED_THIS_TURN, 1, 1);				// 369
    add_tagchange(&data, 398, 1, p->entity_id);
    add_tagchange(&data, 412, 1, p->entity_id);

    //damage_done(p, attacker, defender);
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

    add_powerend(&data);

    add_tagchange(&data, STEP, 10, 1);
    add_powerstart(&data, 5, 2, p->entity_id, 0, NULL, 0);

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

    add_tagchange(&data, NEXT_STEP, 12, 1);
    add_powerend(&data);

    add_ph(&ph, data);
    add_packet(&packet, ph, P_POWERHISTORY);

    powerhistory_dump(ph);


    return packet;
}

struct packet_s *buff(struct conn_client_s *c, struct chooseoption_s *opt, int player, struct card_s *attacker, struct card_s *defender)
{
    struct packet_s *packet = NULL;
    struct powerhistory_s *ph = NULL;
    struct powerhistory_data_s *data = NULL;
    struct hs_holder_s *p;
    struct deck_s *deck;
    int i;

    p = c->data;
    deck = p->deck;


    add_powerstart(&data, 7, 0, attacker->id, defender->id, NULL, 0);
    add_tagchange(&data, RESOURCES_USED, deck->mana_used, p->entity_id);      ///< total cost
    add_tagchange(&data, 418, 26, p->entity_id);
    add_tagchange(&data, NUM_CARDS_PLAYED_THIS_TURN, 1, p->entity_id);
    add_tagchange(&data, 430, 1, p->entity_id);

    add_tagchange(&data, ZONE, 1, attacker->id);
    add_tagchange(&data, ZONE_POSITION, 0, attacker->id);
    add_tagchange(&data, JUST_PLAYED, 1, attacker->id);

    add_tagchange(&data, 397, attacker->id, p->entity_id);
    add_tagchange(&data, CARD_TARGET, defender->id, attacker->id);

    //cards_reorder(deck, &data, attacker, opt, player);

    if(player == 2) {
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

    add_powerstart(&data, 3, -1, attacker->id, defender->id, NULL, 0);

    add_meta(&data, 0, 0, defender->id);
    //add_tagchange(&data, PREDAMAGE, attacker->attack, defender->id);
    //add_tagchange(&data, PREDAMAGE, 0, opt->target);
    add_meta(&data, 3, 0, opt->target);

    add_tagchange(&data, LAST_AFFECTED_BY, attacker->id, defender->id);
    //add_tagchange(&data, DAMAGE, defender->total_health - defender->health, defender->id);
    add_powerend(&data);

    add_tagchange(&data, ZONE, 4, attacker->id);
    add_tagchange(&data, NUM_OPTIONS_PLAYED_THIS_TURN, 1, p->entity_id);

    add_tagchange(&data, WINDFURY, 1, defender->id);
    //damage_done(&data, p, attacker, defender);

    add_powerend(&data);

    add_ph(&ph, data);
    add_packet(&packet, ph, P_POWERHISTORY);

    powerhistory_dump(ph);

    return packet;
}

/*
   static void defender_attacked(struct hs_holder_s *p, struct powerhistory_data_s **data, struct card_s *attacker, struct card_s *defender, struct chooseoption_s *opt)
   {
   add_meta(data, 0, 0, defender->id);
   add_tagchange(data, PREDAMAGE, attacker->attack, defender->id);
   add_tagchange(data, PREDAMAGE, 0, opt->target);
   add_meta(data, 1, attacker->attack, opt->target);

   add_tagchange(data, LAST_AFFECTED_BY, attacker->id, defender->id);
   add_tagchange(data, DAMAGE, defender->total_health - defender->health, defender->id);
   add_powerend(data);

   add_tagchange(data, ZONE, 4, attacker->id);
   add_tagchange(data, NUM_OPTIONS_PLAYED_THIS_TURN, 1, p->entity_id);

//damage_done(p, attacker, defender);
}
*/

struct packet_s *spell_damage_aoe(struct conn_client_s *c, struct chooseoption_s *opt, int player, struct card_s *attacker, struct card_list_s *defenders)
{
    struct packet_s *packet = NULL;
    struct powerhistory_s *ph = NULL;
    struct powerhistory_data_s *data = NULL;
    struct hs_holder_s *p;
    struct deck_s *deck;
    struct card_list_s *def;

    p = c->data;
    deck = p->deck;

    add_powerstart(&data, 7, 0, attacker->id, 0, NULL, 0);
    add_tagchange(&data, RESOURCES_USED, deck->mana_used, p->entity_id);      ///< total cost
    add_tagchange(&data, 418, 26, p->entity_id);
    add_tagchange(&data, NUM_CARDS_PLAYED_THIS_TURN, 1, p->entity_id);
    add_tagchange(&data, 430, 1, p->entity_id);

    add_tagchange(&data, ZONE, 1, attacker->id);
    add_tagchange(&data, ZONE_POSITION, 0, attacker->id);
    add_tagchange(&data, JUST_PLAYED, 1, attacker->id);

    add_tagchange(&data, 397, attacker->id, p->entity_id);

    //cards_reorder(deck, &data, attacker, opt, player);

    /*
       if(player == 2) {
       struct powerhistory_entity_s *show = NULL;
       struct powerhistory_tag_s *card_tags = default_tags(attacker->entity);
       add_full_entity(&show, card_tags, attacker->id, attacker->entity->name, attacker->entity->nname);
       add_ph_data_show(&data, show);
       }
       */

    if(defenders != NULL) {
        add_powerstart(&data, 3, -1, attacker->id, 0, NULL, 0);

        struct powerhistory_info_s *info = NULL;

        for(def = defenders; def != NULL; def = def->next) {
            add_info(&info, def->card->id);
        }

        add_meta_info(&data, 0, 0, info);
    }

    for(def = defenders; def != NULL; def = def->next) {
        hm_log(LOG_DEBUG, lg, "AOE spell %d attacking %d", attacker->id, def->card->id);
        add_tagchange(&data, PREDAMAGE, attacker->attack, def->card->id);
        add_tagchange(&data, PREDAMAGE, 0, def->card->id);
        add_meta(&data, 1, attacker->attack, def->card->id);

        //check_remove_divine_shield(&data, def);

        add_tagchange(&data, LAST_AFFECTED_BY, attacker->id, def->card->id);

        add_tagchange(&data, DAMAGE, def->card->total_health - def->card->health, def->card->id);

        //damage_done(p, attacker, def->card);

    }

    if(defenders != NULL) {
        add_powerend(&data);
    }

    add_tagchange(&data, ZONE, 4, attacker->id);

    add_powerend(&data);

    add_ph(&ph, data);
    add_packet(&packet, ph, P_POWERHISTORY);

    powerhistory_dump(ph);

    flag(&attacker->state, CARD_BOARD, FLAG_UNSET);

    return packet;
}

struct packet_s *spell_damage(struct conn_client_s *c, struct chooseoption_s *opt, int player, struct card_s *attacker, struct card_s *defender)
{
    /*
       struct packet_s *packet = NULL;
       struct powerhistory_s *ph = NULL;
       struct powerhistory_data_s *data = NULL;
       struct hs_holder_s *p;
       struct deck_s *deck;

       p = c->data;
       deck = p->deck;

       add_powerstart(&data, 7, 0, attacker->id, defender->id, NULL, 0);
       add_tagchange(&data, RESOURCES_USED, deck->mana_used, p->entity_id);      ///< total cost
       add_tagchange(&data, 418, 26, p->entity_id);
       add_tagchange(&data, NUM_CARDS_PLAYED_THIS_TURN, 1, p->entity_id);
       add_tagchange(&data, 430, 1, p->entity_id);

       add_tagchange(&data, ZONE, 1, attacker->id);
       add_tagchange(&data, ZONE_POSITION, 0, attacker->id);
       add_tagchange(&data, JUST_PLAYED, 1, attacker->id);

       add_tagchange(&data, 397, attacker->id, p->entity_id);
       add_tagchange(&data, CARD_TARGET, defender->id, attacker->id);

    //cards_reorder(deck, &data, attacker, opt, player);

    //check_remove_divine_shield(&data, defender);


    if(player == 2) {
    for(i = 0; i < deck->ncards; i++) {
    if(deck->cards[i]->id == p->held_card) {
    struct powerhistory_entity_s *show = NULL;
    struct powerhistory_tag_s *card_tags = default_tags(deck->cards[i]->entity);
    add_full_entity(&show, card_tags, deck->cards[i]->id, deck->cards[i]->entity->name, deck->cards[i]->entity->nname);
    add_ph_data_show(&data, show);

    break;
    }
    }
    }

    if(EQFLAG(attacker->state, CARD_NONTARGET)) {
    struct deck_s *opp = p->opponent.deck;
    for(i = 0; i < opp->ncards; i++) {
    if(EQFLAG(opp->cards[i]->state, CARD_BOARD)) {

    add_powerstart(&data, 3, -1, attacker->id, opp->cards[i]->id, NULL, 0);
    defender_attacked(p, &data, attacker, opp->cards[i], opt);
    add_powerend(&data);
    }
    }
    } else {
    add_powerstart(&data, 3, -1, attacker->id, defender->id, NULL, 0);
    defender_attacked(p, &data, attacker, defender, opt);
    add_powerend(&data);
    }

    add_ph(&ph, data);
    add_packet(&packet, ph, P_POWERHISTORY);

    powerhistory_dump(ph);

    return packet;
    */
    return NULL;
}


int choose_option(struct conn_client_s *c, struct chooseoption_s *opt)
{
    //struct packet_s *p1 = NULL, *p2 = NULL;
    //struct hs_holder_s *p;
    //char output[1024 * 8], *ptr;
    //int n;
    //const char opt1[14] = { 0x0e, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x08, 0x08, 0x12, 0x02, 0x08, 0x02, };

    game(c, opt);

    /*

       chooseoption_dump(opt, p->held_card);

    // aoe spell
    if(p->held_card > 0) {
    struct card_s *attacker = card_get(p->deck, NULL, p->held_card);
    if(attacker && flag(&attacker->state, CARD_AOE_ENEMY_MINIONS, FLAG_ISSET)) {
    struct card_list_s *defenders = NULL;

    //attacker->state &= ~CARD_DECK;
    flag(&attacker->state, CARD_DECK, FLAG_UNSET);

    ///< get opponent's board cards
    defenders = cards_get_board(NULL, p->opponent.deck);

    hm_log(LOG_INFO, c->log, "Casting AOE %d against enemy minions", p->held_card);
    assert(attacker);
    game_attack_aoe(p, attacker, defenders);

    p1 = spell_damage_aoe(c, opt, 1, attacker, defenders);
    p2 = spell_damage_aoe(c, opt, 2, attacker, defenders);

    ptr = output;
    n = serialize(p1, &ptr, ptr + sizeof(output));

    hm_log(LOG_INFO, c->log, "Serialized %d bytes", n);
    hm_send(c, output, n);

    if(p2) {
    ptr = output;
    n = serialize(p2, &ptr, ptr + sizeof(output));
    }

    hm_send(p->opponent.client, output, n);


    int so = set_options(p->deck, p->opponent.deck, output, output + sizeof(output), p->game->turn);
    hm_send(c, output, so);

    goto skip;
    }
    }

    // play nontarget hero power
    if(p->held_card > 0 && p->held_card == p->deck->heropower->id &&
    flag(&p->deck->heropower->state, CARD_NONTARGET, FLAG_ISSET)) {

    hm_log(LOG_ERR, c->log, "playing nontarget hero power %d", p->held_card);

    struct card_s *attacker = p->deck->heropower;

    assert(attacker);

    p1 = nontarget(c, attacker);

    ptr = output;
    n = serialize(p1, &ptr, ptr + sizeof(output));
    hm_send(c, output, n);
    hm_send(p->opponent.client, output, n);

    int so = set_options(p->deck, p->opponent.deck, output, output + sizeof(output), p->game->turn);
    hm_send(c, output, so);

    // next turn
    } else if(opt->index == 0) {
    struct card_s *drawn_card = NULL;
    int zone = 0;

    if(p->game->turn % 2 == 0 && p->deck->mana < 10) {
    p->deck->mana++;
    p->opponent.deck->mana++;
}

p->game->turn++;

cards_unset_flag(p->opponent.deck, CARD_EXHAUSTED);

hm_log(LOG_INFO, c->log, "Next turn: %d", p->game->turn);
p1 = next_turn(c, opt, &drawn_card, &zone);
p2 = next_turn(c, opt, &drawn_card, &zone);

ptr = output;
n = serialize(p1, &ptr, ptr + sizeof(output));
hm_send(c, output, n);

ptr = output;
n = serialize(p2, &ptr, ptr + sizeof(output));

int so = set_options(p->opponent.deck, p->deck, output + n, output + sizeof(output) - n, p->game->turn);

//memcpy(output + n + so, opt1, sizeof(opt1));

hm_send(p->opponent.client, output, n + so);

// attack or spell damage
} else if((opt->target != 0 && p->arroworigin != 0)) {
    struct card_s *attacker = card_get(p->deck, NULL, p->arroworigin);
    struct card_s *defender;

    hm_log(LOG_ERR, c->log, "origin: %d target: %lld id %d", p->arroworigin, opt->target, attacker->id);

    defender = card_get(p->deck, p->opponent.deck, opt->target);
    assert(attacker && defender);

    if(flag(&attacker->state, (CARD_SPELL_DAMAGE), FLAG_ISSET)) {
        hm_log(LOG_INFO, c->log, "Casting spell damage %d > %lld", p->arroworigin, opt->target);
        assert(attacker && defender);
        game_attack(p, attacker, defender);

        //attacker->state &= ~CARD_DECK;
        flag(&attacker->state, CARD_DECK, FLAG_UNSET);

        p1 = spell_damage(c, opt, 1, attacker, defender);
        p2 = spell_damage(c, opt, 2, attacker, defender);
    } else if(flag(&attacker->state, CARD_SPELL, FLAG_ISSET) && flag(&attacker->state, CARD_SPELL_HEAL, FLAG_ISSET)) {
        hm_log(LOG_INFO, c->log, "Healing %d > %lld", p->arroworigin, opt->target);
        p1 = heal(c, defender, attacker);
    } else if(flag(&attacker->state, CARD_SPELL, FLAG_ISSET) && flag(&attacker->state, CARD_HP_SPELL_DAMAGE, FLAG_ISSET)) {
        hm_log(LOG_INFO, c->log, "Casting HP spell %d > %lld", p->arroworigin, opt->target);
        p1 = spell_hp_damage(c, defender, attacker);
    } else if(flag(&attacker->state, CARD_SPELL, FLAG_ISSET) && flag(&attacker->state, CARD_WINDFURY, FLAG_ISSET)) {
        hm_log(LOG_INFO, c->log, "Casting windfury spell %d > %lld", p->arroworigin, opt->target);
        p1 = buff(c, opt, 1, attacker, defender);
        p2 = buff(c, opt, 2, attacker, defender);
    } else {
        hm_log(LOG_INFO, c->log, "Attacking %d > %lld", p->arroworigin, opt->target);
        p1 = attack(c, opt);
    }

    ptr = output;
    n = serialize(p1, &ptr, ptr + sizeof(output));

    hm_log(LOG_INFO, c->log, "Serialized %d bytes", n);
    hm_send(c, output, n);

    if(p2) {
        ptr = output;
        n = serialize(p2, &ptr, ptr + sizeof(output));
    }

    hm_send(p->opponent.client, output, n);


    int so = set_options(p->deck, p->opponent.deck, output, output + sizeof(output), p->game->turn);
    hm_send(c, output, so);


    // put card on board
} else if(p->held_card > 0) {
    struct card_s *attacker = card_get(p->deck, NULL, p->held_card);
    assert(attacker);

    hm_log(LOG_INFO, lg, "Putting card %d on board", attacker->id);

    flag(&attacker->state, CARD_HAND, FLAG_UNSET);
    flag(&attacker->state, CARD_BOARD, FLAG_SET);
    if(!flag(&attacker->state, CARD_CHARGE, FLAG_ISSET)) {
        flag(&attacker->state, CARD_EXHAUSTED, FLAG_SET);
    }

    hm_log(LOG_INFO, lg, "Flags for card %d:", attacker->id);
    flag(&attacker->state, -1, FLAG_DUMP);

    p1 = choose_option_player(c, opt, 1, attacker);
    p2 = choose_option_player(c, opt, 2, attacker);

    ptr = output;
    n = serialize(p1, &ptr, ptr + sizeof(output));

    int so = set_options(p->deck, p->opponent.deck, output + n, output + sizeof(output) - n, p->game->turn);

    //hm_log(LOG_DEBUG, lg, "sending %d bytes to initiator", n + so);
    hm_send(c, output, (int)(n + so));

    char output2[2048];
    ptr = output2;
    int n2 = serialize(p2, &ptr, ptr + sizeof(output2));

    //hm_log(LOG_DEBUG, lg, "sending %d bytes to initiator's opponent", n);
    hm_send(p->opponent.client, output2, n2);
}

skip:

// unset origin
p->arroworigin = 0;

// unset
p->held_card = 0;

hm_log(LOG_INFO, c->log, "Unsetting held card %d arroworigin %d", p->held_card, p->arroworigin);
*/

return 0;
}
