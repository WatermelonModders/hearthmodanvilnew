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

int sizeofu64(u64 val)
{
    unsigned int num = 1u;

    while(1 == 1) {
        val >>= 7;
        if(val == 0) {
            break;
        }
        num++;
    }

    return num;
}

int sizeofu32(int val)
{
    unsigned int num = 1u;

    while(1 == 1) {
        val >>= 7;
        if(val == 0) {
            break;
        }
        num++;
    }

    return num;
}

int read_mem_int(char **dst, const char *end)
{
    int n;

    if(*dst + sizeof(int) > end) {
        return -1;
    }

    n = *(int *)(*dst);
    (*dst) += sizeof(int);
    return n;
}

int write_mem_int(char **dst, const char *end, const int src)
{
    if(*dst + sizeof(src) > end) {
        hm_log(LOG_EMERG, lg, "Cannot write %d %p %p", src, *dst, end);
        abort();
    }

    memcpy(*dst, &src, sizeof(src));
    (*dst) += sizeof(src);
    return 0;
}


int write_byte(char **dst, const char *end, char src)
{
    if(*dst + sizeof(src) > end) {
        hm_log(LOG_EMERG, lg, "Cannot write %d %p %p", src, *dst, end);
        abort();
    }

    memcpy(*dst, &src, sizeof(src));
    (*dst)++;
    return 0;
}

char read_byte(char **dst, const char *end)
{
    char out;

    if(*dst + sizeof(char) > end) {
        return -1;
    }

    out = *((char *)(*dst));

    (*dst)++;
    return out;
}

int skip(char **dst, const char *end, const int jump)
{
    if((*dst + jump) > end) {
        hm_log(LOG_ALERT, lg, "Invalid skip %p %p %d", *dst, end, jump);
        return -1;
    }

    *dst += jump;

    return 0;
}

int read_uint(char **dst, const char *end)
{
    int num = 0, num2;
    int i;

    for(i = 0; (i < 5 && *dst < end); i++) {
        num2 = read_byte(dst, end);

        if(i == 4 && (num2 & 240) != 0) {
            hm_log(LOG_EMERG, lg, "Incorrect num size");
            abort();
        }

        if((num2 & 128) == 0) {
            return num | (unsigned int)((unsigned int)num2 << 7 * i);
        }

        num |= (unsigned int)((unsigned int)(num2 & 127) << 7 * i);
    }

    return num;
}

u64 read_uint64(char **dst, const char *end)
{
    u64 num = 0;
    int num2;
    int i;

    for(i = 0; (i < 10 && *dst < end); i++) {
        num2 = read_byte(dst, end);

        if(i == 9 && (num2 & 254) != 0) {
            hm_log(LOG_EMERG, lg, "Incorrect num size");
            abort();
        }

        if((num2 & 128) == 0) {
            return num | (unsigned long long)((unsigned long long)num2 << 7 * i);
        }

        num |= (unsigned long long)((unsigned long long)(num2 & 127) << 7 * i);
    }

    return num;
}

void write_uint(char **dst, const char *end, int src)
{
    char b;

    while(1 == 1) {
        b = (char)(src & 127);
        src >>= 7;

        if(src == 0) {
            break;
        }

        b |= 128;

        write_byte(dst, end, b);
    }

    write_byte(dst, end, b);
}

char *read_bytes(char **dst, const char *end, int *ndst)
{
    char *out;

    *ndst = read_uint(dst, end);

    if(*dst + *ndst > end) {
        hm_log(LOG_EMERG, lg, "Dst read bytes %p %p %d", end, *dst + *ndst, *ndst);
        abort();
    }

    out = malloc(*ndst);
    memcpy(out, *dst, *ndst);
    *dst += *ndst;

    return out;
}

int write_bytes(char **dst, const char *end, const char *src, const int nsrc)
{
    if(*dst + nsrc > end) {
        hm_log(LOG_EMERG, lg, "Cannot write %d %p %p", nsrc, *dst, end);
        abort();
    }

    write_uint(dst, end, nsrc);

    memcpy(*dst, src, nsrc);
    (*dst) += nsrc;
    return 0;
}

void write_uint64(char **dst, const char *end, u64 src)
{
    char b;

    while(1 == 1) {
        b = (char)(src & 127);
        src >>= 7;

        if(src == 0) {
            break;
        }

        b |= 128;

        write_byte(dst, end, b);
    }

    write_byte(dst, end, b);
}
