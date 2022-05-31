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

int game(struct conn_client_s *c, struct chooseoption_s *opt)
{
    struct hs_holder_s *p;
    struct card_s *attacker = NULL;

    p = c->data;

    if(p->held_card > 0) {
        attacker = card_get(p->deck, NULL, p->held_card);
    }

    if(opt->index == 0) {
        next_turn(c, p);
    } else if(
            (!(attacker && flag(&attacker->state, CARD_BATTLECRY, FLAG_ISSET))) &&
            (!(attacker && flag(&attacker->state, CARD_WEAPON, FLAG_ISSET))) &&
            (
             (opt->target != 0 && p->arroworigin != 0) ||
             (p->held_card > 0 && flag(&attacker->state, CARD_SPELL, FLAG_ISSET)))

            /*
               (p->held_card > 0 && flag(&attacker->state, CARD_BUFF_AOE, FLAG_ISSET)) ||
               (p->held_card > 0 && flag(&attacker->state, CARD_VANISH, FLAG_ISSET)) ||
               (p->held_card > 0 && flag(&attacker->state, CARD_SUMMON, FLAG_ISSET)) ||
               (p->held_card > 0 && flag(&attacker->state, CARD_DRAW_CARD, FLAG_ISSET)) ||
               (p->held_card > 0 && flag(&attacker->state, CARD_HEROPOWER, FLAG_ISSET))
               */
            ) {

        // attack or spell damage
        hm_log(LOG_INFO, c->log, "Power attack held: %d, attacker: %p, target: %lld, arroworigin: %d", p->held_card, attacker, opt->target, p->arroworigin);
        power_attack(c, p, opt);
    } else if(p->held_card > 0) {
        // put card on board
        put_card_onboard(c, p, opt);
    }

    //skip:

    // unset origin
    p->arroworigin = 0;

    // unset
    p->held_card = 0;

    cards_dump(p->deck, p->opponent.deck);

    unset_mechanics_flags(p);

    hm_log(LOG_INFO, c->log, "Unsetting held card %d arroworigin %d", p->held_card, p->arroworigin);

    return 0;


}
