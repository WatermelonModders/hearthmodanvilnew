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
#ifndef USERUI_H_
#define USERUI_H_

struct userui_s {
    struct mouseinfo_s *mouseinfo;
    u64 emote;
    u64 player_id;
};

void userui_free(struct userui_s *u);
int userui_serialize(void *ao, char **dst, const char *maxdst);
void *userui_deserialize(char **dst, const char *maxdst);
int userui_size(struct userui_s *p);
void userui_dump(struct userui_s *u);

#endif
