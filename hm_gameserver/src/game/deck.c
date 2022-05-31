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

#define TEST_DECK_MAX	2
const char *test_deck[TEST_DECK_MAX] = { "EX1_581", "EX1_306" };

struct card_s *add_new_card(struct game_s *game, struct deck_s *deck, int index, const char *card, enum flags_e flags)
{
    int n;
    struct card_s *c;

    n = ++(deck->ncards);

    deck->cards = realloc(deck->cards, sizeof(void *) * deck->ncards);

    game->maxentity_id++;

    if(index == -1) {
        c = new_card(game->maxentity_id, card, deck->controller);
    } else {
        c = new_card(index, card, deck->controller);
    }

    if(flags != -1) {
        //c->state |= flags;
        flag(&c->state, flags, FLAG_SET);
    }

    assert(deck);
    assert(deck->parent);
    c->parent = deck->parent;

    deck->cards[n - 1] = c;

    hm_log(LOG_DEBUG, lg, "Added new card to deck: %p controller: %d card: %d properties: %d total cards: %d game maxentity_id: %d", deck, deck->controller, c->id, c->entity->nprop, deck->ncards, game->maxentity_id);

    return c;
}

static struct deck_s *create_deck(struct conn_client_s *c, const int controller, int *counter)
{
    struct deck_s *deck;
    int i, j;
    struct hs_holder_s *p;
    struct game_s *game;

    p = c->data;
    game = p->game;

    deck = malloc(sizeof(*deck));
    if(deck == NULL) {
        return NULL;
    }
    memset(deck, 0, sizeof(*deck));

    deck->controller = controller;
    assert(p);
    deck->parent = p;

#ifdef TEST_DECK
#define DEFAULT_DECK_MAX    30

    int id;

    deck->ncards = DEFAULT_DECK_MAX;

    deck->cards = malloc(sizeof(void *) * deck->ncards);

    for(j = 0, i = 0, j = 0; i < deck->ncards; i++, j++ ) {
        if(controller == 1) {
            id = i + 4;
        } else {
            id = i + 4 + DEFAULT_DECK_MAX;
        }

        if(j == TEST_DECK_MAX) j = 0;

        //j = (controller == 1 ? 0 : 1);
        deck->cards[i] = new_card(id, test_deck[j], controller);
        assert(deck);
        deck->cards[i]->parent = deck->parent;

        //deck->cards[i]->state |= CARD_DECK;
        flag(&deck->cards[i]->state, CARD_DECK, FLAG_SET);
        game->maxentity_id++;
    }
#else
    struct player_deck_s *pdc;

    deck->ncards = c->ncards;

    deck->cards = malloc(sizeof(void *) * deck->ncards);

    for(j = 0, pdc = c->cards; pdc != NULL; pdc = pdc->next) {
        for(i = 0; i < pdc->count; i++) {
            deck->cards[j] = new_card((*counter)++, pdc->code, controller);
            deck->cards[j]->parent = deck->parent;

            flag(&deck->cards[j]->state, CARD_DECK, FLAG_SET);

            hm_log(LOG_DEBUG, lg, "Added card (%d)[%s]", deck->cards[j]->id, deck->cards[j]->entity->desc);

            game->maxentity_id++;
            j++;
        }
    }

    assert(j == deck->ncards);
#endif

    hm_log(LOG_DEBUG, lg, "Created deck: %p controller: %d ncards: %d", deck, controller, deck->ncards);

    return deck;
}

void prepare_decks(struct conn_client_s *c)
{
    struct hs_holder_s *p1;
    struct deck_s *p2_deck;
    int i;

    p1 = c->data;

    assert(p1);

    if(p1->opponent.accessible == 1) {
        p2_deck = p1->opponent.deck;
    }

    assert(p2_deck);

    if(EQFLAG(p1->deck->flags, DECK_INIT)) {
        return;
    }

    p1->deck->flags |= DECK_INIT;
    p2_deck->flags |= DECK_INIT;


    // randomly select 4 cards for p1 - 2nd turn
    // randomly select 3 cards for p2 - 1st turn

    // put one random boss
    draw_first_boss(p1->deck);

    for(i = 0; i < 3; i++) {
        draw_random_card(p1->deck, p1->entity_id, -1);
    }

    for(i = 0; i < p1->deck->ncards; i++) {
        if(p1->deck->cards[i]->entity->nname == strlen("GAME_005") &&
                memcmp(p1->deck->cards[i]->entity->name, "GAME_005", strlen("GAME_005")) == 0) {
            flag(&p1->deck->cards[i]->state, CARD_HAND, FLAG_SET);

            p1->deck->cards[i]->zone_position = ++p1->deck->handposition;
        }
    }

    hm_log(LOG_DEBUG, lg, "P1 deck default zone position: %d", p1->deck->handposition);

    // put one random boss
    draw_first_boss(p2_deck);

    for(i = 0; i < 2; i++) {
        draw_random_card(p2_deck, 3, -1);
    }

    hm_log(LOG_DEBUG, lg, "P2 deck default zone position: %d", p2_deck->handposition);
}

