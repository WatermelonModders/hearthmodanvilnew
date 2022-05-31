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
#ifndef ENT_S_
#define ENT_S_

#include <flag_def.h>
#include <flags.h>

#define MAX_PROPERTIES		16

struct ent_property_s {
    int key;
    int value;
};

struct ent_s {
    const char *name;
    int nname;
    int nprop;

    const char *level2;
    const char *level3;

    const char *desc;

    struct flags_s flags;
    const char *attachment;
    struct ent_property_s p[MAX_PROPERTIES];
};

extern struct ent_s entities[MAX_ENTITIES];
void entities_init();
void entities_init2();

#endif
