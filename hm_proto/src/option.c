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

void option_free(struct option_s *o)
{
    struct suboption_s *s, *del;

    for(s = o->mainoption; s != NULL; del = s, s = s->next, suboption_free(del));

    for(s = o->suboptions; s != NULL; del = s, s = s->next, suboption_free(del));

    free(o);
}

int option_serialize(struct option_s *s, char **dst, const char *maxdst)
{
    int n = 0;
    struct suboption_s *m;
    char *start;

    start = *dst;

    write_byte(dst, maxdst, 8);
    write_uint64(dst, maxdst, s->type);

    if(s->mainoption) {
        write_byte(dst, maxdst, 18);
        n = suboption_size(s->mainoption);
        write_uint(dst, maxdst, n);
        suboption_serialize(s->mainoption, dst, maxdst);
    }

    if(s->suboptions) {
        for(m = s->suboptions; m != NULL; m = m->next) {
            write_byte(dst, maxdst, 26);
            n = suboption_size(m);
            write_uint(dst, maxdst, n);
            suboption_serialize(m, dst, maxdst);
        }
    }

    return (*dst - start);
}

int option_size(struct option_s *p)
{
    int num = 0, n;
    struct suboption_s *s;

    num += 1;
    num += sizeofu64(p->type);

    if(p->mainoption) {
        num += 1;
        n = suboption_size(p->mainoption);
        num += sizeofu32(n) + n;
    }

    if(p->suboptions) {
        for(s = p->suboptions; s != NULL; s = s->next) {
            num += 1;
            n = suboption_size(s);
            num += sizeofu32(n) + n;
        }
    }

    return num;
}
