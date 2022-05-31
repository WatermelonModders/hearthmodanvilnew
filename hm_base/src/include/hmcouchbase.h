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
#ifndef HMCOUCHBASE_H_
#define HMCOUCHBASE_H_

#include <libcouchbase/views.h>

#define CB_ERR_OK           0x0     /**< No error */
#define CB_ERR_LCBCREATEIO  0x1     /**< Failed to lcb_create_io_ops() */
#define CB_ERR_LCBCREATE    0x2     /**< Failed to lcb_create() */
#define CB_ERR_LCBCONNECT   0x3     /**< Failed to lcb_connect() */
#define CB_ERR_NOINSTANCE   0x4     /**< No active instance found */
#define CB_ERR_NOACTIVE     0x5     /**< Instance is not active */
#define CB_ERR_OPNOT        0x6     /**< Operation not supporeted */
#define CB_ERR_OPERR        0x7     /**< Operation error */

#define CB_QUERY_VER    0

#define CBVQ(m) cbop->u.v.q.m // couchbase view query
#define CBSQ(m) cbop->u.s.q.v.v0.m // couchbase store query
#define CBRQ(m) cbop->u.r.q.v.v0.m // couchbase remove query
#define CBGQ(m) cbop->u.g.q.v.v0.m // couchbase get query
#define CBGR(m) cbop->u.g.r->v.v0.m // CouchBase Get Reply
#define CBRR(m) cbop->u.r.r->v.v0.m // couchbase remove reply
#define CBSR(m) cbop->u.s.r->v.v0.m // couchbase store reply
#define CBAQ(m) cbop->u.a.q.v.v0.m // couchbase arithmetic query
#define CBAR(m) cbop->u.a.r->v.v0.m // couchbase arithmetic reply

/**< CBSQ_V0 - CouchBase Store Query V0
 * bucket, operation, key, nkey, bytes, nbytes, callback, cas, exptime, datatype, flags
 * 'struct cbop_s *cbop' must exist within context
 */
#define CBSQ_V0(b, o, k, nk, bt, nb, f, c, e, d, flag)\
    cbop->bucket = b;\
cbop->operation = CB_STORE;\
cbop->u.s.q.version = CB_QUERY_VER;\
CBSQ(operation) = o;\
CBSQ(key) = k;\
CBSQ(nkey) = nk;\
CBSQ(bytes) = bt;\
CBSQ(nbytes) = nb;\
CBSQ(cas) = c;\
CBSQ(exptime) = e;\
CBSQ(datatype) = d;\
CBSQ(flags) = flag;\
cbop->post = f;\
(void)couchbase_operation(cbop);
/*if(couchbase_operation(cbop) != CB_ERR_OK) { */


/**< CBGQ_V0 - CouchBase Get Query V0
 * bucket, key, nkey, callback, exptime, lock
 * 'struct cbop_s *cbop' must exist within context
 */
#define CBGQ_V0(b, k, nk, f, e, l)\
    cbop->bucket = b;\
cbop->operation = CB_GET;\
cbop->u.g.q.version = CB_QUERY_VER;\
CBGQ(key) = k;\
CBGQ(nkey) = nk;\
CBGQ(exptime) = e;\
CBGQ(lock) = l;\
cbop->post = f;\
(void)couchbase_operation(cbop);
/*if(couchbase_operation(cbop) != CB_ERR_OK) { */

/**< CBRQ_V0 - CouchBase Remove Query V0
 * bucket, key, nkey, callback, cas
 * 'struct cbop_s *cbop' must exist within context
 */
#define CBRQ_V0(b, k, nk, f, c)\
    cbop->bucket = b;\
cbop->operation = CB_REMOVE;\
cbop->u.r.q.version = CB_QUERY_VER;\
CBRQ(key) = k;\
CBRQ(nkey) = nk;\
CBRQ(cas) = c;\
cbop->post = f;\
(void)couchbase_operation(cbop);
/*if(couchbase_operation(cbop) != CB_ERR_OK) { */

/**< CBAQ_V0 - CouchBase Arithmetic Query V0
 * bucket, key, nkey, callback, create, delta, initial
 * 'struct cbop_s *cbop' must exist within context
 */
#define CBAQ_V0(b, k, nk, f, c, d, i)\
    cbop->bucket = b;\
cbop->operation = CB_ARITH;\
cbop->u.a.q.version = CB_QUERY_VER;\
CBAQ(key) = k;\
CBAQ(nkey) = nk;\
CBAQ(create) = c;\
CBAQ(delta) = d;\
CBAQ(initial) = i;\
cbop->post = f;\
(void)couchbase_operation(cbop);
/*if(couchbase_operation(cbop) != CB_ERR_OK) { */

