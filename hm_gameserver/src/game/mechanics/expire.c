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

void mechanics_expire(int game_turn, struct card_s *card, enum expire_e key, int value, const char *attachment)
{
    struct card_expire_s *ce;

    ce = malloc(sizeof(*ce));
    memset(ce, 0, sizeof(*ce));

    ce->turn = game_turn;

    switch(key) {
        case EXPIRE_ATTACK:
            ce->attack = value;
            break;

        case EXPIRE_FROZEN:
            ce->frozen = value;
            break;

        case EXPIRE_MAGIC_IMMUNITY:
            ce->magic_immunity = value;
            break;

        case EXPIRE_DESTROY:
            ce->destroy = value;
            break;

        case EXPIRE_ATTACHMENT:
            ce->attachment = attachment;
            break;

        default:
            abort();
            break;
    }

    ce->next = card->expire;
    card->expire = ce;
}

static void expiration_list(struct hs_holder_s *p, struct deck_s *deck, struct deck_s *deck_opponent, struct card_s *card, int *unfreeze, int *mimmunity)
{
    struct card_expire_s *ce;

    for(ce = card->expire; ce != NULL; ce = ce->next) {

        if(ce->frozen > 0 && *unfreeze < ce->turn) {
            *unfreeze = ce->turn;       // get the highest unfreeze turn
        }

        if(ce->magic_immunity > 0 && *mimmunity < ce->turn) {
            *mimmunity = ce->turn;       // get the highest magic immunity turn
        }

        if(ce->turn == p->game->turn) {
            if(ce->attack != 0) {
                card->attack += ce->attack;
                hm_log(LOG_DEBUG, lg, "%s Changed attack of card %d to %d", LOG_MECHANICS, card->id, card->attack);
                flag(&card->state, MECHANICS_ATTACK, FLAG_SET);
            } else if(ce->destroy == 1) {
                hm_log(LOG_DEBUG, lg, "%s Card %d set to be destroyed", LOG_MECHANICS, card->id);
                flag(&card->state, CARD_DESTROYED, FLAG_SET);
            }

            if(ce->attachment != NULL) {
                card_remove_attachment(deck, p->opponent.deck_copy, card, ce->attachment);
                ce->attachment = NULL;
            }
        }
    }
}

void mechanics_expired(struct hs_holder_s *p, struct deck_s *deck)
{
    int i;
    int unfreeze_turn, magic_immunity_turn;

    for(i = 0; i < deck->ncards; i++) {

        unfreeze_turn = magic_immunity_turn = 0;

        expiration_list(p, deck, p->opponent.deck_copy, deck->cards[i], &unfreeze_turn, &magic_immunity_turn);

        if(p->game->turn == unfreeze_turn) {
            hm_log(LOG_DEBUG, lg, "%s Unfreezing card: %d", LOG_MECHANICS, deck->cards[i]->id);
            flag(&deck->cards[i]->state, CARD_FROZEN, FLAG_UNSET);
        }

        if(p->game->turn == magic_immunity_turn) {
            hm_log(LOG_DEBUG, lg, "%s Removing magic immunity from card: %d", LOG_MECHANICS, deck->cards[i]->id);
            flag(&deck->cards[i]->state, CARD_DIVINE_SHIELD, FLAG_UNSET);
        }
    }
}

void force_expire(struct hs_holder_s *p, struct deck_s *deck, struct card_s *card)
{
    struct card_expire_s *ce;
    struct deck_s *deck_opponent = p->opponent.deck_copy;

#define ATT\
    if(ce->attachment != NULL) {\
        card_remove_attachment(deck, deck_opponent, card, ce->attachment);\
    }

    for(ce = card->expire; ce != NULL; ce = ce->next) {

        if(ce->frozen == 1) {
            ATT
                flag(&card->state, CARD_FROZEN, FLAG_UNSET);
        }
    }
}
