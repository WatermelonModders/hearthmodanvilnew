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

struct packet_s *net_game_over(struct hs_holder_s *p, int loser)
{
    struct packet_s *packet = NULL;
    struct powerhistory_s *ph = NULL;
    struct powerhistory_data_s *data = NULL;

    add_powerstart(&data, 5, 0, 2, 0, NULL, 0);

    add_tagchange(&data, PLAYSTATE, 3, loser == 2 ? 3 : 2);   // needed
    add_tagchange(&data, PLAYSTATE, 5, loser == 2 ? 3 : 2);   // needed
    add_tagchange(&data, PLAYSTATE, 4, loser);                // needed

    add_powerend(&data);

    add_ph(&ph, data);
    add_packet(&packet, ph, P_POWERHISTORY);

    powerhistory_dump(ph);

    return packet;
}
