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

void power_attack(struct conn_client_s *c, struct hs_holder_s *p, struct chooseoption_s *opt)
{
    struct card_s *attacker = NULL, *defender;
    struct deck_s *deck, *opponent;
    struct packet_s *p1 = NULL, *p2 = NULL, *extra = NULL;
    struct card_list_s *defenders = NULL, *clitem, *def;

    decks_copy(p);

    deck = p->deck_copy;
    opponent = p->opponent.deck_copy;

    attacker = card_get(deck, NULL, p->arroworigin);
    if(!attacker) attacker = card_get(deck, NULL, p->held_card);

    assert(attacker);

    if(flag(&attacker->state, CARD_HAND, FLAG_ISSET)) {
        flag(&attacker->state, CARD_HAND, FLAG_UNSET);
    }

    if(flag(&attacker->state, CARD_SPELL, FLAG_ISSET)) {
        // update resources
        update_resources(deck, attacker, 0);

        // put spell on board
        if(!flag(&attacker->state, CARD_HEROPOWER, FLAG_ISSET)) {
            flag(&attacker->state, CARD_BOARD, FLAG_SET);
        }
    }

    defender = card_get(deck, opponent, opt->target);

    // there can only be none or one defender here
    if(defender) {
        LIST_ADD(defenders, clitem, defender);
    }

    hm_log(LOG_ERR, c->log, "Power attack: origin: %d target: %lld held: %d attacker: [%s](%d) defenders: %p", p->arroworigin, opt->target, p->held_card, attacker->entity->desc, attacker->id, defenders);

    if(/*p->held_card > 0 && */flag(&attacker->state, CARD_AOE, FLAG_ISSET)) {

        hm_log(LOG_INFO, c->log, "Attacker AOE [%s](%d)", attacker->entity->desc, attacker->id);
        /*
        ///< get opponent's board cards
        if(flag(&attacker_aoe->state, CARD_AOE_ENEMY_MINIONS, FLAG_ISSET)) {
        defenders = cards_get_board(NULL, p->opponent.deck_copy, 0, -1);
        }  else if(flag(&attacker_aoe->state, CARD_AOE_ALLENEMY, FLAG_ISSET)) {
        defenders = cards_get_board(NULL, p->opponent.deck_copy, 1, -1);
        }  else if(flag(&attacker_aoe->state, CARD_AOE_TARGET_ALL, FLAG_ISSET)) {
        defenders = cards_get_board(p->deck_copy, p->opponent.deck_copy, 1, -1);
        }  else if(flag(&attacker_aoe->state, CARD_AOE_MINIONS, FLAG_ISSET)) {
        defenders = cards_get_board(p->deck_copy, p->opponent.deck_copy, 0, -1);
        } else if(flag(&attacker_aoe->state, CARD_AOE_CLEAVE, FLAG_ISSET) || flag(&attacker_aoe->state, CARD_AOE_MULTISHOT, FLAG_ISSET)) {
        defenders = cards_get_random(p->opponent.deck_copy, 2, attacker, 1);
        } else if(flag(&attacker_aoe->state, CARD_TRACKING, FLAG_ISSET)) {
        defenders = cards_get_random(p->deck_copy, 3, attacker_aoe, 6);
        flag(&attacker_aoe->state, MECHANICS_SELECTED, FLAG_SET);

        } else if(flag(&attacker_aoe->state, CARD_ARCANE_MISSILES, FLAG_ISSET)) {
        int tsp = attacker_aoe->attack + p->deck_copy->spellpower;
        defenders = cards_get_random(p->opponent.deck_copy, tsp, attacker_aoe, 1);

        } else if(flag(&attacker_aoe->state, CARD_SINISTER_STRIKE, FLAG_ISSET) ||
        flag(&attacker_aoe->state, CARD_MIND_BLAST, FLAG_ISSET)) {
        LIST_ADD(defenders, clitem, opponent->hero);
        }

        hm_log(LOG_INFO, c->log, "Casting AOE card [%s](%d) against:", attacker_aoe->entity->desc, attacker_aoe->id);
        for(def = defenders; def != NULL; def = def->next) {
        hm_log(LOG_INFO, c->log, "\t[%s](%d)", def->card->entity->desc, def->card->id);
        }

        mechanics_attack_aoe(c, opt->position, attacker_aoe, &defenders);
        */
        {
            struct card_s *__fake_levelup = NULL;
            __SECTION_SPELL_AOE
        }

        {
            struct card_s *__fake_defender = attacker;
            __SECTION_DESTROYED
        }

        p1 = net_spell_aoe(c, opt, deck->controller, attacker);
        p2 = net_spell_aoe(c, opt, opponent->controller, attacker);

        if(flag(&attacker->state, CARD_TRACKING, FLAG_ISSET)) {
            struct entitychoices_s *ec;
            ec = chooseone_choices(defenders, p->id, attacker->id);
            add_packet(&extra, ec, P_ENTITYCHOICES);

            entitychoices_dump(ec);

            // set client to choose one
            p->choose_one = 1;
        }


    } else if(
            attacker && flag(&attacker->state, CARD_BUFF, FLAG_ISSET) && flag(&attacker->state, CARD_SPELL, FLAG_ISSET)) {

        /*
           if(flag(&attacker->state, CARD_BLOODLUST, FLAG_ISSET)) {
           defenders = cards_get_board(p->deck_copy, NULL, 0, attacker->id);
           } else if(flag(&attacker->state, CARD_VANISH, FLAG_ISSET)) {
           defenders = cards_get_board(p->deck_copy, p->opponent.deck_copy, 0, attacker->id);
           } else if(flag(&attacker->state, CARD_SAVAGE_ROAR, FLAG_ISSET)) {
           defenders = cards_get_board(p->deck_copy, NULL, 1, attacker->id);
           } else if(flag(&attacker->state, CARD_CLAW, FLAG_ISSET) ||
           flag(&attacker->state, CARD_HEROIC_STRIKE, FLAG_ISSET)) {
           LIST_ADD(defenders, clitem, deck->hero);
           } else if(flag(&attacker->state, CARD_MIND_VISION, FLAG_ISSET)) {
           defenders = NULL;
           } else {
           LIST_ADD(defenders, clitem, defender);
           }
           */

        hm_log(LOG_INFO, c->log, "Buff [%s)(%d)", attacker->entity->desc, attacker->id);

        mechanics_buff(c, opt, attacker, &defenders);

        for(def = defenders; def != NULL; def = def->next) {
            hm_log(LOG_INFO, c->log, "Buffing [%s)(%d) > [%s](%d)", attacker->entity->desc, attacker->id, def->card->entity->desc, def->card->id);
        }

        p1 = net_buff(c, opt, deck->controller, attacker, defenders);
        p2 = net_buff(c, opt, opponent->controller, attacker, defenders);

        /*
           } else if(attacker && flag(&attacker->state, CARD_BATTLECRY_TARGET, FLAG_ISSET) && (
           (attacker->total_plays == 0 && flag(&attacker->state, CARD_BATTLECRY_HEAL2_TARGET, FLAG_ISSET)) ||
           (attacker->total_plays == 0 && flag(&attacker->state, CARD_BATTLECRY_DAMAGE1_TARGET, FLAG_ISSET)) ||
           (attacker->total_plays == 0 && flag(&attacker->state, CARD_BATTLECRY_DAMAGE2_TARGET, FLAG_ISSET)) ||
           (attacker->total_plays == 0 && flag(&attacker->state, CARD_BATTLECRY_DAMAGE3_TARGET, FLAG_ISSET)))
           ) {

           flag(&attacker->state, MECHANICS_BATTLECRY_TRIGGER, FLAG_SET);

           LIST_ADD(defenders, clitem, defender);

           hm_log(LOG_INFO, c->log, "Battlecry target %d > %lld, total plays %d", p->arroworigin, opt->target, attacker->total_plays);
           mechanics_attack(c, opt, attacker, defenders);

           p1 = net_spell(c, opt, 1, attacker, defenders);
           */
    } else if(flag(&attacker->state, CARD_SPELL, FLAG_ISSET) && flag(&attacker->state, CARD_HP_SPELL_DAMAGE, FLAG_ISSET)) {

        //LIST_ADD(defenders, clitem, defender);

        //mechanics_attack(c, opt, attacker, defenders);
        hm_log(LOG_INFO, c->log, "Casting HP spell %d > %lld", p->arroworigin, opt->target);
        p1 = net_spell_hp_damage(c, defenders, attacker);

    } else if(flag(&attacker->state, (CARD_SPELL), FLAG_ISSET)) {

        hm_log(LOG_INFO, c->log, "Casting spell %d > %lld", p->arroworigin, opt->target);

        assert(attacker);

        /*
           LIST_ADD(defenders, clitem, defender);

           if(flag(&attacker->state, CARD_SWIPE, FLAG_ISSET)) {
           mechanics_attack_swipe(c, opt->position, attacker, &defenders);
           } else {
           mechanics_attack(c, opt, attacker, defenders);
           }

*/

        struct card_s *__fake_levelup = NULL;
        __SECTION_SPELL

        {
            struct card_s *__fake_defender = attacker;
            __SECTION_DESTROYED
        }

        p1 = net_spell(c, opt, deck->controller, attacker, defenders, __fake_levelup);
        if(!flag(&attacker->state, CARD_HEROPOWER, FLAG_ISSET)) {
            p2 = net_spell(c, opt, opponent->controller, attacker, defenders, __fake_levelup);
        }
    } else {
        hm_log(LOG_INFO, c->log, "Attacking %d > %lld", p->arroworigin, opt->target);

        struct card_s *__fake_levelup = NULL;
        __SECTION_ATTACK

            p1 = net_attack(c, opt, attacker, defender, deck->controller);
        p2 = net_attack(c, opt, attacker, defender, opponent->controller);
    }

    decks_swap(p);

    net_send(c, p1);

    if(p2) {
        net_send(p->opponent.client, p2);
    } else {
        net_send(p->opponent.client, p1);
    }

    if(extra) {
        net_send(c, extra);
        //net_send(p->opponent.client, extra);
    } else {
        net_send_options(c, p->deck, p->opponent.deck, p->game->turn);
    }

    packet_free(p1);
    if(p2) packet_free(p2);
    if(extra) packet_free(extra);

    /*
       if(p->em)
       em_fire(p);
       if(player2->em)
       em_fire(player2);
       */


    /*
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
       */
}