/**< CBVQ_V0 - CouchBase View Query V0
 * bucket, cmd, doc, ndoc, view, nview, options, noptions, post, npost, dcm, callback, handle
 * 'struct cbop_s *cbop' must exist within context
 */
#define CBVQ_V0(b, c, d, nd, v, nv, o, no, p, np, dcm, f, h)\
    cbop->bucket = b;\
cbop->operation = CB_VIEW;\
CBVQ(cmdflags) = c;\
CBVQ(ddoc) = d;\
CBVQ(nddoc) = nd;\
CBVQ(view) = v;\
CBVQ(nview) = nv;\
CBVQ(optstr) = o;\
CBVQ(noptstr) = no;\
CBVQ(postdata) = p;\
CBVQ(npostdata) = np;\
CBVQ(docs_concurrent_max) = dcm;\
CBVQ(callback) = f;\
CBVQ(handle) = h;\
(void)couchbase_operation(cbop);
/*if(couchbase_operation(cbop) != CB_ERR_OK) { */

#define CONNSTR_SIZE    2048
#define MAX_GROUPS  16 /**< max couchbase datacenters */

typedef enum {
    CB_GET = 0x1,
    CB_STORE = 0x2,
    CB_REMOVE = 0x3,
    CB_ARITH = 0x4,
    CB_VIEW = 0x5
} cboperation_t;

/*
   typedef enum {
   LCB_ADD = 0x01,
   LCB_REPLACE = 0x02,
   LCB_SET = 0x03,
   LCB_APPEND = 0x04,
   LCB_PREPEND = 0x05
   } lcb_storage_t;
   */

/*
   syslog priorities:
   LOG_EMERG      system is unusable
   LOG_ALERT      action must be taken immediately
   LOG_CRIT       critical conditions
   LOG_ERR        error conditions
   LOG_WARNING    warning conditions
   LOG_NOTICE     normal, but significant, condition
   LOG_INFO       informational message
   LOG_DEBUG      debug-level message
   */

/**
 * @brief cbop_s structure is passed to every couchbase request
 *
 * User must allocate and deallocate space for this structure.
 */
struct cbop_s {
    int bucket;  /**< Index of bucket */
    cboperation_t operation;  /**< Couchbase operation, @see cboperation_t */
    void (*post)(struct cbop_s *cbop);
    lcb_error_t lcb_error; /**< Libcouchbase error code in case callback function needs it */

    union {
        struct {
            lcb_get_cmd_t q;  /**< Query */
            const lcb_get_resp_t *r;  /**< Response */
        } g; /**< This is a Get procedure definition. */

        struct {
            lcb_store_cmd_t q; /**< Query */
            const lcb_store_resp_t *r;  /**< Response */
        } s; /**< This is a Store procedure definition. */

        struct {
            lcb_remove_cmd_t q; /**< Query */
            const lcb_remove_resp_t *r;   /**< Response */
        } r; /**< This is a Remove procedure definition. */

        struct {
            lcb_arithmetic_cmd_t q;  /**< Query */
            const lcb_arithmetic_resp_t *r;  /**< Response */
        } a; /**< This is an Arithmetic procedure definition. */

        struct {
            lcb_CMDVIEWQUERY q;  /**< Query */
            struct lcb_RESPVIEW_st r; /**< Response */
        } v; /**< This is a View procedure definition. */
    } u;

    void *g_child; /** grunt's child */
    void *data; /**< Some other client's data. */
};

/**
 * @brief struct cluster_s
 *
 * Every bucket has its own instance.
 */
struct cluster_s {
    char **bucket;
    char **passwd;
    lcb_t *instance;

};

struct couchbase_data_s;
/**
 * @brief struct instance_s
 *
 * Every cluster has its own group.
 */
struct instance_s {
    struct ev_loop *loop;
    struct hm_log_s *log;
    struct hm_pool_s *pool;
    char connstr[CONNSTR_SIZE];
    int status;
    int group;
    int index;
    lcb_io_opt_t ioops;
    struct instance_s *next;
};

/**
 * @brief struct couchbase_data_s
 *
 */
struct couchbase_data_s {
    struct ev_loop *loop;
    struct hm_log_s *log;
    struct hm_pool_s *pool;
    void (*callback)(struct instance_s *data);

    const char **hosts;
    const int *hgroups;
    int nhosts;
    const char **buckets;
    const char **bpasswd;
    int nbuckets;
};

struct connstr_s {
    int id;
    char *host;
    int len;
};

int couchbase_init(struct couchbase_data_s *cbd);
int couchbase_bucket_index(const char *bucketName);  ///< Returns the bucket index, or -1 if there is no such bucket
int couchbase_operation(struct cbop_s *cbop);
void couchbase_deinit();
extern int couchbase_deinit_async;

#endif
