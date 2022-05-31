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

void turntimer_free(struct turntimer_s *t)
{
    free(t);
}

int turntimer_serialize(void *ao, char **dst, const char *maxdst)
{
    struct turntimer_s *s;
    char *start;

    start = *dst;
    s = ao;

    write_byte(dst, maxdst, 8);
    write_uint64(dst, maxdst, s->seconds);

    write_byte(dst, maxdst, 16);
    write_uint64(dst, maxdst, s->turn);

    write_byte(dst, maxdst, 24);
    write_byte(dst, maxdst, s->show);

    return (*dst - start);
}

void turntimer_dump(struct turntimer_s *t)
{
    hm_log(LOG_DEBUG, lg, "Turn timer dump:");
    hm_log(LOG_DEBUG, lg, "\t\tseconds: %lld", t->seconds);
    hm_log(LOG_DEBUG, lg, "\t\tturn: %lld", t->turn);
    hm_log(LOG_DEBUG, lg, "\t\tshow: %d", t->show);
}
