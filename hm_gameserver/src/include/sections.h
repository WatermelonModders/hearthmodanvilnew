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
#ifndef SECTIONS_H_
#define SECTIONS_H_

#include <section_buff.h>
#include <section_battlecry.h>
#include <section_defender.h>
#include <section_spell.h>
#include <section_spell_aoe.h>
#include <section_attack.h>
#include <section_attack_effect.h>
#include <section_weapon.h>
#include <section_endturn.h>
#include <section_onboard.h>
#include <section_destroyed.h>
#include <section_weapon_destroyed.h>
#include <section_idle.h>
#include <section_weapon_attack.h>
#include <section_attachment_attack.h>
#include <section_attack_effect_attachment.h>

// built-in variables
#define HSL_attacker (attacker)
#define HSL_defender (defender)
#define HSL_defenders (defenders)
#define HSL_player (p)
#define HSL_opponent ((struct hs_holder_s *)(p->opponent.client->data))
#define HSL_deck_player (p->deck_copy)
#define HSL_deck_opponent (p->opponent.deck_copy)
#define HSL_target (opt->target)
#define HSL_ally_hero (p->deck_copy->hero)
#define HSL_enemy_hero (p->opponent.deck_copy->hero)
#define HSL_weapon_player (p->deck_copy->heroweapon)
#define HSL_weapon_opponent (p->opponent.deck_copy->heroweapon)
#define HSL_deck_defender (defender->parent)
#define HSL_fake_defender (__fake_defender)
#define HSL_fake_attacker (__fake_attacker)
#define HSL_fake_weapon (__fake_weapon)
#define HSL_fake_deck (__fake_deck)
#define HSL_fake_levelup (&__fake_levelup)
#define HSL_no_attacker (-1)
#define HSL_heroes_included 1
#define HSL_heroes_excluded 0
#define HSL_magic_immunity_included 1
#define HSL_magic_immunity_excluded 0

// generic macros
#define def(m_type, m_dst) struct m_type *m_dst = NULL;

#define is_spell(m_dst) (flag(&m_dst->state, CARD_SPELL, FLAG_ISSET))

#define cmp_cardname(m_card, m_src)  (strcmp(m_card->entity->name, m_src) == 0)

#define hsl_set_windfury(m_dst)\
    flag(&m_dst->state, CARD_WINDFURY, FLAG_SET)

#define hsl_set_taunt(m_dst)\
    flag(&m_dst->state, CARD_TAUNT, FLAG_SET);

#define hsl_refresh_mana(m_deck, m_amount)\
    deck->mana_used = (m_amount > deck->mana_used) ? 0 : deck->mana_used - m_amount ;

#define turn_plays(m_card) (m_card->turn_plays)

/**
 * @brief Foreach
 *
 * @param m_alias iterator
 * @param m_src source
 */
#define foreach(m_alias, m_src)\
    for(m_alias = m_src; m_alias != NULL; m_alias = m_alias->next)

/**
 * @brief Find out if controller is current player
 *
 * @param m_card played card
 */
#define is_controller_player(m_card)\
    (p->id == m_card->parent->deck_copy->controller)

// hsl_* macros

// only works on friendly deck
#define hsl_set_magic_immunity(m_dst)\
    force_expire(p, HSL_deck_player, m_dst);\
mechanics_expire(p->game->turn + 2, m_dst, EXPIRE_MAGIC_IMMUNITY, 1, NULL);\
flag(&m_dst->state, CARD_DIVINE_SHIELD, FLAG_SET);

#define hsl_set_fullhealth(m_dst)\
    m_dst->health = m_dst->total_health;\
flag(&m_dst->state, MECHANICS_HEALTH, FLAG_SET);

#define hsl_swap_health(m_dst, m_src)\
    int hp = m_dst->health;\
m_dst->health = m_dst->total_health = m_src->health - m_src->buff_health;\
m_src->health = m_src->total_health = hp + m_src->buff_health;\
flag(&m_dst->state, MECHANICS_HEALTH, FLAG_SET);\
flag(&m_src->state, MECHANICS_HEALTH, FLAG_SET);

#define hsl_set_health(m_dst, m_change)\
    m_dst->health = m_dst->total_health = (m_change + m_dst->buff_health);\
flag(&m_dst->state, MECHANICS_HEALTH, FLAG_SET);

#define hsl_decrease_health(m_dst, m_change)\
    m_dst->buff_health -= m_change;\
m_dst->total_health -= m_change;\
flag(&m_dst->state, MECHANICS_HEALTH, FLAG_SET);\
if(m_dst->health > m_dst->total_health) {\
    m_dst->health = m_dst->total_health;\
}

