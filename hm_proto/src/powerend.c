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
#include <proto.h>

void powerend_free(struct powerhistory_powerend_s *p)
{
    free(p);
}

struct powerhistory_powerend_s *powerend_deserialize(char **dst, const char *maxdst)
{
    struct powerhistory_powerend_s *t;

    t = malloc(sizeof(*t));

    return t;
}

int powerend_serialize(struct powerhistory_powerend_s *t, char **dst, const char *maxdst)
{
    return 0;
}

int powerend_size(struct powerhistory_powerend_s *p)
{
    return 0;
}
