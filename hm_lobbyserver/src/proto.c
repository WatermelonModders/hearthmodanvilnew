/*
   hm_lobbyserver - HearthStone HearthMod lobbyserver
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
#include <stdio.h>
#include <memory.h>

#include <hmbase.h>

#include <client.h>

int proto_add(char **dst, const char *maxdst, const void *src, const int nsrc)
{
    char *start;

    if(maxdst < *dst + nsrc + sizeof(nsrc)) {
        return -1;
    }

    start = *dst;

    memcpy(*dst, &nsrc, sizeof(nsrc));
    *dst += sizeof(nsrc);

    memcpy(*dst, src, nsrc);
    *dst += nsrc;

    return (*dst - start);
}

int proto_packet(char *dst, enum packet_e packet, char *src, const int nsrc)
{
    int len;
    int magic = 0xffffffff;

    len = 3 * sizeof(packet) + nsrc;

    memcpy(dst, &magic, sizeof(magic));
    dst += sizeof(magic);

    memcpy(dst, &packet, sizeof(packet));
    dst += sizeof(packet);

    memcpy(dst, &len, sizeof(len));
    dst += sizeof(len);

    memcpy(dst, src, nsrc);
    dst += nsrc;

    return len;

}

char *read_str(char **buf, char *end, int *len)
{
    char *r;

    if(end < (*buf + 4)) {
        return NULL;
    }

    *len = *(int *)(*buf);
    *buf += sizeof(int);

    if(*len + *buf > end) {
        return NULL;
    }

    r = *buf;

    *buf += *len;

    return r;
}
