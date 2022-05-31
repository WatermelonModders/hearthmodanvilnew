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

#define F_ENRAGED 0x1

static void consider_attachments(struct deck_s *deck, struct deck_s *opponent, struct card_s *card, int *heal, int *damage)
{
    // check buffs
    struct card_link_s *child;
    struct card_s *child_card;

    int flag = 0;

    for(child = card->attached_children; child != NULL; child = child->next) {
        child_card = card_get(deck, opponent, child->id);

        assert(child_card);

        // Enraged - 80% damage reduction
        if(!EQFLAG(flag, F_ENRAGED) && strcmp(child_card->entity->name, "CS2_011o") == 0 && *damage > 0) {
            flag |= F_ENRAGED;
            int d = *damage;
            if((*damage *= .2) < 1) *damage = 1;
            hm_log(LOG_DEBUG, lg, "Defender [%s](%d) incoming damage reduced from %d to %d", card->entity->desc, card->id, d, *damage);
        }
    }
}

static int tags_change(struct hs_holder_s *p, struct card_s *attacker, struct card_s *defender)
{
    int attack = 0, heal = 0;
    int defender_health;
    int defender_armor;
    /*
       int a_side;
       int d_side;

       a_side = attacker->controller;
       d_side = defender->controller;
       */

    defender_health = defender->health;
    defender_armor = defender->armor;

    if(defender->receive.damage > 0) {
        attack = defender->receive.damage;
        if(flag(&attacker->state, CARD_SPELL, FLAG_ISSET) && p->deck_copy->spellpower > 0) {
            attack += p->deck_copy->spellpower;
        }
    } else if(defender->receive.heal > 0) {
        heal = defender->receive.heal;
    }

    consider_attachments(p->deck_copy, p->opponent.deck_copy, defender, &heal, &attack);

    __SECTION_ATTACK_EFFECT

        // heal
        if(heal > 0) {
            hm_log(LOG_DEBUG, lg, "Attacker [%s](%d) healing [%s](%d) healpower: %d", attacker->entity->desc, attacker->id, defender->entity->desc, defender->id, heal);
            if(defender->health + heal > defender->total_health) {
                defender->health = defender->total_health;
            } else {
                defender->health += heal;
            }

            defender->receive.heal = heal;
            // attack
        } else {
            if(attack == 0) {
                attack = attacker->attack;
            }

            __SECTION_ATTACK_EFFECT_ATTACHMENT

                hm_log(LOG_DEBUG, lg, "Attacker [%s](%d) attacking [%s](%d) power: %d", attacker->entity->desc, attacker->id, defender->entity->desc, defender->id, attack);

            defender->receive.damage = attack;

            if(attack >= (defender->health + defender->armor)) {
                defender->health = 0;
                defender->armor = 0;
                flag(&defender->state, CARD_DESTROYED, FLAG_SET);
            } else {
                defender->armor -= attack;
                if(defender->armor < 0) {
                    defender->health += defender->armor;
                    defender->armor = 0;
                }
            }
        }

    if(defender_health != defender->health) {
        flag(&defender->state, MECHANICS_DAMAGE, FLAG_SET);

        if(defender_health > defender->health) {
            flag(&defender->state, MECHANICS_ATTACK_DAMAGE, FLAG_SET);

            if(flag(&defender->state, CARD_GURU_BERSERKER, FLAG_ISSET) && defender->health != 0) {
                defender->attack += 3;
                flag(&defender->state, MECHANICS_ATTACK, FLAG_SET);
                add_single_attachment(p, p->deck_copy, defender, defender);
            }

            hm_log(LOG_DEBUG, lg, "Defender: %d set to be damaged", defender->id);

        } else {
            flag(&defender->state, MECHANICS_ATTACK_HEAL, FLAG_SET);

            hm_log(LOG_DEBUG, lg, "Defender: %d set to be healed", defender->id);

            //attacker->target.heal = attacker->attack;
            //defender->receive.heal = attacker->attack;
        }
    } else {
        hm_log(LOG_DEBUG, lg, "Defender: %d health unchanged", defender->id);
    }

    if(defender->armor != defender_armor) {
        flag(&defender->state, MECHANICS_ARMOR, FLAG_SET);
    }

    return 0;
}

int entity_attack(struct hs_holder_s *p, struct card_s *attacker, struct card_s *defender)
{	
    struct card_list_s *item;

    hm_log(LOG_DEBUG, lg, "Attack");
    hm_log(LOG_DEBUG, lg, "Attacker: %d Health: %d Attack: %d Deck: %d", attacker->id, attacker->health, attacker->attack, attacker->controller);
    hm_log(LOG_DEBUG, lg, "Defender: %d Health: %d Attack: %d Deck: %d", defender->id, defender->health, defender->attack, defender->controller);

    LIST_ADD(attacker->target.card, item, defender);

    // increase total plays of attacker
    attacker->total_plays++;
    attacker->turn_plays++;

    tags_change(p, attacker, defender);

    ///< defender strikes back
    if(!flag(&attacker->state, CARD_SPELL, FLAG_ISSET) &&
            !flag(&attacker->state, MECHANICS_BATTLECRY_TRIGGER, FLAG_ISSET) &&
            // FIXME: !!! do friendly minions ever attack each other physically?
            attacker->parent->deck_copy->controller != defender->parent->deck_copy->controller
      ) {
        tags_change(p, defender, attacker);
    }

    if(!flag(&attacker->state, CARD_SPELL, FLAG_ISSET)) {
        if(attacker->health == 0) {

            hm_log(LOG_DEBUG, lg, "Destroying attacker %d from deck %d", attacker->id, p->deck_copy->controller);
            card_destroy(attacker, p->deck_copy, 0);

        } else {
            if(!(attacker->turn_plays == 1 && flag(&attacker->state, CARD_WINDFURY, FLAG_ISSET))) {
                flag(&attacker->state, CARD_EXHAUSTED, FLAG_SET);
            }
        }
    }

    if(defender->health == 0) {
        if(flag(&defender->state, CARD_HERO, FLAG_ISSET)) {
            // TODO: game over
            hm_log(LOG_DEBUG, lg, "Game over");
        } else {
            hm_log(LOG_DEBUG, lg, "Destroying defender %d from deck %d", defender->id, p->opponent.deck_copy->controller);
            card_destroy(defender, p->opponent.deck_copy, 0);
        }
    }

    {
        struct card_s *__fake_defender = attacker;
        struct card_s *__fake_attacker = defender;
        if(__fake_attacker->attack > 0) {
            __SECTION_DEFENDER
        }
    }

    {
        struct card_s *__fake_defender = defender;
        struct card_s *__fake_attacker = attacker;
        if(__fake_attacker->attack > 0) {
            __SECTION_DEFENDER
        }
    }

    hm_log(LOG_DEBUG, lg, "Post Attacker: [%s](%d) Health: %d", attacker->entity->desc, attacker->id, attacker->health);
    hm_log(LOG_DEBUG, lg, "Post Defender: [%s](%d) Health: %d", defender->entity->desc, defender->id, defender->health);

    return 0;
}
