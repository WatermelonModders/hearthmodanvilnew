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

void card_to_deck(struct card_s *card)
{
    flag(&card->state, CARD_HAND, FLAG_UNSET);
}

struct card_s *card_get_levelup_boss(struct deck_s *deck, struct card_s *card)
{
    const char *bossname = NULL;
    int i;

    for(i = 0; i < MAX_LEVELUP; i++) {

        hm_log(LOG_DEBUG, lg, "Levelup boss: [%s] [%s]", card->entity->name, levelup[i].src);
        if(strcmp(card->entity->name, levelup[i].src) == 0) {
            bossname = levelup[i].dst;
            break;
        }
    }

    if(bossname == NULL) {
        return NULL;
    }

    for(i = 0; deck != NULL && i < deck->ncards; i++) {
        if((flag(&deck->cards[i]->state, CARD_BOARD, FLAG_ISSET) &&
                    !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET)) &&
                flag(&deck->cards[i]->state, CARD_BOSS, FLAG_ISSET)) {
            return deck->cards[i];
        }
    }

    return NULL;
}

struct card_s *card_get_hero(struct deck_s *deck)
{
    int i;

    for(i = 0; deck != NULL && i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_HERO, FLAG_ISSET)) {
            return deck->cards[i];
        }
    }

    return NULL;
}


struct card_s *card_get(struct deck_s *deck, struct deck_s *opponent, const int id)
{
    int i;


#define FCARD(m__d)\
    for(i = 0; m__d != NULL && i < m__d->ncards; i++) {\
        if(id == m__d->cards[i]->id) {\
            hm_log(LOG_DEBUG, lg, "card [%s](%d) found in deck %d", m__d->cards[i]->entity->desc, m__d->cards[i]->id, m__d->controller);\
            assert(m__d->cards[i]->parent);\
            return m__d->cards[i];\
        }\
    }

    FCARD(deck)
        FCARD(opponent)

        hm_log(LOG_DEBUG, lg, "card (%d) not found in any deck (%d %d)", id, (deck ? deck->ncards : -1), (opponent ? opponent->ncards : -1));\
        return NULL;
}

struct card_list_s *cards_get_board_boss(struct deck_s *deck)
{
    int i;
    struct card_list_s *list = NULL, *item;

    for(i = 0; deck != NULL && i < deck->ncards; i++) {
        if((flag(&deck->cards[i]->state, CARD_BOARD, FLAG_ISSET) &&
                    !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET)) &&
                flag(&deck->cards[i]->state, CARD_BOSS, FLAG_ISSET)) {
            LIST_ADD(list, item, deck->cards[i]);
        }
    }

    return list;
}

struct card_list_s *cards_get_hand(struct deck_s *deck)
{
    int i;
    struct card_list_s *list = NULL, *item;

    for(i = 0; deck != NULL && i < deck->ncards; i++) {
        if((flag(&deck->cards[i]->state, CARD_HAND, FLAG_ISSET) &&
                    !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET))) {
            LIST_ADD(list, item, deck->cards[i]);
        }
    }

    return list;
}


struct card_list_s *cards_get_board(struct deck_s *deck, struct deck_s *opponent, int hero, int exclude, int magic_immunity, int boss)
{
    int i;
    struct card_list_s *list = NULL, *item;

#define CBOARD(d)\
    for(i = 0; d != NULL && i < d->ncards; i++) {\
        if(d->cards[i]->id == exclude) continue;\
        if((flag(&d->cards[i]->state, CARD_DIVINE_SHIELD, FLAG_ISSET)) && magic_immunity == 0) continue;\
        if((flag(&d->cards[i]->state, CARD_BOSS, FLAG_ISSET)) && boss == 0) continue;\
        if((flag(&d->cards[i]->state, CARD_BOARD, FLAG_ISSET) && !flag(&d->cards[i]->state, CARD_DESTROYED, FLAG_ISSET) && !flag(&d->cards[i]->state, CARD_HEROWEAPON, FLAG_ISSET)) || \
                (flag(&d->cards[i]->state, CARD_HERO, FLAG_ISSET) && hero == 1)\
          ) {\
            hm_log(LOG_DEBUG, lg, "Card [%s](%d) found in deck %d", d->cards[i]->entity->desc, d->cards[i]->id, d->controller);\
            LIST_ADD(list, item, d->cards[i]);\
        }\
    }

