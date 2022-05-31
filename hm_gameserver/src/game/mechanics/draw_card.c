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

void mechanics_draw_card(struct conn_client_s *c, struct chooseoption_s *opt, struct card_s *attacker, int cards, struct card_list_s **defenders)
{
    struct hs_holder_s *p;
    struct deck_s *deck;
    int i;
    struct card_s *card;
    struct card_list_s *clitem;

    p = c->data;
    deck = p->deck_copy;

    assert(deck);

    hm_log(LOG_DEBUG, c->log, "%s Drawing %d cards:", LOG_MECHANICS, cards);
    for(i = 0; i < cards; i++) {
        card = draw_random_card(deck, p->entity_id, -1);
        flag(&card->state, CARD_HAND, FLAG_SET);

        LIST_ADD(*defenders, clitem, card);

        hm_log(LOG_DEBUG, c->log, "%s Drawing card %d from deck %d", LOG_MECHANICS, card->id, card->parent->deck_copy->controller);
    }

    if(flag(&attacker->state, CARD_SPELL, FLAG_ISSET) && !flag(&attacker->state, CARD_HEROPOWER, FLAG_ISSET)) {
        flag(&attacker->state, CARD_DESTROYED, FLAG_SET);
        cards_reorder(p, deck, attacker, opt->position, 0);
    }
}

