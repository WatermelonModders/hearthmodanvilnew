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
#ifndef HMPOOL_H_
#define HMPOOL_H_

#define POOL_DEBUG
#define POOL_STDLIB

struct pool_bucket_s {
    void *memory_region;
    void *nodes;
    struct pool_bucket_s *next;
};

struct hm_pool_s {
    int size;
    int used;
    struct hm_log_s *log;
    struct pool_node_s *freenode;
    struct pool_bucket_s *buckets;
    struct hm_pool_s *next;
};

struct pool_node_s {
    void *ptr;
    int size;
    int realsize;
    int used;
    struct hm_pool_s *pool;
    struct pool_node_s *next;
};

struct hm_pool_s *hm_create_pool();
void *hm_palloc(struct hm_pool_s *pool, int size);
void *hm_prealloc(struct hm_pool_s *pool, void *ptr, const int size);
void *hm_memcpy(void *dst, const void *src, const int n, void *start);
int hm_pfree(struct hm_pool_s *pool, void *ptr);
int hm_destroy_pool(struct hm_pool_s *pool);

#endif