    CBOARD(deck)
        CBOARD(opponent)

        return list;
}

int cards_get_board_count(struct deck_s *deck, struct deck_s *opponent, int hero, int exclude)
{
    int i;
    int total = 0;

#define CBOARD1(d)\
    for(i = 0; d != NULL && i < d->ncards; i++) {\
        if(d->cards[i]->id == exclude) continue;\
        if((flag(&d->cards[i]->state, CARD_BOARD, FLAG_ISSET) && !flag(&d->cards[i]->state, CARD_DESTROYED, FLAG_ISSET)) || \
                (flag(&d->cards[i]->state, CARD_HERO, FLAG_ISSET) && hero == 1)\
          ) {\
            total++;\
            /*hm_log(LOG_DEBUG, lg, "Card [%s](%d) found in deck %d", d->cards[i]->entity->desc, d->cards[i]->id, d->controller);*/\
            \
        }\
    }

    CBOARD1(deck)
        CBOARD1(opponent)

        return total;
}

struct card_s *card_random(struct card_list_s *l)
{
    struct card_list_s *c;
    int count;

    for(count = 0, c = l; c != NULL; c = c->next, count++);

    if(count < 1) {
        return NULL;
    }

    int r = random_number(count);

    for(count = 0, c = l; c != NULL && count != r; c = c->next, count++);

    return c ? c->card : NULL;
}

int cards_count(struct card_list_s *l)
{
    struct card_list_s *c;
    int count;

    for(count = 0, c = l; c != NULL; c = c->next, count++);

    return count;
}

void cards_board_destroyed_reorder(struct deck_s *deck, int position)
{
    int i;

    hm_log(LOG_DEBUG, lg, "Destroying zone position %d from deck %d", position, deck->controller);
    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_BOARD, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_SPELL, FLAG_ISSET) &&
                deck->cards[i]->zone_position > position) {
            --(deck->cards[i]->zone_position);
            hm_log(LOG_DEBUG, lg, "Decreased board zone position %d for card: %d", deck->cards[i]->zone_position, deck->cards[i]->id);
        }
    }
}

void card_free(struct card_s *card, unsigned int flags)
{

    struct card_link_s *children, *del;
    struct card_list_s *t, *tdel;
    struct card_expire_s *ce, *ced;

    if(EQFLAG(flags, FREE_CARD_CHILDREN)) {
        for(children = card->attached_children; children != NULL; ) {
            del = children;
            children = children->next;
            free(del);
        }
    }

    if(EQFLAG(flags, FREE_CARD_EXPIRE)) {
        for(ce = card->expire; ce != NULL; ) {
            ced = ce;
            ce = ce->next;
            free(ced);
        }
    }

    for(t = card->target.card; t != NULL; ) {
        tdel = t;
        t = t->next;
        free(tdel);
    }

    free(card);
}

int deck_get_taunts(struct deck_s *deck)
{
    int i, c = 0;

    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_TAUNT, FLAG_ISSET) && flag(&deck->cards[i]->state, CARD_BOARD, FLAG_ISSET)) {
            c++;
        }
    }

    hm_log(LOG_DEBUG, lg, "Deck %d taunt cards: %d", deck->controller, c);

    return c;
}

void cards_unset_flag(struct deck_s *deck, enum flags_e flags)
{
    int i;

    for(i = 0; i < deck->ncards; i++) {
        flag(&deck->cards[i]->state, flags, FLAG_UNSET);
    }
}


void reset_turn_plays(struct deck_s *deck)
{
    int i;

    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_BOARD, FLAG_ISSET)) {
            deck->cards[i]->turn_plays = 0;
        }
    }
}

