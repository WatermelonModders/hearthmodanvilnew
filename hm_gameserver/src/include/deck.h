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
#ifndef DECK_H_
#define DECK_H_

#define DECK_INIT			0x1

#define G_P1_CARDS_CHOSEN	0x1
#define G_P2_CARDS_CHOSEN	0x2

#define MAX_BOSS            2
#define MAX_BOARD           7

#define LIST_ADD(list__, item__, src__)\
    item__ = malloc(sizeof(*item__));\
item__->card = src__;\
item__->next = NULL;\
if(list__ != NULL && (list__)->tail) {\
    (list__)->tail->next = item__;\
    (list__)->tail = item__;\
} else {\
    (list__) = item__;\
    (list__)->tail = item__;\
}

#define LIST_ADD_LINK(list__, src__)\
    list__ = src__;

enum deck_mod_e {
    DECK_MODIFIER_ATTACK = 1,
    DECK_MODIFIER_HEALTH,
    DECK_MODIFIER_ATTACK_CHARGE,
    DECK_MODIFIER_ATTACK_BEAST,
};

enum deck_free_e {
    FREE_DECK_ATTACHMENT = (1<<1),
    FREE_CARD_CHILDREN = (1<<2),
    FREE_CARD_EXPIRE = (1<<3),
};

struct card_expire_s {
    int turn;
    int attack;
    int frozen;
    int destroy;
    int magic_immunity;
    const char *attachment;

    struct card_expire_s *next;
};

struct attachment_list_s {
    const char *name;
    enum flags_e group;
    struct attachment_list_s *next;
};

struct card_link_s {
    int id;
    const char *name;
    struct card_link_s *next, *tail;
};

struct card_s {
    struct hs_holder_s *parent;

    int controller;
    int id;
    struct flags_s state;

    const char *attachment;                     // name of CARD_ATTACHMENT
    int attached_to;                            // only for CARD_ATTACHMENT
    struct card_link_s *attached_children;      // children

    const struct ent_s *entity;
    int zone_position;

    int total_health;
    int health;
    int default_health;
    int buff_health;

    int attack;
    int default_attack;

    int cost;
    int default_cost;

    int cardtype;
    int durability;
    int maxdurability;
    int armor;
    int actions_number;
    int spellpower;

    struct {
        unsigned int flags;

        struct card_list_s *card;
    } target;

    struct {
        int damage;
        int heal;
    } receive;

    struct {
        char level;
        int experience;
    } boss;

    int linked;

    struct card_expire_s *expire;

    int priority_override;
    int total_plays; // total number of plays with this card, zero'd when returned to hand
    int turn_plays; // total number of plays since turn started
};

struct deck_s {
    struct hs_holder_s *parent;

    u64 bnet_hi;
    int controller;

    int mana;
    int mana_used;
    int spellpower;

    int handposition;
    int ncards;
    struct card_s **cards;

    struct {
        int attack;
        int attack_charge;
        int attack_beast;
        int health;
    } modifiers;


    struct card_s *hero;
    struct card_s *heropower;
    struct card_s *heroweapon;

    int board_position[MAX_BOARD];
    struct attachment_list_s *attachments;

    int fatigue;
    int gameover;
    unsigned int flags;
};

struct card_list_s {
    struct card_s *card;
    struct card_list_s *tail, *next;
};

struct game_s {
    int entity_id;
    unsigned int flags;
    int turn;
    int maxentity_id;

    struct ev_timer turntimer, force_next_turn;
};

struct hs_holder_s {
    struct conn_client_s *client;

    struct deck_s *deck;
    struct deck_s *deck_copy;
    int id;
    int entity_id;
    int held_card;
    int arroworigin;
    int choose_one;

    struct {
        int accessible;
        struct deck_s *deck;
        struct deck_s *deck_copy;
        struct conn_client_s *client;
    } opponent;

    struct game_s *game;

    unsigned int flags;
};

