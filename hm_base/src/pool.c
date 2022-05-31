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
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <memory.h>

#include <hmbase.h>

#define BUCKET_MAX     2
#define ROUND16(dst)  (((dst) + 15) & ~15)

#define get_meta(m_ptr) (*(struct pool_node_s **)((struct pool_node_s ***)(m_ptr - sizeof(void *))))

static int pool_create_bucket(struct hm_pool_s *pool);

int hm_pfree(struct hm_pool_s *pool, void *ptr)
{
#ifdef POOL_STDLIB
    free(ptr);
    return 0;
#endif

    struct hm_pool_s *p;
    struct pool_node_s *node;

    if(ptr == NULL) {
        return -1;
    }

    /** get to metadata */
    node = get_meta(ptr);

    for(p = pool; p != NULL; p = p->next) {
        if(node && node->used && node->size == p->size) {
#ifdef POOL_DEBUG
            hm_log(LOG_MEMORY, pool->log, "{Pool}: old freenode: %p/%d, new freenode: %p/%d from pool:%p/%d", p->freenode, p->freenode->realsize, node, node->realsize, p, p->size);
#endif
            node->used = 0;
            node->next = p->freenode;
            p->freenode = node;

            --p->used;

            return 0;
        }
    }

    return -1;
}

static void *pool_get_node(struct hm_pool_s *pool, const int realsize)
{
    struct pool_node_s *node = NULL;

    node = pool->freenode;
    assert(node);
    pool->freenode = node->next;

    if(node->next == NULL) {
#ifdef POOL_DEBUG
        hm_log(LOG_MEMORY, pool->log, "no more freenodes remaining in pool: %p/%d, creating new bucket", pool, pool->size);
#endif
        if(pool_create_bucket(pool) != 0) {
            return NULL;
        }
    }

#ifdef POOL_DEBUG
    hm_log(LOG_MEMORY, pool->log, "{Pool}: returning node: %p/%d, new freenode: %p from pool: %p/%d", node, realsize, pool->freenode, pool, pool->size);
#endif

    /** increment used nodes */
    ++pool->used;

    /** real size of block */
    node->realsize = realsize;

    node->used = 1;

    /** metadata offset */
    return (node->ptr + sizeof(void *));
}

/** don't do anything but creating a zero valued holder */
struct hm_pool_s *hm_create_pool(struct hm_log_s *log)
{
    struct hm_pool_s *pool;

    pool = malloc(sizeof(*pool));

    if(pool == NULL) {
        return NULL;
    }

    memset(pool, 0, sizeof(*pool));
    pool->log = log;

#ifdef POOL_DEBUG
    hm_log(LOG_MEMORY, pool->log, "{Pool}: pool created %p", pool);
#endif
    return pool;
}

static int pool_create_bucket(struct hm_pool_s *pool)
{
    int i;
    void *nodes, *region;
    struct pool_bucket_s *b;
    struct pool_node_s *node;

    /** holder of buckets */
    b = malloc(sizeof(*b));

    if(b == NULL) {
        return -1;
    }

    region = malloc((pool->size + sizeof(void *)) * BUCKET_MAX);

    /** nodes holders */
    nodes = malloc(BUCKET_MAX * sizeof(struct pool_node_s));

    if(nodes == NULL) {
        return -1;
    }

    b->memory_region = region;
    b->nodes = nodes;

    for(i = 0; BUCKET_MAX > i; i++) {
        node = (struct pool_node_s *)(nodes + (i * sizeof(struct pool_node_s)));
        /** node's offset set by i * (metadata + size)*/
        node->ptr = (void *)(region + i * (sizeof(void *) + pool->size));
        /** copy metadata to offset + 0 */
        memcpy(node->ptr, &node, sizeof(node));

#ifdef POOL_DEBUG
        hm_log(LOG_MEMORY, pool->log, "new node: %p in pool: %p old freenode: %p pool size: %d node ptr: %p reg: %p", node, pool, pool->freenode, pool->size, node->ptr, *(void **)node->ptr);

        /*
           for(j = 0; j < 32; j++) {
           printf("%d|", ((char *)(node->ptr))[j]);
           }
           */
#endif

        node->size = pool->size;
        node->next = pool->freenode;
        node->pool = pool;
        node->realsize = 0;
        pool->freenode = node;
    }

    b->next = pool->buckets;
    pool->buckets = b;

    return 0;
}

void *hm_memcpy(void *dst, const void *src, const int n, void *start)
{
    int diff;
    struct pool_node_s *node;

    node = get_meta(start);

    diff = dst - (node->ptr + sizeof(void *));

    //printf("memcpy diff %d\n", node->size);

    if(n + diff > node->size) {
        hm_log(LOG_ERR, node->pool->log, "{Pool}: illegal memcpy(), overlapping dst of %d with %d bytes", node->size, diff + n);
        return NULL;
    } else {
        return memcpy(dst, src, n);
    }
}