void unset_mechanics_flags(struct hs_holder_s *p)
{
    struct deck_s *deck;
    int i;

#define clr\
    for(i = 0; i < deck->ncards; i++) {\
        flag(&deck->cards[i]->state, MECHANICS_ATTACK, FLAG_UNSET);\
        flag(&deck->cards[i]->state, MECHANICS_DAMAGE, FLAG_UNSET);\
        flag(&deck->cards[i]->state, MECHANICS_HEALTH, FLAG_UNSET);\
        flag(&deck->cards[i]->state, MECHANICS_ATTACK_HEAL, FLAG_UNSET);\
        flag(&deck->cards[i]->state, MECHANICS_ATTACK_DAMAGE, FLAG_UNSET);\
        flag(&deck->cards[i]->state, MECHANICS_HEROPOWER, FLAG_UNSET);\
        flag(&deck->cards[i]->state, MECHANICS_BATTLECRY_TRIGGER, FLAG_UNSET);\
        flag(&deck->cards[i]->state, MECHANICS_CHANGESIDES, FLAG_UNSET);\
        flag(&deck->cards[i]->state, MECHANICS_ARMOR, FLAG_UNSET);\
        flag(&deck->cards[i]->state, MECHANICS_DURABILITY, FLAG_UNSET);\
        flag(&deck->cards[i]->state, MECHANICS_DETACH, FLAG_UNSET);\
        flag(&deck->cards[i]->state, MECHANICS_SELECTED, FLAG_UNSET);\
        flag(&deck->cards[i]->state, MECHANICS_DESTROY_WEAPON, FLAG_UNSET);\
        flag(&deck->cards[i]->state, MECHANICS_TURN_TRIGGER, FLAG_UNSET);\
        flag(&deck->cards[i]->state, MECHANICS_COST, FLAG_UNSET);\
    }

    deck = p->deck;
    clr
        deck = p->opponent.deck;
    clr
}

int zone_position_last(struct deck_s *deck)
{
    int i, position = 0;

    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_BOARD, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET) &&
                deck->cards[i]->zone_position > position) {
            position = deck->cards[i]->zone_position;
        }
    }

    return ++position;
}

int cards_get_board_count_flag(struct deck_s *deck, struct card_s *card, int f)
{
    int i;
    int count = 0;

    for(i = 0; i < deck->ncards; i++) {
        if(deck->cards[i]->id == card->id) {
            continue;
        }

        if(flag(&deck->cards[i]->state, CARD_BOARD, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET) &&
                flag(&deck->cards[i]->state, f, FLAG_ISSET)) {
            count++;
        }
    }

    hm_log(LOG_DEBUG, lg, "Found %d cards for flag %d", count, f);

    return count;
}

int cards_board_last(struct deck_s *deck)
{
    int i;
    int position = 1;

    for(i = 0; i < deck->ncards; i++) {
        /*
           if(deck->cards[i]->id == card->id) {
           continue;
           }
           */

        if(flag(&deck->cards[i]->state, CARD_BOARD, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_SPELL, FLAG_ISSET) &&
                deck->cards[i]->zone_position >= position) {

            hm_log(LOG_DEBUG, lg, "Zone position increased with card %d position %d", deck->cards[i]->id, deck->cards[i]->zone_position);
            position = deck->cards[i]->zone_position + 1;
        }
    }

    return position;
}

static void add_child_link(struct card_s *dst, int id, const char *name)
{
    struct card_link_s *a;

    a = malloc(sizeof(*a));
    a->id = id;
    a->name = name;
    a->next = dst->attached_children;
    dst->attached_children = a;
}

void cards_dump(struct deck_s *deck, struct deck_s *opponent)
{
    int i;

    hm_log(LOG_DEBUG, lg, "Cards dump started");

#define dump(d)\
    hm_log(LOG_DEBUG, lg, "Cards dump for deck %d:", d->controller);\
    for(i = 0; i < d->ncards; i++) {\
        if(flag(&d->cards[i]->state, CARD_BOARD, FLAG_ISSET) || \
                flag(&d->cards[i]->state, CARD_HAND, FLAG_ISSET) \
          ) {\
            hm_log(LOG_DEBUG, lg, "\t\tName[%s](%d) zone position[%d] attack[%d] health[%d] totalhealth[%d]", d->cards[i]->entity->desc, d->cards[i]->id, d->cards[i]->zone_position, d->cards[i]->attack, d->cards[i]->health, d->cards[i]->total_health);\
            flag(&d->cards[i]->state, -1, FLAG_DUMP);\
        }\
    }

    dump(deck)
        dump(opponent)

        hm_log(LOG_DEBUG, lg, "Cards dump finished");
}

void cards_reorder_board_owner(struct deck_s *deck, u64 held, u64 position)
{
    int i;

    if(position - 1 >= MAX_BOARD) {
        position = MAX_BOARD;
    }

    for(i = 0; i < deck->ncards; i++) {
        if(deck->cards[i]->id == held) {

            deck->cards[i]->zone_position = position;
            deck->board_position[position - 1] = deck->cards[i]->id;

            continue;
        }

        // when card on board is positioned equally or higher than new card
        if(flag(&deck->cards[i]->state, CARD_BOARD, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_SPELL, FLAG_ISSET) &&
                deck->cards[i]->zone_position >= position) {
            ++(deck->cards[i]->zone_position);

            deck->board_position[ (deck->cards[i]->zone_position - 1) ] = deck->cards[i]->id;
        }
    }
}

