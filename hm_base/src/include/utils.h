/*
   hm_base - hearthmod base library
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
#ifndef UTILS_H_
#define	UTILS_H_

#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <memory.h>

#define EQFLAG(d, f)\
    ((d & f) == f)

/** N memory compare */
#define nmc(dst, ndst, src, nsrc) (ndst == nsrc && memcmp(dst, src, ndst) == 0)

inline static void to_lower(char *bytes, const int nbytes)
{
    int i;

    for(i = 0; i < nbytes; i++) {
        if(bytes[i] >= 65 && bytes[i] <= 90) {
            bytes[i] += 32;
        }
    }
}

inline static void mac_tonum(char *dst, const int ndst, char *src, const int nsrc)
{
    char *s, *e;
    int i;

    if(nsrc != 17 || ndst < 17) {
        return;
    }

    for(i = 0, s = src, e = src + nsrc; s < e; s++) {
        if(*s == ':' && s - 2 >= src) {
            memcpy(&dst[i], s - 2, 2);
            i += 2;
        }
        else if(i == 10 && s + 2 <= e) {
            memcpy(&dst[i], s, 2);
            i += 2;
        }
    }

    dst[i] = '\0';
}

inline static void replace_space(char *dst, const int ndst, const char src)
{
    char *s, *e;

    for(s = dst, e = dst + ndst; s < e; s++) {
        if(*s == ' ') {
            *s = src;
        }
    }
}

inline static void num_tomac(char *dst, const int ndst, char *src, const int nsrc)
{
    int i = 0;
    char *ds, *de;
    char *ss, *se;

    // 17 + 1 term
    if(ndst < 18) {
        return;
    }

    for(i = 0,
            ss = src, se = src + nsrc,
            ds = dst, de = dst + ndst;
            (ss < se && (ds + i)< de);
            /* void */) {
        memcpy(&dst[i], ss, 2);
        i += 2;
        dst[i++] = ':';
        ss += 2;
        if(i >= 16) {
            dst[--i] = '\0';
            return;
        }
    }
}

inline static int is_valid_mac(char *buf, const int len)
{
    char *s, *e;

    if(len < 17) {
        return -1;
    }

    /** buf must point to first char of mac */
    for(s = buf, e = buf + len; s < e; s++) {
        /** we're looking for a mac address xx:xx:xx:xx:xx:xx
         * ( 1 - : ) || ( A - Z ) || ( a - z )
         */
        if(!((*s >= 48 && *s <= 58) || (*s >= 65 && *s <= 90) || (*s >= 97 && *s <= 122))) {
            return -1;
        }

        if(s - buf == 16) {
            return (s - buf) + 1;
        }
    }

    return -1;
}

inline static int random_number(int max)
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);
    srand(ts.tv_nsec);

    return (rand() % max);
}

#endif
