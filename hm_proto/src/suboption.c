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

void suboption_free(struct suboption_s *s)
{
    struct suboption_target_s *t, *del;

    for(t = s->target; t != NULL; del = t, t = t->next, free(del));

    free(s);
}

int suboption_size(struct suboption_s *p)
{
    int num = 0, num2;
    struct suboption_target_s *m;

    num += sizeofu64(p->id);

    if(p->target) {
        num += 1;
        num2 = num;
        for(m = p->target; m != NULL; m = m->next) {
            num += sizeofu64(m->value);
        }
        num += sizeofu32(num - num2);
    }

    num += 1;

    return num;
}

int suboption_serialize(struct suboption_s *s, char **dst, const char *maxdst)
{
    int num = 0;
    struct suboption_target_s *m;
    char *start;

    start = *dst;

    write_byte(dst, maxdst, 8);
    write_uint64(dst, maxdst, s->id);

    if(s->target) {
        write_byte(dst, maxdst, 26);
        for(m = s->target; m != NULL; m = m->next) {
            num += sizeofu64(m->value);
        }

        write_uint(dst, maxdst, num);

        for(m = s->target; m != NULL; m = m->next) {
            write_uint64(dst, maxdst, m->value);
        }
    }

    return (*dst - start);
}