void cards_reorder_hand_owner(struct deck_s *deck, u64 id)
{
    int i, pos = 0;
    struct card_s *card;

    card = card_get(deck, NULL, id);

    pos = card->zone_position - 1;	

    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_HAND, FLAG_ISSET) && deck->cards[i]->id != id) {
            if(deck->cards[i]->zone_position > card->zone_position) {
                hm_log(LOG_DEBUG, lg, "Hand card position %d board card position %d", deck->cards[i]->zone_position, card->zone_position);
                deck->cards[i]->zone_position--;
                if(deck->cards[i]->zone_position > pos) {
                    pos = deck->cards[i]->zone_position;
                }
            }
        }
    }

    card->zone_position = 0;

    hm_log(LOG_DEBUG, lg, "Reordering zone positions %d", pos);
    deck->handposition = pos;
}

int cards_get_board_flag(struct deck_s *deck, int f, int exclude)
{
    int i;
    int count = 0;

    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_BOARD, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET) &&
                flag(&deck->cards[i]->state, f, FLAG_ISSET) &&
                deck->cards[i]->id != exclude )
        {
            count++;
        }
    }

    hm_log(LOG_DEBUG, lg, "Found %d entities of flag %d while excluded %d", count, f, exclude);

    return count;
}

struct card_s *card_get_board_zoneposition(struct deck_s *deck, u64 position)
{
    int i;

    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_BOARD, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET) &&
                deck->cards[i]->zone_position == position)
        {

            hm_log(LOG_DEBUG, lg, "Card zone positon %lld card [%s](%d) found", position, deck->cards[i]->entity->desc, deck->cards[i]->id);
            return deck->cards[i];
        }
    }

    return NULL;
}

void card_discard_by_name(struct hs_holder_s *p, struct card_s *card, const char *name)
{
    int i;

    struct deck_s *deck;


    hm_log(LOG_DEBUG, lg, "Discarding attacker's [%s](%d) associates: name %s", card->entity->desc, card->id, name);

    deck = card->controller == p->deck_copy->controller ? p->deck_copy : p->opponent.deck_copy;

    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_HAND, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET) &&
                strcmp(name, deck->cards[i]->entity->name) == 0) {

            flag(&deck->cards[i]->state, CARD_DISCARD, FLAG_SET);\
                cards_reorder_hand_owner(deck, deck->cards[i]->id);\

        }
    }
}

int board_count_by_name(struct deck_s *deck, const char *name)
{
    int i, count = 0;

    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_BOARD, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET) &&
                strcmp(name, deck->cards[i]->entity->name) == 0) {
            count++;
        }
    }

    return count;
}

struct card_list_s *cards_by_name(struct hs_holder_s *p, struct deck_s *deck, const char *name, int f)
{
    int i;
    struct card_list_s *list = NULL, *item;

    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, f, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET) &&
                strcmp(name, deck->cards[i]->entity->name) == 0) {
            LIST_ADD(list, item, deck->cards[i]);
        }
    }

    return list;
}


struct card_s *card_by_name(struct hs_holder_s *p, struct deck_s *deck, const char *name, int f)
{
    int i;

    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, f, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET) &&
                strcmp(name, deck->cards[i]->entity->name) == 0) {
            return deck->cards[i];
        }
    }

    return NULL;
}


struct card_list_s *cards_get_random(struct deck_s *deck, int number, struct card_s *attacker, int zone)
{
    int count = 0, r, r1, i, total;
    struct card_list_s *list = NULL, *item, *i1;
    struct card_list_s *output = NULL;

    hm_log(LOG_DEBUG, lg, "Getting %d random cards", number);