#define hsl_increase_health(m_dst, m_change)\
    m_dst->buff_health += m_change;\
m_dst->health += m_change;\
m_dst->total_health += m_change;\
flag(&m_dst->state, MECHANICS_HEALTH, FLAG_SET);

#define hsl_replenish_health(m_dst, m_change)\
    if(m_dst->total_health >= (m_dst->health + m_change)) {\
        m_dst->health += m_change;\
        flag(&m_dst->state, MECHANICS_HEALTH, FLAG_SET);\
    }


#define hsl_change_armor(m_dst, m_change)\
    m_dst->armor += m_change;\
flag(&m_dst->state, MECHANICS_ARMOR, FLAG_SET);

#define hsl_change_attack(m_dst, m_change)\
    m_dst->attack += m_change;\
flag(&m_dst->state, MECHANICS_ATTACK, FLAG_SET);


#define hsl_cards_by_name(m_deck, m_name) (cards_by_name(HSL_player, m_deck, m_name, CARD_BOARD))

#define hsl_board_count_by_name(m_deck, m_name) (board_count_by_name(m_deck, m_name))

#define hsl_board_minions_count(m_deck, m_attacker) cards_get_board_count(m_deck, NULL, 0, m_attacker->id)

#define hsl_add_single_attachment(m_player, m_deck, m_dst, m_src) add_single_attachment(m_player, m_deck, m_dst, m_src)

#define hsl_remove_attachment(m_deck, m_opponent, m_card, m_name) (card_remove_attachment(m_deck, m_opponent, m_card, m_name))

#define level(m_dst) (m_dst->boss.level)

#define is_destroyed(m_dst) (flag(&m_dst->state, CARD_DESTROYED, FLAG_ISSET))

#define is_boss(m_dst) (flag(&m_dst->state, CARD_BOSS, FLAG_ISSET))

#define hsl_apply_attachment(m_player, m_deck, m_dst, m_src) (card_apply_attachment(m_player, m_deck, m_dst, m_src))

#define hsl_cards_get_hand_random(m_deck) (cards_get_hand_random(m_deck))

#define hsl_cards_get_hand(m_deck) (cards_get_hand(m_deck))

#define hsl_receive_damage(m_dst, m_dmg)\
    if(m_dst) { m_dst->receive.damage = m_dmg; }

#define hsl_receive_heal(m_dst, m_heal)\
    if(m_dst) { m_dst->receive.heal = m_heal; }

#define hsl_freeze(m_src, m_dst)\
    hm_log(LOG_DEBUG, lg, "Attacker [%s](%d) froze [%s](%d)", m_src->entity->desc, m_src->id, m_dst->entity->desc, m_dst->id);\
mechanics_expire(p->game->turn + 3, m_dst, EXPIRE_FROZEN, 1, NULL);\
flag(&m_dst->state, CARD_FROZEN, FLAG_SET);

#define hsl_add_defender(m_src, m_holder)\
    struct card_list_s *m_holder;\
LIST_ADD(defenders, m_holder, m_src)

#define hsl_return_to_hand(m_card)\
    flag(&m_card->state, CARD_BOARD, FLAG_UNSET);\
flag(&m_card->state, CARD_HAND, FLAG_SET);\
flag(&m_card->state, CARD_EXHAUSTED, FLAG_UNSET);\
m_card->health = m_card->total_health = m_card->default_health;\
flag(&m_card->state, MECHANICS_HEALTH, FLAG_SET);\
m_card->attack = m_card->default_attack;\
flag(&m_card->state, MECHANICS_ATTACK, FLAG_SET);\
card_remove_attachment(hsl_deck(m_card), NULL, m_card, NULL);\
card_destroy(m_card, hsl_deck(m_card), 1);\
/* order is importatnt here, new zone position must come after card is destroyed */\
if(m_card->controller == HSL_deck_player->controller) {\
    m_card->zone_position = ++(HSL_deck_player->handposition);\
} else {\
    m_card->zone_position = ++(HSL_deck_opponent->handposition);\
}\

#define hsl_cards_get_hand(m_deck) (cards_get_hand(m_deck))

#define hsl_add_new_card_hand(m_deck, m_card, m_holder)\
    struct card_s *m_holder = add_new_card(m_deck->parent->game, m_deck, -1, m_card, -1);\
flag(&m_holder->state, CARD_HAND, FLAG_SET);\
m_holder->zone_position = ++(m_deck->handposition);

