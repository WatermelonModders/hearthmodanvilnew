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

void powerhistory_data_free(struct powerhistory_data_s *d)
{
    if(d->full) entity_free(d->full);
    if(d->show) entity_free(d->show);
    if(d->hide) hide_free(d->hide);
    if(d->tagchange) tagchange_free(d->tagchange);
    if(d->creategame) creategame_free(d->creategame);
    if(d->powerstart) powerstart_free(d->powerstart);
    if(d->powerend) powerend_free(d->powerend);
    if(d->change_entity) entity_free(d->change_entity);
    if(d->meta) meta_free(d->meta);

    free(d);
}

struct powerhistory_data_s *powerhistory_data_deserialize(char **dst, const char *maxdst)
{
    struct powerhistory_data_s *p;
    int n, len;

    p = malloc(sizeof(*p));
    memset(p, 0, sizeof(*p));

    while(*dst < maxdst) {
        n = read_byte(dst, maxdst);
        len = read_uint(dst, maxdst);

        if(n == 10) {
            p->full = entity_deserialize(dst, *dst + len);
        } else if(n == 18) {
            p->show = entity_deserialize(dst, *dst + len);
        } else if(n == 26) {
            p->hide = hide_deserialize(dst, *dst + len);
        } else if(n == 34) {
            p->tagchange = tagchange_deserialize(dst, *dst + len);
        } else if(n == 42) {
            p->creategame = creategame_deserialize(dst, *dst + len);
        } else if(n == 50) {
            p->powerstart = powerstart_deserialize(dst, *dst + len);
        } else if(n == 58) {
            p->powerend = powerend_deserialize(dst, *dst + len);
        } else if(n == 66) {
            p->meta = meta_deserialize(dst, *dst + len);
        } else {
            return NULL;
        }
    }

    return p;
}

int powerhistorydata_serialize(struct powerhistory_data_s *phd, char **dst, const char *maxdst)
{
    int n;

    if(phd->full) {
        write_byte(dst, maxdst, 10);
        n = entity_size(phd->full);
        write_uint(dst, maxdst, n);
        entity_serialize(phd->full, dst, maxdst);
    }

    if(phd->show) {
        write_byte(dst, maxdst, 18);
        n = entity_size(phd->show);
        write_uint(dst, maxdst, n);
        entity_serialize(phd->show, dst, maxdst);
    }

    if(phd->hide) {
        write_byte(dst, maxdst, 26);
        n = hide_size(phd->hide);
        write_uint(dst, maxdst, n);
        hide_serialize(phd->hide, dst, maxdst);
    }

    if(phd->tagchange) {
        write_byte(dst, maxdst, 34);
        n = tagchange_size(phd->tagchange);
        write_uint(dst, maxdst, n);
        tagchange_serialize(phd->tagchange, dst, maxdst);
    }

    if(phd->creategame) {
        write_byte(dst, maxdst, 42);
        n = creategame_size(phd->creategame);
        write_uint(dst, maxdst, n);
        creategame_serialize(phd->creategame, dst, maxdst);
    }

    if(phd->powerstart) {
        write_byte(dst, maxdst, 50);
        n = powerstart_size(phd->powerstart);
        write_uint(dst, maxdst, n);
        powerstart_serialize(phd->powerstart, dst, maxdst);
    }

    if(phd->powerend) {
        write_byte(dst, maxdst, 58);
        n = powerend_size(phd->powerend);
        write_uint(dst, maxdst, n);
        powerend_serialize(phd->powerend, dst, maxdst);
    }

    if(phd->meta) {
        write_byte(dst, maxdst, 66);
        n = meta_size(phd->meta);
        write_uint(dst, maxdst, n);
        meta_serialize(phd->meta, dst, maxdst);
    }

    if(phd->change_entity) {
        write_byte(dst, maxdst, 74);
        n = entity_size(phd->change_entity);
        write_uint(dst, maxdst, n);
        entity_serialize(phd->change_entity, dst, maxdst);
    }

    return 0;
}

int powerhistorydata_size(struct powerhistory_data_s *phd)
{
    int num = 0, n;

    if(phd->full) {
        num += 1;
        n = entity_size(phd->full);
        num += n + sizeofu32(n);
    }

    if(phd->show) {
        num += 1;
        n = entity_size(phd->show);
        num += n + sizeofu32(n);
    }

    if(phd->hide) {
        num += 1;
        n = hide_size(phd->hide);
        num += n + sizeofu32(n);
    }

    if(phd->tagchange) {
        num += 1;
        n = tagchange_size(phd->tagchange);
        num += n + sizeofu32(n);
    }

    if(phd->creategame) {
        num += 1;
        n = creategame_size(phd->creategame);
        num += n + sizeofu32(n);
    }

    if(phd->powerstart) {
        num += 1;
        n = powerstart_size(phd->powerstart);
        num += n + sizeofu32(n);
    }

    if(phd->powerend) {
        num += 1;
        n = powerend_size(phd->powerend);
        num += n + sizeofu32(n);
    }

    if(phd->meta) {
        num += 1;
        n = meta_size(phd->meta);
        num += n + sizeofu32(n);
    }

    if(phd->change_entity) {
        num += 1;
        n = entity_size(phd->change_entity);
        num += n + sizeofu32(n);
    }

    return num;
}