    for(i = 0; i < deck->ncards; i++) {


        if( (zone == 1 &&
                    (
                     (
                      flag(&deck->cards[i]->state, CARD_BOARD, FLAG_ISSET) &&
                      !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET)
                     ) ||
                     flag(&deck->cards[i]->state, CARD_HERO, FLAG_ISSET)
                    )
            )

                ||

                (zone == 6 &&
                 (
                  flag(&deck->cards[i]->state, CARD_DECK, FLAG_ISSET) &&
                  !flag(&deck->cards[i]->state, CARD_BOARD, FLAG_ISSET) &&
                  !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET)
                 )
                )
          ) {
            LIST_ADD(list, item, deck->cards[i]);
            count++;
        }
    }

    assert(count > 0);

    if(count <= number && !flag(&attacker->state, CARD_ARCANE_MISSILES, FLAG_ISSET)) {
        return list;
    }

    total = 0;
    r1 = -1;

again:
    r = random_number(count);

    // arcane missiles can hit the same target multiple times
    if(r1 == r && !flag(&attacker->state, CARD_ARCANE_MISSILES, FLAG_ISSET)) goto again;

    for(i = 0, item = list; item != NULL; item = item->next, i++) {
        if(i == r) {
            hm_log(LOG_DEBUG, lg, "Random card [%s](%d) added", item->card->entity->desc, item->card->id);
            LIST_ADD(output, i1, item->card);
            r1 = r;
            if(number == ++total) break;
            else goto again;
        }
    }

    // TODO: free list

    return output;
}

struct card_s *cards_get_hand_random(struct deck_s *deck)
{
    int count = 0, r, i, count_all;
    struct card_list_s *list = NULL, *item;

    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_HAND, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_DESTROYED, FLAG_ISSET) &&
                !flag(&deck->cards[i]->state, CARD_BOARD, FLAG_ISSET)) {
            LIST_ADD(list, item, deck->cards[i]);

            hm_log(LOG_DEBUG, lg, "\tGetting random inhand card %d", deck->cards[i]->id);
            count++;
        }
    }

    hm_log(LOG_DEBUG, lg, "Getting random card from total count of %d", count);

    if(count == 0) {
        return NULL;
    }

    count_all = count;
    r = random_number(count);

    count = 0;
    for(item = list; item != NULL; item = item->next, count++) {
        if(count == r) {
            hm_log(LOG_DEBUG, lg, "Returning random card [%s](%d) from %d cards", item->card->entity->desc, item->card->id, count_all);
            return item->card;
        }
    }

    return NULL;
}

int card_deck_count(struct deck_s *deck)
{
    int count = 0, i;

    for(i = 0; i < deck->ncards; i++) {
        assert(deck);
        assert(deck->cards);
        assert(deck->cards[i]);
        if(flag(&deck->cards[i]->state, CARD_DECK, FLAG_ISSET)
          ) {
            count++;
        }
    }

    hm_log(LOG_DEBUG, lg, "Found %d cards in deck %d", count, deck->controller);

    return count;
}

struct card_s *draw_first_boss(struct deck_s *deck)
{
    int i, r, count = 0;

    r = random_number(MAX_BOSS);

    for(i = 0; i < deck->ncards; i++) {
        if(flag(&deck->cards[i]->state, CARD_BOSS, FLAG_ISSET)) {
            if((count++) == r) {
                hm_log(LOG_DEBUG, lg, "Start Hand Random Boss %d", r);
                flag(&deck->cards[i]->state, CARD_DECK, FLAG_UNSET);
                flag(&deck->cards[i]->state, CARD_HAND, FLAG_SET);
                deck->cards[i]->zone_position = ++deck->handposition;
                deck->cards[i]->cost = 0;   // first boss costs 0
                return deck->cards[i];
            }
        }
    }

    return NULL;
}

struct card_s *draw_random_card(struct deck_s *deck, int entity_id, int zone_position)
{
    int r;

    if(card_deck_count(deck) == 0) {
        return NULL;
    }

again:

    r = random_number(deck->ncards);

    // exclude drawn, board and mana card
    if(!flag(&deck->cards[r]->state, CARD_DECK, FLAG_ISSET) ||
            deck->cards[r]->id == 68) {
        goto again;
    }

    flag(&deck->cards[r]->state, CARD_DECK, FLAG_UNSET);
    flag(&deck->cards[r]->state, CARD_HAND, FLAG_SET);

    deck->cards[r]->zone_position = zone_position >= 1 ? zone_position : ++deck->handposition;

    hm_log(LOG_DEBUG, lg, "Drawn random card %d [%s], hand zone position %d player entity: %d", deck->cards[r]->id, deck->cards[r]->entity->desc, deck->handposition, entity_id);
    return deck->cards[r];
}