void holder_free(struct hs_holder_s *h)
{
    deck_free(h->deck, (FREE_DECK_ATTACHMENT | FREE_CARD_CHILDREN | FREE_CARD_EXPIRE));
    deck_free(h->deck_copy, (FREE_DECK_ATTACHMENT | FREE_CARD_CHILDREN | FREE_CARD_EXPIRE));

    free(h);
}

int append_decks(struct conn_client_s *p1, struct conn_client_s *p2)
{
    struct hs_holder_s *p1_holder, *p2_holder;
    struct game_s *g;

    p1_holder = malloc(sizeof(*p1));
    p2_holder = malloc(sizeof(*p2));

    memset(p1_holder, 0, sizeof(*p1_holder));
    memset(p2_holder, 0, sizeof(*p2_holder));

    p1->data = p1_holder;
    p2->data = p2_holder;

    p1_holder->client = p1;
    p2_holder->client = p2;

    g = malloc(sizeof(*g));

    memset(g, 0, sizeof(*g));

    g->entity_id = 1;
    g->flags = 0;
    g->turn = 1;
    g->maxentity_id = 3;        ///< start with offset 4, as 1 - game entity, 2/3 - players

    p1_holder->game = p2_holder->game = g;

    assert(p1_holder->game && p2_holder->game);

    int counter = 4;
    p1_holder->deck = create_deck(p1, 1, &counter);
    p2_holder->deck = create_deck(p2, 2, &counter);

    p1_holder->opponent.accessible = 1;
    p2_holder->opponent.accessible = 1;

    p1_holder->opponent.deck = p2_holder->deck;
    p2_holder->opponent.deck = p1_holder->deck;

    p1_holder->deck->bnet_hi = 0x4ab3967;
    p2_holder->deck->bnet_hi = 0x49E8Eb5;

    p1_holder->id = 1;
    p2_holder->id = 2;

    p1_holder->entity_id = 2;
    p2_holder->entity_id = 3;

    p1_holder->opponent.client = p2;
    p2_holder->opponent.client = p1;

    p1_holder->deck->mana = p2_holder->deck->mana = 1;

    p1_holder->deck->hero = add_new_card(p1_holder->game, p1_holder->deck, -1, "TB_SPT_Boss" /*p1->hero*/, -1);
    p1_holder->deck->heropower = add_new_card(p1_holder->game, p1_holder->deck, -1, p1->hp, -1);

    p2_holder->deck->hero = add_new_card(p1_holder->game, p2_holder->deck, -1, "LOEA04_01"/*p2->hero*/, -1);
    p2_holder->deck->heropower = add_new_card(p1_holder->game, p2_holder->deck, -1, p2->hp, -1);

    (void)add_new_card(p1_holder->game, p1_holder->deck, -1, "GAME_005", CARD_HAND);

    prepare_decks(p1);

    return 0;
}

void game_free(struct game_s *g)
{
    ev_timer_stop(loop, &g->turntimer);
    ev_timer_stop(loop, &g->force_next_turn);
    free(g);
}

