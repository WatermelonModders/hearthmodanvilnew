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
#ifndef TURNTIMER_H_
#define TURNTIMER_H_

struct turntimer_s {
    u64 seconds;
    u64 turn;
    char show;
};

void turntimer_free(struct turntimer_s *t);
int turntimer_serialize(void *ao, char **dst, const char *maxdst);
void turntimer_dump(struct turntimer_s *t);

#endif
