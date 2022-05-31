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
#ifndef GAME_OPTIONS_H_
#define GAME_OPTIONS_H_

int set_options(struct deck_s *deck, struct deck_s *opponent, char *buffer, char *endbuffer, int turn);
int set_options_cards(struct card_list_s *cl, char *buffer, char *endbuffer, int turn);

#endif