void copy_card(struct card_s **dst, struct card_s *src)
{
    if(src == NULL) {
        *dst = NULL;
        return;
    }

    *dst = malloc(sizeof(**dst));
    memset(*dst, 0, sizeof(**dst));

#define cpyc(m_ele)\
    (*dst)->m_ele = src->m_ele;

    cpyc(parent);
    cpyc(id);
    cpyc(controller);
    memcpy(&(*dst)->state, &src->state, sizeof(src->state));

    cpyc(attachment);
    cpyc(attached_to);
    cpyc(attached_children);

    cpyc(entity);
    cpyc(zone_position);

    cpyc(total_health);
    cpyc(health);
    cpyc(default_health);
    cpyc(buff_health);

    cpyc(attack);
    cpyc(default_attack);

    cpyc(cost);
    cpyc(default_cost);

    cpyc(cardtype);
    cpyc(durability);
    cpyc(maxdurability);
    cpyc(armor);
    cpyc(actions_number);
    cpyc(spellpower);

    // do not copy 'target' and 'receive'

    memcpy(&(*dst)->boss, &src->boss, sizeof(src->boss));

    cpyc(linked);
    cpyc(expire);
    cpyc(total_plays);
    cpyc(turn_plays);
}

void deck_free(struct deck_s *deck, unsigned int flags)
{
    int i;
    struct attachment_list_s *a, *del;

    if(deck == NULL) {
        return;
    }

    if(EQFLAG(flags, FREE_DECK_ATTACHMENT)) {
        for(a = deck->attachments; a != NULL; ) {
            del = a;
            a = a->next;
            free(del);
        }
    }

    for(i = 0; i < deck->ncards; i++) {
        card_free(deck->cards[i], flags);
    }

    free(deck);
}

struct deck_s *copy_deck(struct deck_s *src)
{
    struct deck_s *dst;
    int i;

    dst = malloc(sizeof(*dst));

    dst->hero = dst->heropower = dst->heroweapon = NULL;
#define cpy(m_ele)\
    dst->m_ele = src->m_ele;

    assert(src->parent);
    cpy(parent);
    cpy(bnet_hi);
    cpy(controller);
    cpy(mana);
    cpy(mana_used);
    cpy(spellpower);
    cpy(handposition);
    cpy(ncards);

    memcpy(&dst->modifiers, &src->modifiers, sizeof(src->modifiers));

    dst->cards = malloc(sizeof(void *) * dst->ncards);
    for(i = 0; i < dst->ncards; i++) {

        copy_card(&dst->cards[i], src->cards[i]);

        if(flag(&dst->cards[i]->state, CARD_HERO, FLAG_ISSET)) {
            dst->hero = dst->cards[i];
        } else if(flag(&dst->cards[i]->state, CARD_HEROPOWER, FLAG_ISSET)) {
            dst->heropower = dst->cards[i];
        } else if(flag(&dst->cards[i]->state, CARD_HEROWEAPON, FLAG_ISSET)) {
            dst->heroweapon = dst->cards[i];
        }
    }

    memcpy(&dst->board_position, &src->board_position, sizeof(src->board_position));
    cpy(attachments);

    cpy(fatigue);
    cpy(gameover);
    cpy(flags);

    return dst;
}

void decks_copy(struct hs_holder_s *p)
{
    struct hs_holder_s *opponent;

    opponent = p->opponent.client->data;

    assert(p->deck_copy == NULL);
    assert(p->opponent.deck_copy == NULL);
    assert(opponent->deck_copy == NULL);
    assert(opponent->opponent.deck_copy == NULL);

    ///< make copies of our and opponent's deck
    p->deck_copy = copy_deck(p->deck);
    opponent->opponent.deck_copy = p->deck_copy;

    p->opponent.deck_copy = copy_deck(p->opponent.deck);
    opponent->deck_copy = p->opponent.deck_copy;

    hm_log(LOG_DEBUG, lg, "Deck old %p new %p Opponeng old %p new %p", p->deck, p->deck_copy, p->opponent.deck, p->opponent.deck_copy);

}

void decks_swap(struct hs_holder_s *p)
{
    struct hs_holder_s *o;

    o = p->opponent.client->data;

    hm_log(LOG_DEBUG, lg, "Freeing decks %p %p", p->deck, p->opponent.deck);
    hm_log(LOG_DEBUG, lg, "Decks copies to swap %p %p", p->deck_copy, p->opponent.deck_copy);

    // free old decks
    deck_free(p->deck, 0);
    deck_free(p->opponent.deck, 0);
    o->deck = NULL;
    o->opponent.deck = NULL;

    // swap current player
    p->deck = p->deck_copy;
    p->opponent.deck = p->opponent.deck_copy;
    p->deck_copy = NULL;
    p->opponent.deck_copy = NULL;

    // swap opponent
    o->deck = p->opponent.deck;
    o->opponent.deck = p->deck;
    o->deck_copy = NULL;
    o->opponent.deck_copy = NULL;
}
