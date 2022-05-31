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
#ifndef ENTITYCHOICES_H_
#define ENTITYCHOICES_H_

struct entitychoices_entities_s {
    u64 entity;
    struct entitychoices_entities_s *next;
};

struct entitychoices_s {
    u64 id;
    u64 type;
    u64 countmin;
    u64 countmax;

    u64 source;
    u64 player_id;

    struct entitychoices_entities_s *entities;
};

void entitychoices_free(struct entitychoices_s *ec);
int entitychoices_serialize(void *ep, char **dst, const char *maxdst);
void entitychoices_dump(struct entitychoices_s *es);

#endif