#define hsl_destroy(m_deck, m_dst)\
{\
    struct card_s *__fake_defender = m_dst;\
    __SECTION_DESTROYED\
}\
flag(&m_dst->state, CARD_DESTROYED, FLAG_SET);\
cards_board_destroyed_reorder(m_deck, m_dst->zone_position);

#define hsl_destroy_inhand(m_dst) card_discard_by_name(HSL_player, HSL_fake_defender, m_dst)

#define hsl_set_weapon_durability(m_dst, m_value)\
    m_dst->durability += m_value;\
flag(&m_dst->state, MECHANICS_DURABILITY, FLAG_SET);

#define hsl_weapon_durability(m_dst) m_dst->durability

#define hsl_destroy_weapon(m_attacker, m_deck)\
    if(m_deck->heroweapon) {\
        /*struct card_list_s *itm;*/\
        __SECTION_WEAPON_DESTROYED;\
        flag(&m_deck->heroweapon->state, CARD_DESTROYED, FLAG_SET);\
        flag(&m_deck->heroweapon->state, CARD_BOARD, FLAG_UNSET);\
        flag(&m_deck->heroweapon->state, CARD_HEROWEAPON, FLAG_UNSET);\
        flag(&m_deck->heroweapon->state, CARD_EXHAUSTED, FLAG_UNSET);\
        \
        /*LIST_ADD(m_attacker->target.card, itm, m_holder->deck_copy->heroweapon);*/\
        \
        flag(&m_deck->heroweapon->state, MECHANICS_DESTROY_WEAPON, FLAG_SET);\
        m_deck->heroweapon = NULL;\
    }

#define is_weapon(m_deck, m_src) (m_deck && m_deck->heroweapon && strcmp(m_deck->heroweapon->entity->name, m_src) == 0)

#define hsl_change_cost(m_card, m_change)\
    (m_card->cost = (m_card->default_cost - m_change >= 0) ? (m_card->default_cost - m_change) : 0);\
flag(&(m_card->state), MECHANICS_COST, FLAG_SET);

#define hsl_card_spell_by_name(m_player, m_deck, m_name) (card_by_name(m_player, m_deck, m_name, CARD_SPELL))
#define hsl_card_board_by_name(m_player, m_deck, m_name) (card_by_name(m_player, m_deck, m_name, CARD_BOARD))
#define hsl_card_hand_by_name(m_player, m_deck, m_name) (card_by_name(m_player, m_deck, m_name, CARD_HAND))

#define hsl_count_attachments(m_deck, m_opponent, m_card, m_name) (card_count_attachment(m_deck, m_opponent, m_card, m_name))

#define hsl_discard_random_hand(m_deck, m_amount, m_holder)\
    int m_holder;\
for(m_holder = 0; m_holder < m_amount; m_holder++) {\
    struct card_s *m_card = cards_get_hand_random(m_deck);\
    if(m_card) {\
        flag(&m_card->state, CARD_DISCARD, FLAG_SET);\
        flag(&m_card->state, CARD_HAND, FLAG_UNSET);\
        cards_reorder_hand_owner(m_deck, m_card->id);\
    }\
}

#define hsl_discard_hand(m_deck, m_all)\
    struct card_list_s *m_holder;\
for(m_holder = m_all; m_holder != NULL; m_holder = m_holder->next) {\
    flag(&m_holder->card->state, CARD_DISCARD, FLAG_SET);\
    cards_reorder_hand_owner(m_deck, m_holder->card->id);\
}

#define card(m_list) (m_list->card)

#define hsl_card(m_target)\
    card_get(HSL_deck_player, HSL_deck_opponent, m_target)

#define hsl_deck(m_card) ((m_card->controller == HSL_deck_player->controller) ? HSL_deck_player : HSL_deck_opponent)

#define default_health(m_card) (m_card->default_health)

#define default_attack(m_card) (m_card->default_attack)

#define hsl_board_all_cards(m_attacker, m_hero) cards_get_board(p->deck_copy, p->opponent.deck_copy, m_hero, m_attacker->id, HSL_magic_immunity_included, 1)

#define hsl_board_all_cards_no_boss(m_attacker, m_hero) (cards_get_board(p->deck_copy, p->opponent.deck_copy, m_hero, m_attacker->id, HSL_magic_immunity_included, 0))

#define hsl_board_friendly_cards(m_deck, m_attacker, m_hero) (cards_get_board(m_deck, NULL, m_hero, m_attacker, HSL_magic_immunity_included, 1))

