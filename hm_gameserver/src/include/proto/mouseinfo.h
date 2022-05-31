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
#ifndef MOUSEINFO_H_
#define MOUSEINFO_H_

struct mouseinfo_s {
    u64 arroworigin;
    u64 heldcard;
    u64 overcard;
    u64 x;
    u64 y;
};

void mouseinfo_free(struct mouseinfo_s *p);
void *mouseinfo_deserialize(char **dst, const char *maxdst);
int mouseinfo_serialize(void *ao, char **dst, const char *maxdst);
int mouseinfo_size(struct mouseinfo_s *p);

#endif
