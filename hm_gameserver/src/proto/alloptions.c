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

void alloptions_free(struct alloptions_s *a)
{
    struct option_s *o, *del;

    for(o = a->options; o != NULL; del = o, o = o->next, option_free(del));

    free(a);
}

int alloptions_serialize(void *ao, char **dst, const char *maxdst)
{
    int n = 0;
    struct option_s *m;
    struct alloptions_s *s;
    char *start;

    start = *dst;
    s = ao;

    write_byte(dst, maxdst, 8);
    write_uint64(dst, maxdst, s->id);

    for(m = s->options; m != NULL; m = m->next) {
        write_byte(dst, maxdst, 18);
        n = option_size(m);
        write_uint(dst, maxdst, n);
        option_serialize(m, dst, maxdst);
    }

    return (*dst - start);
}

int alloptions_size(struct alloptions_s *p)
{
    int num = 0, n;
    struct option_s *s;

    num += 1;
    num += sizeofu64(p->id);

    for(s = p->options; s != NULL; s = s->next) {
        num += 1;
        n = option_size(s);
        num += sizeofu32(n) + n;
    }

    return num;
}

void alloptions_dump(struct alloptions_s *a)
{
    struct suboption_target_s *t;
    struct suboption_s *m;
    struct option_s *o;

    hm_log(LOG_DEBUG, lg, "Dumping alloptions:");	
    hm_log(LOG_DEBUG, lg, "id: %lld", a->id);
    for(o = a->options; o != NULL; o = o->next) {
        hm_log(LOG_DEBUG, lg, "\tOption type: %lld", o->type);
        for(m = o->mainoption; m != NULL; m = m->next) {
            hm_log(LOG_DEBUG, lg, "\t\tMainoption id: %lld", m->id);
            for(t = m->target; t != NULL; t = t->next) {
                hm_log(LOG_DEBUG, lg, "\t\t\tTarget id: %lld", t->value);
            }
        }
        assert(o->suboptions == NULL);
    }
}
