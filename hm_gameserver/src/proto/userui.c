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

void userui_free(struct userui_s *u)
{
    mouseinfo_free(u->mouseinfo);

    free(u);
}

void *userui_deserialize(char **dst, const char *maxdst)
{
    int n, len;
    struct userui_s *c;

    c = malloc(sizeof(*c));
    memset(c, 0, sizeof(*c));

    while(*dst < maxdst) {
        n = read_byte(dst, maxdst);
        if(n == 10) {
            len = read_uint(dst, maxdst);
            c->mouseinfo = mouseinfo_deserialize(dst, *dst + len);
        }
        else if(n == 16) {
            c->emote = read_uint64(dst, maxdst);
        }
        else if(n == 24) {
            c->player_id = read_uint64(dst, maxdst);
        }
    }

    return c;
}

int userui_serialize(void *ao, char **dst, const char *maxdst)
{
    struct userui_s *s;
    char *start;
    int n;

    start = *dst;
    s = ao;

    if(s->mouseinfo) {
        write_byte(dst, maxdst, 10);
        n = mouseinfo_size(s->mouseinfo);
        write_uint(dst, maxdst, n);
        mouseinfo_serialize(s->mouseinfo, dst, maxdst);
    }

    if(s->emote != -1) {
        write_byte(dst, maxdst, 16);
        write_uint64(dst, maxdst, s->emote);
    }

    if(s->player_id != -1) {
        write_byte(dst, maxdst, 24);
        write_uint64(dst, maxdst, s->player_id);
    }

    return (*dst - start);
}

int userui_size(struct userui_s *p)
{
    int num = 0, n;

    if(p->mouseinfo) {
        num += 1;
        n = mouseinfo_size(p->mouseinfo);
        num += n + sizeofu32(n);	
    }

    if(p->emote != 0) {
        num += 1;
        num += sizeofu64(p->emote);	
    }

    if(p->player_id != 0) {
        num += 1;
        num += sizeofu64(p->player_id);	
    }

    return num;
}

void userui_dump(struct userui_s *u)
{
    if(u) {
        if(u->mouseinfo) {
            hm_log(LOG_DEBUG, lg, "arrow origin: %lld", u->mouseinfo->arroworigin);
            hm_log(LOG_DEBUG, lg, "heldcard: %lld", u->mouseinfo->heldcard);
            hm_log(LOG_DEBUG, lg, "overcard: %lld", u->mouseinfo->overcard);
            hm_log(LOG_DEBUG, lg, "x: %lld", u->mouseinfo->x);
            hm_log(LOG_DEBUG, lg, "y: %lld", u->mouseinfo->y);
        }

        if(u->emote != 0) {
            hm_log(LOG_DEBUG, lg, "emote: %lld", u->emote);
        }

        if(u->player_id != 0) {
            hm_log(LOG_DEBUG, lg, "player_id: %lld", u->player_id);
        }

    }
}