#define hsl_board_friendly_cards_wattacker(m_hero) cards_get_board(p->deck_copy, NULL, m_hero, -1, HSL_magic_immunity_included, 1);

#define hsl_board_enemy_cards(m_attacker, m_hero, m_mi) cards_get_board(NULL, p->opponent.deck_copy, m_hero, m_attacker->id, m_mi, 1);

#define hsl_board_friendly_no_boss(m_attacker, m_hero, m_mi) cards_get_board(NULL, p->opponent.deck_copy, m_hero, m_attacker->id, m_mi, 0);

#define hsl_board_boss_cards(m_deck) (cards_get_board_boss(m_deck))

#define hsl_attack_aoe(m_dst, m_src, m_fake_levelup) mechanics_attack_aoe(c, opt->position, m_dst, &m_src, m_fake_levelup)
#define hsl_attack(m_dst, m_src, m_fake_levelup) hsl_attack_aoe(m_dst, m_src, m_fake_levelup)

/**
 * @brief Summon card for deck
 *
 * @param m_deck deck to summon to
 * @param m_name name of the card
 * @param m_card card holder
 **/
#define hsl_summon_card(m_deck, m_name, m_card)\
    int m_card##pos = cards_board_last(m_deck);\
if(m_card##pos <= MAX_BOARD) {\
    struct card_s *m_card = add_new_card(p->game, m_deck, -1, m_name, -1);\
    mechanics_put_card_onboard(c, m_card##pos, m_card, 1, NULL, NULL, m_deck);\
}

/**
 * @brief Count number of cards in list of cards
 *
 * @param m_cards list of cards
 **/
#define hsl_count_cards(m_cards) (cards_count(m_cards))

/**
 * @brief Get random card from list of cards
 *
 * @param m_cards list of cards
 **/
#define hsl_random_card(m_cards) (card_random(m_cards))

/**
 * @brief Draw number of cards from player's deck
 *
 * @param m_player deck owner
 * @param m_deck deck to draw from
 * @param m_card card's holder
 * @param m_amount number of cards to draw
 **/
#define hsl_draw_cards(m_player, m_deck, m_card, m_amount)\
    int m_card##idx;\
struct card_s *m_card;\
for(m_card##idx = 0; m_card##idx < m_amount; m_card##idx++) {\
    m_card = draw_random_card(m_deck, m_player->entity_id, 0);\
    if(m_card) {\
        flag(&m_card->state, CARD_HAND, FLAG_SET);\
        struct card_list_s *m_card##itm;\
        LIST_ADD(defenders, m_card##itm, m_card);\
    }\
}

/**
 * @brief Link card to another card's targets
 *
 * @param m_dst destination card
 * @param m_src source card
 **/
#define hsl_link_target(m_dst, m_src)\
    LIST_ADD_LINK(m_dst->target.card, m_src)

/**
 * @brief Add target to defenders
 *
 * @param m_src source card
 * @param m_holder variable placeholder
 **/
#define hsl_add_target(m_src, m_holder)\
    struct card_list_s *m_holder##item;\
LIST_ADD(defenders, m_holder##item, m_src);\

/**
 * @brief Attack as battlecry
 *
 * @param m_attacker attacking card
 * @param m_defender defending card
 * @param m_damage damage caused
 **/
#define hsl_attack_bc(m_attacker, m_defender, m_damage)\
    m_defender->receive.damage = m_damage;\
entity_attack(p, m_attacker, m_defender);

/**
 * @brief Heal defender by certain amount (alias)
 **/
#define hsl_heal_bc(m_attacker, m_defender, m_damage)\
    hsl_heal(m_attacker, m_defender, m_damage);
/*
   m_defender->receive.heal = m_damage;\
#entity_attack(p, m_attacker, m_defender);\
#struct card_list_s *item;\
LIST_ADD(m_attacker->target.card, item, m_defender);
*/

/**
 * @brief Heal defender by certain amount
 *
 * @param m_attacker healer card
 * @param m_defender receiver card
 * @param m_damage amount of healing
 **/
#define hsl_heal(m_attacker, m_defender, m_damage)\
    m_defender->receive.heal = m_damage;\
entity_attack(p, m_attacker, m_defender);\
struct card_list_s *item;\
LIST_ADD(m_attacker->target.card, item, m_defender);

/**
 * @brief Mark target as corrupted
 *
 * Corrupted target is destroyed after m_turn turns
 *
 * @param m_turn number of turns from current turn when card is destroyed
 * @param m_defender destination card
 */
#define hsl_buff_corrupt_expire(m_turn, m_defender)\
    flag(&(m_defender->state), CARD_CORRUPTED, FLAG_SET);\
mechanics_expire(p->game->turn + m_turn, m_defender, EXPIRE_DESTROY, 1, NULL)

/**
 * @brief Change attack for m_turn turns
 *
 * @param m_turn expiration turn
 * @param m_defender destination card
 * @param m_attack value to change
 */
#define hsl_buff_attack_expire(m_turn, m_defender, m_attack)\
    m_defender->attack += m_attack;\
flag(&(m_defender->state), MECHANICS_ATTACK, FLAG_SET);\
mechanics_expire(p->game->turn + m_turn, m_defender, EXPIRE_ATTACK, -(m_attack), NULL);

/**
 * @brief Change health of card
 *
 * @param m_card destination card
 * @param m_health health value
 */
#define hsl_add_attachment(m_player, m_attacker, m_defender, m_turn)\
    add_single_attachment(m_player, hsl_deck(m_defender), m_attacker, m_defender);\
mechanics_expire(p->game->turn + m_turn, m_defender, EXPIRE_ATTACHMENT, 0, m_attacker->attachment);

/**
 * @brief Change health of card
 *
 * @param m_card destination card
 * @param m_health health value
 */
#define buff_health(m_card, m_health)\
    defenders->card->health = m_health;\
flag(&(m_card->state), MECHANICS_HEALTH, FLAG_SET);

/**
 * @brief Set divine shield to card
 *
 * @param m_card destination card
 */
#define hsl_divine_shield(m_card)\
    flag(&(m_card->state), CARD_DIVINE_SHIELD, FLAG_SET);

/**
 * @brief Set flag to card
 *
 * @param m_card destination card
 * @param m_flag flag to set
 */
#define setflag(m_card, m_flag)\
    flag(&(m_card->state), m_flag, FLAG_SET);

/**
 * @brief Unset flag from card
 *
 * @param m_card destination card
 * @param m_flag flag to unset
 */
#define unsetflag(m_card, m_flag)\
    flag(&(m_card->state), m_flag, FLAG_UNSET);

/**
 * @brief Unset windfury flag from card
 *
 * @param m_card destination card
 */
#define hsl_unset_windfury(m_card) (flag(&(m_card->state), CARD_WINDFURY, FLAG_UNSET))

/**
 * @brief Set exhausted flag to card
 *
 * @param m_card destination card
 */
#define hsl_set_exhausted(m_card) (flag(&(m_card->state), CARD_EXHAUSTED, FLAG_SET))

/**
 * @brief Unset exhausted flag from card
 *
 * @param m_card destination card
 */
#define hsl_unset_exhausted(m_card) (flag(&(m_card->state), CARD_EXHAUSTED, FLAG_UNSET))

/**
 * @brief Transform defender to m_name
 *
 * Any mechanics like polymorph
 *
 * @param m_attacker card with ability to transform target
 * @param m_defender card being attacked
 * @param m_name card code name
 */
#define hsl_transform(m_attacker, m_defender,m_name)\
    struct card_s *card;\
card = add_new_card(p->game, p->opponent.deck_copy, -1, m_name, -1);\
card->zone_position = m_defender->zone_position;\
mechanics_put_card_onboard(c, card->zone_position, card, 1, NULL, NULL);\
m_defender->linked = card->id;\
flag(&m_defender->state, CARD_LINKED, FLAG_SET);\
flag(&m_defender->state, CARD_HIDE, FLAG_SET);\
flag(&m_defender->state, CARD_BOARD, FLAG_UNSET);

/**
 * @brief Give weapon to player
 *
 * Called only in '.section onboard'. There is no need to linked destroyed weapon to defenders
 *
 * @param m_dst hero card
 */
#define hsl_give_weapon(m_dst)\
    if(m_dst->heroweapon != NULL) {\
        /* destroy weapon first */\
        {\
            struct card_s *__fake_weapon = m_dst->heroweapon;\
            __SECTION_WEAPON_DESTROYED;\
        }\
        \
        m_dst->heroweapon->priority_override = 900;\
        flag(&(m_dst->heroweapon->state), CARD_DESTROYED, FLAG_SET);\
        flag(&(m_dst->heroweapon->state), CARD_BOARD, FLAG_UNSET);\
        flag(&(m_dst->heroweapon->state), CARD_HEROWEAPON, FLAG_UNSET);\
    }\
flag(&attacker->state, CARD_BOARD, FLAG_SET);\
flag(&attacker->state, CARD_HEROWEAPON, FLAG_SET);\
m_dst->heroweapon = attacker;

#endif
