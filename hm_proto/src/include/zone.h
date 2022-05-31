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
#ifndef ZONE_H_
#define ZONE_H_

enum zone_e {
    ZONE_INVALID = 0,
    ZONE_PLAY = 1,
    ZONE_DECK = 2,
    ZONE_HAND = 3,
    ZONE_GRAVEYARD = 4,
    ZONE_REMOVEDFROMGAME = 5,
    ZONE_SETASIDE = 6,
    ZONE_SECRET = 7,
    ZONE_DISCARD = -2,
};

#endif
