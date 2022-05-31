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

#ifdef TEST_DECK
#define LEVEL3_EXP   2
#define LEVEL2_EXP   1
#else
//#define LEVEL3_EXP   10
//#define LEVEL2_EXP   5
#define LEVEL3_EXP   2
#define LEVEL2_EXP   1
#endif

#define LEVEL2      2
#define LEVEL3      3

static void level_up(struct hs_holder_s *p, struct deck_s *deck, struct card_s *dst, struct card_s *target)
{
    assert(dst && target);

    dst->boss.experience += target->cost;

    if(dst->boss.experience >= LEVEL3_EXP && dst->boss.level < LEVEL3) {

        card_remove_attachment(deck, NULL, dst, LEVEL_1);
        card_remove_attachment(deck, NULL, dst, LEVEL_2);
        card_apply_attachment(p, deck, dst, LEVEL_3);

        if(dst->entity->level3) {
            hsl_add_new_card_hand(deck, dst->entity->level3, holder);
        }

        // when we bypass level 2
        if(dst->boss.level == 0 && dst->entity->level2) {
            hsl_add_new_card_hand(deck, dst->entity->level2, holder);
        }

        dst->boss.level = LEVEL3;

        hm_log(LOG_DEBUG, lg, "Boss [%s](%d) reached level 3", dst->entity->desc, dst->id);
    } else if(dst->boss.experience >= LEVEL2_EXP && dst->boss.level < LEVEL2) {

        dst->boss.level = LEVEL2;

        card_remove_attachment(deck, NULL, dst, LEVEL_1);
        card_apply_attachment(p, deck, dst, LEVEL_2);

        if(dst->entity->level2) {
            hsl_add_new_card_hand(deck, dst->entity->level2, holder);
        }

        hm_log(LOG_DEBUG, lg, "Boss [%s](%d) reached level 2", dst->entity->desc, dst->id);
    } else {
        hm_log(LOG_DEBUG, lg, "No leveling up for boss [%s](%d) exp %d", dst->entity->desc, dst->id, dst->boss.experience);
    }
}

int game_attack_aoe(struct hs_holder_s *p, struct card_s *attacker, struct card_list_s *defenders, struct card_s **fake_levelup)
{	
    struct card_list_s *def;

#define WEAPON_ATTACK(m_attacker, m_deck)\
    if(flag(&m_attacker->state, CARD_BOSS, FLAG_ISSET)) {\
        {\
            struct card_s *__fake_weapon = m_deck->heroweapon;\
            struct deck_s *__fake_deck = m_deck;\
            __SECTION_WEAPON_ATTACK\
        }\
    }

#define ATTACHMENT_ATTACK(m_attacker, m_deck)\
    {\
        struct card_s *__fake_attacker = m_attacker;\
        struct deck_s *__fake_deck = m_deck;\
        \
        if(flag(&attacker->state, CARD_BOSS, FLAG_ISSET) ||\
                flag(&attacker->state, CARD_MINION, FLAG_ISSET)) {\
            \
            __SECTION_ATTACHMENT_ATTACK\
            \
        }\
    }

    for(def = defenders; def != NULL; def = def->next) {
        entity_attack(p, attacker, def->card);

        ATTACHMENT_ATTACK(def->card, p->opponent.deck_copy);

        // add experience
        if(flag(&attacker->state, CARD_SPELL, FLAG_ISSET) &&
                flag(&attacker->state, CARD_LEVELUP, FLAG_ISSET) &&
                flag(&def->card->state, CARD_DESTROYED, FLAG_ISSET)) {

            // spell experience association
            struct card_s *boss = card_get_levelup_boss(p->deck_copy, attacker);
            if(boss) {
                level_up(p, p->deck_copy, boss, def->card);
            } else {
                hm_log(LOG_DEBUG, lg, "CARD_LEVELUP specified for card %d but boss not found", attacker->id);
                abort();
            }
        } else if(flag(&attacker->state, CARD_BOSS, FLAG_ISSET) &&
                !flag(&attacker->state, CARD_DESTROYED, FLAG_ISSET) &&
                flag(&def->card->state, CARD_DESTROYED, FLAG_ISSET)) {

            level_up(p, p->deck_copy, attacker, def->card);

        } else if(flag(&def->card->state, CARD_BOSS, FLAG_ISSET) &&
                !flag(&def->card->state, CARD_DESTROYED, FLAG_ISSET) &&
                flag(&attacker->state, CARD_DESTROYED, FLAG_ISSET)) {

            level_up(p, p->opponent.deck_copy, def->card, attacker);

        } else {
            hm_log(LOG_DEBUG, lg, "No boss involved or destroyed: Attacker [%s](%d) > Defender [%s](%d) %d %d %d", attacker->entity->desc, attacker->id, def->card->entity->desc, def->card->id, flag(&attacker->state, CARD_SPELL, FLAG_ISSET), flag(&attacker->state, CARD_LEVELUP, FLAG_ISSET), flag(&def->card->state, CARD_DESTROYED, FLAG_ISSET));
        }
    }

    WEAPON_ATTACK(attacker, p->deck_copy);
    ATTACHMENT_ATTACK(attacker, p->deck_copy);

    return 0;
}
