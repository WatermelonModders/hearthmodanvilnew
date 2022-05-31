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

void handshake_free(struct handshake_s *h)
{
    platform_free(h->platform);

    free(h->version);
    free(h->password);

    free(h);
}

void *handshake_deserialize(char **dst, const char *maxdst)
{
    int n, len;
    struct handshake_s *h;

    h = malloc(sizeof(*h));

    n = read_byte(dst, maxdst);
    if(n != 8) {
        error();
    }
    h->gamehandle = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n != 18) {
        error();
    }
    h->password = read_bytes(dst, maxdst, &h->npassword);

    n = read_byte(dst, maxdst);
    if(n != 24) {
        error();
    }
    h->clienthandle = read_uint64(dst, maxdst);

    n = read_byte(dst, maxdst);
    if(n == 32) {
        h->mission = read_uint64(dst, maxdst);
        n = read_byte(dst, maxdst);
    }

    if(n != 42) {
        error();
    }
    h->version = read_bytes(dst, maxdst, &h->nversion);

    n = read_byte(dst, maxdst);
    if(n != 58) {
        error();
    }
    len = read_uint(dst, maxdst);
    h->platform = platform_deserialize(dst, *dst + len);

    return h;
}

void handshake_dump(struct handshake_s *h)
{
    hm_log(LOG_DEBUG, lg, "Gamehandle: %lld password: [%.*s] clienthandle: %lld mission: %lld verion: [%.*s] ", h->gamehandle, h->npassword, h->password, h->clienthandle, h->mission, h->nversion, h->version);
    if(h->platform) {
        hm_log(LOG_DEBUG, lg, "Os: %lld screen: %lld name: [%.*s] store: %lld", h->platform->os, h->platform->screen, h->platform->nname, h->platform->name, h->platform->store);

    }
}