static struct hm_pool_s *pool_create_append(struct hm_pool_s *pool, const int size)
{
    struct hm_pool_s *p, *tp;

    p = malloc(sizeof(*p));

    if(p == NULL) {
        return NULL;
    }

    /** set pool size */
    p->size = size;
    p->next = NULL;
    p->freenode = NULL;
    p->buckets = NULL;
    p->used = 0;
    p->log = pool->log;

    if(pool_create_bucket(p) != 0) {
        return NULL;
    }

    for(tp = pool; tp != NULL; tp = tp->next) {
        if(tp->next == NULL) {
            tp->next = p;
            break;
        }
    }

    return p;
}

/**
  The  realloc()  function  changes the size of the memory block pointed to by ptr to size bytes.  The contents
  will be unchanged in the range from the start of the region up to the minimum of the old and new  sizes.   If
  the new size is larger than the old size, the added memory will not be initialized.  If ptr is NULL, then the
  call is equivalent to malloc(size), for all values of size; if size is equal to zero, and ptr  is  not  NULL,
  then  the call is equivalent to free(ptr).  Unless ptr is NULL, it must have been returned by an earlier call
  to malloc(), calloc() or realloc().  If the area pointed to was moved, a free(ptr) is done.
  */
void *hm_prealloc(struct hm_pool_s *pool, void *ptr, const int size)
{
#ifdef POOL_STDLIB
    return realloc(ptr, size);
#endif

    struct pool_node_s *node = NULL;
    void *dst;

    if(ptr != NULL) {
        node = get_meta(ptr);
    }

    if(ptr == NULL && size == 0) {
#ifdef POOL_DEBUG
        hm_log(LOG_MEMORY, pool->log, "{Pool}: doing nothing");
#endif
        /** do nothing */
        return NULL;
    } else if(ptr == NULL && size != 0) {
#ifdef POOL_DEBUG
        hm_log(LOG_MEMORY, pool->log, "{Pool}: malloc() size: %d", size);
#endif
        /** malloc() */
        return hm_palloc(pool, size);
    } else if(ptr != NULL && size == 0) {
#ifdef POOL_DEBUG
        hm_log(LOG_MEMORY, pool->log, "{Pool}: free() size: %d", size);
#endif
        /** free() */
        if(hm_pfree(pool, ptr) != 0) {
            return NULL;
        }

        return NULL;
    } else if(ptr != NULL && node != NULL && node->size == size) {
#ifdef POOL_DEBUG
        hm_log(LOG_MEMORY, pool->log, "{Pool}: status quo for size: %d", size);
#endif
        /** nothing needs to be changed - return exactly the same pointer */
        return ptr;
    } else if(ptr != NULL && size > 0 && node != NULL && node->size != size) {
#ifdef POOL_DEBUG
        hm_log(LOG_MEMORY, pool->log, "{Pool}: realloc for old size: %d new size: %d", node->size, size);
#endif
        /** realloc() */

        /** first allocate new dst */
        dst = hm_palloc(pool, size);

        if(dst == NULL) {
            return NULL;
        }

        /** copy src to dst */
        if(node->realsize <= size) {
            memcpy(dst, ptr, node->realsize);
        } else {
            memcpy(dst, ptr, size);
        }

        /** free src */
        hm_pfree(pool, ptr);

        return dst;
    }

    return NULL;
}

void *hm_palloc(struct hm_pool_s *pool, int size)
{
#ifdef POOL_STDLIB
    return malloc(size);
#endif

    struct hm_pool_s *p;

    assert(pool);

    for(p = pool; p != NULL; p = p->next) {
        /** do we match an existing pool */
        if(ROUND16(size) == p->size) {
#ifdef POOL_DEBUG
            hm_log(LOG_MEMORY, pool->log, "{Pool}: found existing pool: %p/%d", p, p->size);
#endif
            return pool_get_node(p, size);
        }
    }

    p = pool_create_append(pool, ROUND16(size));

    if(p == NULL) {
        return NULL;
    }

#ifdef POOL_DEBUG
    hm_log(LOG_MEMORY, pool->log, "{Pool}: creating new pool with parent pool: %p/%d", p, p->size);
#endif

    return pool_get_node(p, size);
}

int hm_destroy_pool(struct hm_pool_s *pool)
{
    struct hm_pool_s *p, *pd;
    struct pool_bucket_s *b, *bd;

    for(p = pool; p != NULL; ) {
        for(b = p->buckets; b != NULL; ) {
            free(b->memory_region);
            free(b->nodes);

            bd = b;
            b = b->next;
            free(bd);
        }
        pd = p;
        p = p->next;
        free(pd);
    }

    return 0;
}

void pool_info(struct hm_pool_s *pool)
{
    struct hm_pool_s *p;

    for(p = pool; p != NULL; p = p->next) {
        printf("pool size: %d used: %d\n", p->size, p->used);
    }
}
