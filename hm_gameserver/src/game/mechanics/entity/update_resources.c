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

int update_resources(struct deck_s *deck, struct card_s *attacker, int refresh)
{
    // refresh resources
    if(refresh == 1) {
        hm_log(LOG_DEBUG, lg, "Mana refreshed to %d", deck->mana);
        deck->mana_used = 0;
        return 0;
    }

    deck->mana_used += attacker->cost;

    hm_log(LOG_DEBUG, lg, "Card cost: %d", attacker->cost);
    hm_log(LOG_DEBUG, lg, "Mana used: %d Mana total: %d", deck->mana_used, deck->mana);

    return 0;
}