void holder_free(struct hs_holder_s *h);
int append_decks(struct conn_client_s *p1, struct conn_client_s *p2);
int append_game(struct conn_client_s *p1, struct conn_client_s *p2);
void card_to_deck(struct card_s *card);
struct card_s *draw_random_card(struct deck_s *deck, int entity_id, int zone_position);
struct card_s *new_card(const int id, const char *name, int controller);
void card_used_reorder(struct deck_s *deck, u64 id);
void cards_reorder_board_opponent(struct powerhistory_data_s **data, struct deck_s *deck);
void cards_reorder_board_owner(struct deck_s *deck, u64 held, u64 position);
struct card_s *card_get(struct deck_s *deck, struct deck_s *opponent, const int id);
struct card_list_s *cards_get_board(struct deck_s *deck, struct deck_s *opponent, int hero, int exclude, int magic_immunity, int boss);
struct card_s *card_get_hero(struct deck_s *deck);
void cards_reorder_hand_owner(struct deck_s *deck, u64 id);
void cards_reorder_hand_opponent(struct powerhistory_data_s **data, struct deck_s *deck);
void cards_board_destroyed_reorder(struct deck_s *deck, int position);
int game_attack(struct hs_holder_s *p, struct card_s *attacker, struct card_s *defender);
int update_resources(struct deck_s *deck, struct card_s *attacker, int refresh);
void cards_unset_flag(struct deck_s *deck, enum flags_e flags);
void card_set_exhausted(struct card_s *card);
int cards_board_last(struct deck_s *deck);
struct card_s *add_new_card(struct game_s *game, struct deck_s *deck, int index, const char *card, unsigned int flags);
int zone_position_last(struct deck_s *deck);
void unset_mechanics_flags(struct hs_holder_s *p);
void cards_reorder(struct hs_holder_s *p, struct deck_s *deck, struct card_s *attacker, u64 position, int summoned);
int cards_get_board_count(struct deck_s *deck, struct deck_s *opponent, int hero, int exclude);
void decks_copy(struct hs_holder_s *p);
void decks_swap(struct hs_holder_s *p);
void deck_modifier(struct hs_holder_s *p, struct deck_s *deck, struct card_s *attacker, enum deck_mod_e key, int value);
void deck_apply_modifiers(struct hs_holder_s *p, struct deck_s *deck, struct card_s *attacker, u64 position);
void card_destroy(struct card_s *card, struct deck_s *deck, int to_hand);
void deck_destroy_modifier(struct deck_s *deck, struct card_s *defender);
struct card_s *cards_get_hand_random(struct deck_s *deck);
void copy_card(struct card_s **dst, struct card_s *src);
struct card_list_s *cards_get_random(struct deck_s *deck, int number, struct card_s *attacker, int zone);
int cards_get_board_count_flag(struct deck_s *deck, struct card_s *card, int f);
int cards_get_board_flag(struct deck_s *deck, int f, int exclude);
void card_apply_attachments(struct hs_holder_s *p, struct deck_s *deck, struct card_s *card);
void deck_add_attachment(struct deck_s *deck, const char *attachment, enum flags_e f);
void cards_apply_attachment(struct hs_holder_s *p, struct deck_s *deck, struct card_s *attacker, enum flags_e f);
int card_count_attachment(struct deck_s *deck, struct deck_s *opponent, struct card_s *card, const char *name);
void add_single_attachment(struct hs_holder_s *p, struct deck_s *deck, struct card_s *attacker, struct card_s *defender);
void card_remove_attachment(struct deck_s *deck, struct deck_s *opponent, struct card_s *card, const char *name);
struct card_s *card_get_board_zoneposition(struct deck_s *deck, u64 position);
void card_apply_attachment(struct hs_holder_s *p, struct deck_s *deck, struct card_s *dst, const char *attachment);
void cards_adjacent(struct hs_holder_s *p, struct deck_s *deck);
int deck_get_taunts(struct deck_s *deck);
void card_discard_by_name(struct hs_holder_s *p, struct card_s *card, const char *name);
struct card_s *card_by_name(struct hs_holder_s *p, struct deck_s *deck, const char *name, int f);
struct card_list_s *cards_get_board_boss(struct deck_s *deck);
void reset_turn_plays(struct deck_s *deck);
void card_free(struct card_s *card, unsigned int flags);
int card_deck_count(struct deck_s *deck);
void deck_free(struct deck_s *deck, unsigned int flags);
struct card_s *card_get_levelup_boss(struct deck_s *deck, struct card_s *card);
int cards_count(struct card_list_s *l);
struct card_s *card_random(struct card_list_s *l);
struct card_s *draw_first_boss(struct deck_s *deck);
struct card_list_s *cards_get_hand(struct deck_s *deck);
int board_count_by_name(struct deck_s *deck, const char *name);
void cards_dump(struct deck_s *deck, struct deck_s *opponent);
struct card_list_s *cards_by_name(struct hs_holder_s *p, struct deck_s *deck, const char *name, int f);

#endif