struct card_s *new_card(const int id, const char *name, int controller)
{
    struct card_s *card;
    int i;
#ifdef SPELL_ZERO_MANA
    int cardtype = 0;
#endif

    card = malloc(sizeof(*card));
    memset(card, 0, sizeof(*card));

    card->id = id;
    card->entity = find_entity(name, strlen(name));
    assert(card->entity);	

    for(i = 0; i < card->entity->nprop; i++) {
        if(card->entity->p[i].key == HEALTH) {
            card->health = card->total_health = card->default_health = card->entity->p[i].value;
        } else if(card->entity->p[i].key == COST) {
            card->cost = card->default_cost = card->entity->p[i].value;
        } else if(card->entity->p[i].key == ATK) {
            card->attack = card->default_attack = card->entity->p[i].value;
        } else if(card->entity->p[i].key == CARDTYPE) {
#ifdef SPELL_ZERO_MANA
            card->cardtype = card->entity->p[i].value;
            cardtype = card->entity->p[i].value;
#endif
        } else if(card->entity->p[i].key == DURABILITY) {
            card->durability = card->maxdurability = card->entity->p[i].value;
        } else if(card->entity->p[i].key == SPELLPOWER) {
            card->spellpower = card->entity->p[i].value;
        }
    }

#ifdef MANA_ZERO
    card->cost = 0;
#endif

#ifdef SPELL_ZERO_MANA
    if(cardtype == 5) {
        card->cost = 0;
    }
#endif

    if(card->entity->attachment) {
        card->attachment = card->entity->attachment;
    }

    card->controller = controller;

    flags_copy(&card->state, &card->entity->flags);

    return card;
}

void deck_add_attachment(struct deck_s *deck, const char *name, enum flags_e f)
{
    struct attachment_list_s *a;

    abort();

    a = malloc(sizeof(*a));
    a->name = name;
    a->group = f;

    a->next = deck->attachments;
    deck->attachments = a;
}

int card_count_attachment(struct deck_s *deck, struct deck_s *opponent, struct card_s *card, const char *name)
{
    struct card_link_s *child;
    struct card_s *child_card;
    int count;

    for(count = 0, child = card->attached_children; child != NULL; child = child->next, count++);

    hm_log(LOG_DEBUG, lg, "Total attachments on card [%s](%d) is %d", card->entity->desc, card->id, count);

    for(count = 0, child = card->attached_children; child != NULL; child = child->next) {
        child_card = card_get(deck, opponent, child->id);

        assert(child_card);
        if(strcmp(child_card->entity->name, name) == 0) {
            count++;
        } else {
            hm_log(LOG_DEBUG, lg, "Comparisson [%s] to [%s] failed", child_card->entity->name, name);
        }
    }

    hm_log(LOG_DEBUG, lg, "Total of %d attachments %s on card [%s](%d)", count, name, card->entity->name, card->id);

    return count;
}

void card_remove_attachment(struct deck_s *deck, struct deck_s *opponent, struct card_s *card, const char *name)
{
    struct card_link_s *child, *prev_child;
    struct card_s *child_card;

    hm_log(LOG_DEBUG, lg, "Removing attachments on card [%s](%d)", card->entity->name, card->id);

    for(prev_child = NULL, child = card->attached_children; child != NULL; prev_child = child, child = child->next) {
        child_card = card_get(deck, opponent, child->id);

        assert(child_card);
        hm_log(LOG_DEBUG, lg, "\t\t - checking attachment [%s] [%s](%d)", name, child_card->entity->name, child_card->id);
        if(name == NULL || strcmp(child_card->entity->name, name) == 0) {
            if(prev_child == NULL) {
                card->attached_children = child->next;
            } else {
                prev_child->next = child->next;
            }

            flag(&(child_card->state), MECHANICS_DETACH, FLAG_SET);
            flag(&(child_card->state), CARD_DESTROYED, FLAG_SET);

            hm_log(LOG_DEBUG, lg, "Removing attachment [%s](%d) for card [%s](%d)", child_card->entity->desc, child_card->id, card->entity->desc, card->id);

            flag(&(child_card->state), -1, FLAG_DUMP);

            free(child);
            break;
        }
    }
}

