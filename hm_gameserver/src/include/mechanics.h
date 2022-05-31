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
#ifndef MECHANICS_H_
#define MECHANICS_H_

enum expire_e {
    EXPIRE_ATTACK = 1,
    EXPIRE_FROZEN = 2,
    EXPIRE_DESTROY = 3,
    EXPIRE_ATTACHMENT = 4,
    EXPIRE_MAGIC_IMMUNITY = 5,
};

void mechanics_expire(int game_turn, struct card_s *card, enum expire_e key, int value, const char *attachment);
void mechanics_expired(struct hs_holder_s *p, struct deck_s *deck);

#endif
