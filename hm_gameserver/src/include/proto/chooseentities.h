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
#ifndef CHOOSEENTITIES_H_
#define CHOOSEENTITIES_H_

struct chooseentities_ent_s {
    u64 entity;
    struct chooseentities_ent_s *next;
};

struct chooseentities_s {
    u64 id;
    int nentity;
    struct chooseentities_ent_s *entity;
};

void chooseentities_free(struct chooseentities_s *c);
void *chooseentities_deserialize(char **dst, const char *maxdst);

#endif
