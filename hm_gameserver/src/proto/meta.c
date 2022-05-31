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

void meta_free(struct powerhistory_meta_s *t)
{
    struct powerhistory_info_s *info, *del;

    for(info = t->info; info != NULL; del = info, info = info->next, free(del));

    free(t);
}

struct powerhistory_meta_s *meta_deserialize(char **dst, const char *maxdst)
{
    int n, len;
    struct powerhistory_meta_s *t;
    struct powerhistory_info_s *info;
    int i;

    t = malloc(sizeof(*t));
    memset(t, 0, sizeof(*t));

    while(*dst < maxdst) {
        n = read_byte(dst, maxdst);
        if(n == 18) {
            len = read_uint(dst, maxdst);

            for(i = 0; i < len; i++) {
                info = malloc(sizeof(*info));
                info->id = read_uint64(dst, maxdst);
                info->next = NULL;

                if(t->info == NULL && t->info_tail == NULL) {
                    t->info = info;
                    t->info_tail = info;
                } else {
                    t->info_tail->next = info;
                    t->info_tail = info;
                }
            }
        } else if(n == 24) {
            t->type = read_uint64(dst, maxdst);
        } else if(n == 32) {
            t->data = read_uint64(dst, maxdst);
        } else {
            error();
        }
    }

    return t;
}


int meta_size(struct powerhistory_meta_s *p)
{
    int num = 0, num2;
    struct powerhistory_info_s *m;

    if(p->info) {
        num += 1;
        num2 = num;
        for(m = p->info; m != NULL; m = m->next) {
            num += sizeofu64(m->id);
        }
        num += sizeofu32(num - num2);
    }

    if(p->type != 0) {
        num += 1;
        num += sizeofu64(p->type);
    }

    if(p->data != 0) {
        num += 1;
        num += sizeofu64(p->data);
    }

    return num;
}

int meta_serialize(struct powerhistory_meta_s *p, char **dst, const char *maxdst)
{
    int num = 0;
    struct powerhistory_info_s *m;

    if(p->info) {
        write_byte(dst, maxdst, 18);
        for(m = p->info; m != NULL; m = m->next) {
            num += sizeofu64(m->id);
        }

        write_uint(dst, maxdst, num);

        for(m = p->info; m != NULL; m = m->next) {
            write_uint64(dst, maxdst, m->id);
        }
    }

    if(p->type != 0) {
        write_byte(dst, maxdst, 24);
        write_uint64(dst, maxdst, p->type);
    }

    if(p->data != 0) {
        write_byte(dst, maxdst, 32);
        write_uint64(dst, maxdst, p->data);
    }

    return 0;
}
