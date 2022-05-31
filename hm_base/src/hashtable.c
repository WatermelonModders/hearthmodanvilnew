/*
   hm_base -  hearthmod base library
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
#include <assert.h>

#include <hmbase.h>

struct ht_s **ht_init(struct hm_pool_s *pool)
{
    struct ht_s **ht;

    ht = hm_palloc(pool, sizeof(void *) * HT_MAX);
    if(ht == NULL) {
        return NULL;
    }

    memset(ht, 0, sizeof(void *) * HT_MAX);

    return ht;
}

void ht_free(struct ht_s **ht, struct hm_pool_s *pool)
{
    hm_pfree(pool, ht);
}

inline static void ht_key(int *dst, const char *key, const int nkey)
{
    int i;

    for(i = 0; i < nkey; i++) {
        *dst += key[i];
    }

    *dst %= HT_MAX;
}

int ht_add(struct ht_s **ht, const char *key, const int nkey, const void *value, const int nvalue, const int alloc, struct hm_pool_s *pool)
{
    struct ht_s *h;
    int index = 0;

    assert(ht);

    ht_key(&index, key, nkey);

    for(h = ht[index]; h != NULL; h = h->next) {
        if(nkey == h->nk && memcmp(key, h->k, nkey) == 0) {
            /** first, free existing value */
            if(h->flag == HT_ALLOC) {
                hm_pfree(pool, h->s);
            }

            /** then copy new value */
            if(alloc == HT_ALLOC) {
                h->s = hm_palloc(pool, nvalue);
                if(h->s == NULL) {
                    return -1;
                }
#ifdef POOL_STDLIB
                memcpy(h->s, value, nvalue);
#else
                hm_memcpy(h->s, value, nvalue, h->s);
#endif
            } else {
                h->s = (void *)value;
            }

            h->flag = alloc;
            h->n = nvalue;

            return 0;
        }
    }

    h = hm_palloc(pool, sizeof(*h));
    if(h == NULL) {
        return -1;
    }

    h->nk = nkey;
    h->k = hm_palloc(pool, nkey);
    if(h->k == NULL) {
        return -1;
    }

    h->n = nvalue;

    /* always allocate a key */
#ifdef POOL_STDLIB
    memcpy(h->k, key, nkey);
#else
    hm_memcpy(h->k, key, nkey, h->k);
#endif

    if(alloc == HT_ALLOC) {
        h->flag = HT_ALLOC;
        h->s = hm_palloc(pool, nvalue);
        if(h->s == NULL) {
            return -1;
        }

#ifdef POOL_STDLIB
        memcpy(h->s, value, nvalue);
#else
        hm_memcpy(h->s, value, nvalue, h->s);
#endif
    } else {
        h->flag = 0;
        h->s = (void *)value;
    }

    h->next = ht[index];
    ht[index] = h;

    return 0;
}

int ht_rem(struct ht_s **ht, const char *key, const int nkey, struct hm_pool_s *pool)
{
    struct ht_s *h, *prev = NULL;
    int index = 0;

    assert(ht);

    ht_key(&index, key, nkey);

    for(h = ht[index], prev = NULL; h != NULL; prev = h, h = h->next) {
        if(nkey == h->nk && memcmp(h->k, key, nkey) == 0) {
            if(prev == NULL) {
                ht[index] = h->next;
            } else {
                prev->next = h->next;
            }

            if(h->flag == HT_ALLOC) {
                hm_pfree(pool, h->s);
            }
            hm_pfree(pool, h->k);

            hm_pfree(pool, h);
            return 0;
        }
    }

    return -1;
}

struct ht_s *ht_get(struct ht_s **ht, const char *key, const int nkey)
{
    struct ht_s *h;
    int index = 0;

    assert(ht);

    ht_key(&index, key, nkey);

    if(ht[index]) {
        /** fast access - only one key exists under ht index */
        if(ht[index] && ht[index]->next == NULL) {
            return ht[index];
        }

        /** otherwise traverse compare against all existing keys */
        for(h = ht[index]; h != NULL; h = h->next) {
            if(h->nk == nkey && memcmp(h->k, key, nkey) == 0) {
                return h;
            }
        }
        return NULL;
    } else {
        return NULL;
    }
}

void ht_dump_index(struct ht_s **ht, const char *key, const int nkey)
{
    struct ht_s *h;
    int index = 0;

    assert(ht);

    ht_key(&index, key, nkey);

    for(h = ht[index]; h != NULL; h = h->next) {
        printf("index [%d] with key [%.*s], value [%.*s]\n", index, h->nk, h->k, h->n, h->s);
    }
}