void deck_remove_attachment(struct deck_s *deck, struct deck_s *opponent, const char *name)
{
    struct attachment_list_s *a, *prev;
    struct card_list_s *cards, *card;

    cards = cards_get_board(deck, NULL, -1, 0, -1, 1);

    hm_log(LOG_DEBUG, lg, "Starting attachments removal");

    for(prev = NULL, a = deck->attachments; a != NULL; prev = a, a = a->next) {
        if(strcmp(name, a->name) == 0) {
            if(prev == NULL) {
                deck->attachments = a->next;
            } else {
                prev->next = a->next;
            }

            hm_log(LOG_DEBUG, lg, "Removing attachment [%s] from deck %p", a->name, deck);
            free(a);
            break;
        }
    }

    for(card = cards; card != NULL; card = card->next) {
        card_remove_attachment(deck, opponent, card->card, name);
    }
}

void add_single_attachment(struct hs_holder_s *p, struct deck_s *deck, struct card_s *attacker, struct card_s *defender)
{
    if(attacker && attacker->attachment) {
        // FIXME: deck might be opponent's too
        struct card_s *att = add_new_card(p->game, deck, -1, attacker->attachment, -1);
        att->attached_to = defender->id;

        add_child_link(defender, att->id, att->entity->name);

        hm_log(LOG_DEBUG, lg, "Adding attachment [%s](%d) to card [%s]%d", att->entity->desc, att->id, defender->entity->desc, defender->id);
    } else {
        hm_log(LOG_ALERT, lg, "Attacker [%s](%d) has no attachment", attacker->entity->desc, attacker->id);
        abort();
    }
}

void card_apply_attachment(struct hs_holder_s *p, struct deck_s *deck, struct card_s *dst, const char *attachment)
{
    struct card_s *card;

    card = add_new_card(p->game, deck, -1, attachment, -1);
    card->attached_to = dst->id;

    add_child_link(dst, card->id, card->entity->name);

    hm_log(LOG_DEBUG, lg, "Adding attachment [%s](%d) to card [%s]%d", card->entity->desc, card->id, dst->entity->desc, dst->id);
}

void card_apply_attachments(struct hs_holder_s *p, struct deck_s *deck, struct card_s *dst)
{
    struct attachment_list_s *c;

    assert(deck);
    assert(dst);

    // applying current attachments to card
    for(c = deck->attachments; c != NULL; c = c->next) {
        //cards_dump_board(deck);
        abort();
        if(c->group > 0 && !flag(&dst->state, c->group, FLAG_ISSET)) {
            continue;
        }

        card_apply_attachment(p, deck, dst, c->name);
    }
}

void cards_apply_attachment(struct hs_holder_s *p, struct deck_s *deck, struct card_s *attacker, enum flags_e f)
{
    struct card_list_s *c, *cards;
    struct card_s *att;

    cards = cards_get_board(deck, NULL, -1, attacker->id, -1, 1);

    for(c = cards; c != NULL; c = c->next) {
        if(f > 0 && !flag(&c->card->state, f, FLAG_ISSET)) {
            continue;
        }

        att = add_new_card(p->game, deck, -1, attacker->attachment, -1);
        att->attached_to = c->card->id;

        add_child_link(c->card, att->id, att->entity->name);

        hm_log(LOG_DEBUG, lg, "Adding attachment [%s](%d) to card [%s]%d", att->entity->desc, att->id, c->card->entity->desc, c->card->id);
    }
}

void card_destroy(struct card_s *card, struct deck_s *deck, int to_hand)
{
    struct hs_holder_s *p = deck->parent;
    struct conn_client_s *c = p->client;

    if(flag(&card->state, CARD_BOARD, FLAG_ISSET)) {
        flag(&card->state, CARD_BOARD, FLAG_UNSET);
    }

    if(flag(&card->state, CARD_SPELLPOWER, FLAG_ISSET)) {
        //deck->spellpower -= card->spellpower;
        deck->spellpower--;
    }

    if(!to_hand) {
        flag(&card->state, CARD_DESTROYED, FLAG_SET);
    }

    cards_board_destroyed_reorder(deck, card->zone_position);

    {
        struct card_s *__fake_defender = card;
        __SECTION_DESTROYED
    }
}

void card_dump(struct card_s *card)
{
    hm_log(LOG_DEBUG, lg, "Dumping card %d:\
            \n\tParent: %p\n\
            \tController: %d\n\
            \tID: %d\n\
            \tHealth: %d\n", card->id, card->parent, card->controller, card->id, card->health);\
}

