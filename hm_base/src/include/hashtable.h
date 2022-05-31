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
#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#define HT_MAX          (1<<12)
#define HT_ALLOC        0x1

#define HT_ADD(ht, key, nkey, value, nvalue, pool) ht_add(ht, key, nkey, value, nvalue, HT_ALLOC, pool)
#define HT_ADD_WA(ht, key, nkey, value, nvalue, pool) ht_add(ht, key, nkey, value, nvalue, 0, pool)
#define HT_REM(ht, key, nkey, pool) ht_rem(ht, key, nkey, pool)

#define ht_item_clear(pool, dst)\
    if(dst && dst->n > 0) {\
        hm_pfree(pool, dst->s);\
        dst->s = NULL;\
        dst->n = 0;\
    }

struct ht_s {
    char *k;
    int nk;
    char *s;        ///< value
    int n;          ///< nvalue
    unsigned int flag;
    struct ht_s *next;
};

struct ht_s **ht_init(struct hm_pool_s *pool);
void ht_free(struct ht_s **ht, struct hm_pool_s *pool);
int ht_add(struct ht_s **ht, const char *key, const int nkey, const void *value, const int nvalue, const int alloc, struct hm_pool_s *pool);
int ht_rem(struct ht_s **ht, const char *key, const int nkey, struct hm_pool_s *pool);
struct ht_s *ht_get(struct ht_s **ht, const char *key, const int nkey);
void ht_dump_index(struct ht_s **ht, const char *key, const int nkey);

#endif
