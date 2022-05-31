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

void mechanics_put_card_onboard(struct conn_client_s *c, u64 position, struct card_s *attacker, int summoned, struct card_list_s **d, struct chooseoption_s *opt, struct deck_s *deck)
{
    struct hs_holder_s *p;
    struct card_s *card;
    struct card_list_s *defenders = NULL;

    p = c->data;

    assert(deck);

    hm_log(LOG_DEBUG, c->log, "%s Putting card %d onboard summoned %d defenders %p", LOG_MECHANICS, attacker->id, summoned, defenders);
    flag(&attacker->state, CARD_HAND, FLAG_UNSET);

    // if charge or battlecry target
    if(!flag(&attacker->state, CARD_CHARGE, FLAG_ISSET) &&
            !flag(&attacker->state, CARD_BATTLECRY_TARGET, FLAG_ISSET) &&
            !flag(&attacker->state, CARD_WEAPON, FLAG_ISSET)
      ) {
        flag(&attacker->state, CARD_EXHAUSTED, FLAG_SET);
    }

    // spellpower
    if(flag(&attacker->state, CARD_SPELLPOWER, FLAG_ISSET)) {
        deck->spellpower++; //attacker->spellpower;
        hm_log(LOG_DEBUG, c->log, "%s Player %d spellpower %d", LOG_MECHANICS, p->entity_id, deck->spellpower);
    }

    // regular minion or weapon
    if(flag(&attacker->state, CARD_WEAPON, FLAG_ISSET)) {
        __SECTION_WEAPON
    } else {
        flag(&attacker->state, CARD_BOARD, FLAG_SET);
    }

    cards_reorder(p, deck, attacker, position, summoned);

    // battlecries
    if(flag(&attacker->state, CARD_BATTLECRY, FLAG_ISSET) &&
            !flag(&attacker->state, MECHANICS_CHANGESIDES, FLAG_ISSET)  // captured minions do not trigger BC
      ) {
        flag(&attacker->state, MECHANICS_BATTLECRY_TRIGGER, FLAG_SET);

        {
            struct card_s *__fake_levelup = NULL;
            __SECTION_BATTLECRY
        }
    }

    {
        struct card_s *__fake_attacker = attacker;
        __SECTION_IDLE
    }

    {
        __SECTION_ONBOARD
    }
}
