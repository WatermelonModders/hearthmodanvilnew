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
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <libcouchbase/couchbase.h>
#include <libcouchbase/views.h>
#include <ev.h>

#include <hmbase.h>
#include <connector.h>
#include <hmcouchbase.h>

// TODO: even though these globals can stay here, move them elsewhere in the future
static lcb_t *active_instance;
static struct conn_node_s **c;
static struct conn_data_s *cd;
static int BUCKETS_PER_CLUSTER;
static int HOSTS;
static int grcount = 0; /**< groups counter */
static void (*callback)(struct instance_s *) = NULL;
static struct instance_s *instances = NULL;
static struct connstr_s **groups = NULL;
static struct hm_log_s *cb_log = NULL;

static void error_callback(lcb_t instance, struct cbop_s *cbop, lcb_error_t error);
static int is_active_instance(lcb_t instance);
static void switch_instances(lcb_t instance);
static void bootstrap_callback(lcb_t instance, lcb_error_t err);
static void store_callback(lcb_t instance, const void *cookie,
        lcb_storage_t operation, lcb_error_t error, const lcb_store_resp_t *resp);
static void remove_callback(lcb_t instance, const void *cookie, lcb_error_t error, const lcb_remove_resp_t *resp);
static void arithmetic_callback(lcb_t instance, const void *cookie, lcb_error_t error, const lcb_arithmetic_resp_t *resp);
static void get_callback(lcb_t instance, const void *cookie, lcb_error_t error, const lcb_get_resp_t *resp);
static int cb_get(struct cbop_s *cbop);
static int cb_arithmetic(struct cbop_s *cbop);
static int cb_remove(struct cbop_s *cbop);
static int cb_store(struct cbop_s *cbop);
static int cb_view(struct cbop_s *cbop);
static int storage_init(struct conn_data_s *cd, struct conn_node_s *cn, struct connstr_s *cs, const int idx );
static void connector_cb(struct conn_data_s *c);

static void error_callback(lcb_t instance, struct cbop_s *cbop, lcb_error_t error)
{
    hm_log(LOG_ERR, cb_log, "{Couchbase}: error 0x%x", error);

    // network related errors
    if(error == LCB_NETWORK_ERROR || error == LCB_ETIMEDOUT || error == LCB_ENETUNREACH) {
        if(is_active_instance(instance) == CB_ERR_OK) {
            // switch instances and try again
            // TODO: keep track of switching to avoid dead lock if all groups are offline
            switch_instances(instance);
            couchbase_operation(cbop);
        }
    }
}

#define TYPE(T)\
    if(resp->version == CB_QUERY_VER) {\
        cbop->lcb_error = error;\
        cbop->u.T.r = resp;\
        if(cbop->post) {\
            cbop->post(cbop);\
        }\
    }

static void store_callback(lcb_t instance, const void *cookie,
        lcb_storage_t operation, lcb_error_t error, const lcb_store_resp_t *resp)
{
    struct cbop_s *cbop = (struct cbop_s *)cookie;

    if(error == LCB_SUCCESS || error == LCB_KEY_ENOENT || error == LCB_KEY_EEXISTS || (error == LCB_ETMPFAIL && CBSQ(cas) != 0)) {
        TYPE(s)
    } else {
        error_callback(instance, cbop, error);
    }
}

static void remove_callback(lcb_t instance, const void *cookie, lcb_error_t error, const lcb_remove_resp_t *resp)
{
    struct cbop_s *cbop = (struct cbop_s *)cookie;

    if(error == LCB_SUCCESS || error == LCB_KEY_ENOENT || (error == LCB_ETMPFAIL && CBRQ(cas) != 0)) {
        TYPE(r)
    } else {
        error_callback(instance, cbop, error);
    }
}

static void arithmetic_callback(lcb_t instance, const void *cookie, lcb_error_t error, const lcb_arithmetic_resp_t *resp)
{
    struct cbop_s *cbop = (struct cbop_s *)cookie;

    if(error == LCB_SUCCESS || error == LCB_KEY_ENOENT) {
        TYPE(a)
    } else {
        error_callback(instance, cbop, error);
    }
}

static void get_callback(lcb_t instance, const void *cookie, lcb_error_t error, const lcb_get_resp_t *resp)
{
    struct cbop_s *cbop = (struct cbop_s *)cookie;

    if(error == LCB_SUCCESS || error == LCB_KEY_ENOENT) {
        TYPE(g)
    } else {
        error_callback(instance, cbop, error);
    }
}

static int cb_get(struct cbop_s *cbop)
{
    lcb_error_t error;
    const lcb_get_cmd_t *commands[] = { &(cbop->u.g.q) };

    if(0) hm_log(LOG_DEBUG, cb_log, "{Couchbase}: get [%.*s]", (int)commands[0]->v.v0.nkey, (char*)commands[0]->v.v0.key);
    error = lcb_get(active_instance[cbop->bucket], cbop, 1, commands);
    if(error != LCB_SUCCESS) {
        hm_log(LOG_ERR, cb_log, "{Couchbase}: couchbase operation failed with error: %d", error);
        return error;
    }

    return CB_ERR_OK;
}

static int cb_arithmetic(struct cbop_s *cbop)
{
    lcb_error_t error;
    const lcb_arithmetic_cmd_t *cmds[] = { &(cbop->u.a.q) };

    error = lcb_arithmetic(active_instance[cbop->bucket], cbop, 1, cmds);
    if(error != LCB_SUCCESS) {
        hm_log(LOG_ERR, cb_log, "{Couchbase}: couchbase operation failed with error: %d", error);
        return error;
    }

    return CB_ERR_OK;
}

static int cb_remove(struct cbop_s *cbop)
{
    lcb_error_t error;
    const lcb_remove_cmd_t *cmds[] = { &(cbop->u.r.q) };

    error = lcb_remove(active_instance[cbop->bucket], cbop, 1, cmds);
    if(error != LCB_SUCCESS) {
        hm_log(LOG_ERR, cb_log, "{Couchbase}: couchbase operation failed with error: %d", error);
        return error;
    }

    return CB_ERR_OK;
}

static int cb_store(struct cbop_s *cbop)
{
    lcb_error_t error;
    const lcb_store_cmd_t *cmds[] = { &(cbop->u.s.q) };

    if(1 == 1) hm_log(LOG_DEBUG, cb_log, "{Couchbase}: set [%.*s], cas=0x%lx [%.*s]", (int)cmds[0]->v.v0.nkey, (char*)cmds[0]->v.v0.key, cmds[0]->v.v0.cas, (int)cmds[0]->v.v0.nbytes, (char*)cmds[0]->v.v0.bytes);
    printf("bucket %p\n", active_instance[cbop->bucket]);
    error = lcb_store(active_instance[cbop->bucket], cbop, 1, cmds);

    printf("bucket %p\n", active_instance[cbop->bucket]);
    if (error != LCB_SUCCESS) {
        hm_log(LOG_ERR, cb_log, "{Couchbase}: couchbase operation failed with error: %d", error);
        return error;
    }

    printf("error %d\n", error);

    return CB_ERR_OK;
}

static int cb_view(struct cbop_s *cbop)
{
    lcb_error_t error;

    error = lcb_view_query(active_instance[cbop->bucket], cbop, &(cbop->u.v.q));
    if (error != LCB_SUCCESS) {
        hm_log(LOG_ERR, cb_log, "{Couchbase}: couchbase operation failed with error: %d", error);
        return error;
    }

    return CB_ERR_OK;
}

/**< @brief
 *
 * Natural selection based on network latency, whichever instance comes first, takes 0 index
 */
static void set_instance(lcb_t instance)
{
    const void *cookie;
    struct instance_s *ins;

    cookie = lcb_get_cookie(instance);
    ins = (struct instance_s *)cookie;

    if(active_instance[ins->index] == NULL) {
        active_instance[ins->index] = instance;
        hm_log(LOG_DEBUG, ins->log, "{Couchbase}: New instance on index %d from group: %d : %p", ins->index, ins->group, instance);
    }
}

// broken instance is set as active?
static int is_active_instance(lcb_t instance)
{
    int i;

    for(i = 0; i < BUCKETS_PER_CLUSTER; i++) {
        if(active_instance[i] == instance) {
            return CB_ERR_OK;
        }
    }

    return CB_ERR_NOACTIVE;
}

static void switch_instances(lcb_t instance)
{
    int i, j, k;
    const void *cookie;
    struct instance_s *ins;

    cookie = lcb_get_cookie(instance);
    ins = (struct instance_s *)cookie;

    for(i = 0; i < HOSTS; i++) {
        for(j = 0; j < BUCKETS_PER_CLUSTER; j++) {
            if(((struct cluster_s *)(cd->src[i]->data))->instance[j] != instance &&
                    cd->src[i]->group != ins->group ) {
                for(k = 0; k < BUCKETS_PER_CLUSTER; k++) {
                    hm_log(LOG_DEBUG, ins->log, "{Couchbase}: old instance: %p, new instance: %p, index: %d new host: %s", active_instance[k], ((struct cluster_s *)(cd->src[i]->data))->instance[k], k, cd->src[i]->host);
                    active_instance[k] = ((struct cluster_s *)(cd->src[i]->data))->instance[k];
                }
                return;
            }
        }
    }
}

static void bootstrap_callback(lcb_t instance, lcb_error_t err)
{
    static int counter = 0;
    const void *cookie;
    struct instance_s *ins;

    cookie = lcb_get_cookie(instance);
    ins = (struct instance_s *)cookie;

    if(err == LCB_SUCCESS) {
        ins->status = 1;
        set_instance(instance);
    } else {
        ins->status = 0;
        hm_log(LOG_ERR, ins->log, "{Couchbase}: Bootstrap error 0x%x", err);
    }

    ins->next = instances;
    instances = ins;

    if(++counter >= (BUCKETS_PER_CLUSTER * grcount) && callback) {
        callback(instances);
    }
}

static void destroy_callback(const void *cookie)
{
    struct instance_s *ins;

    ins = (struct instance_s *)cookie;

    ///< this causes SIGSEGV
    //lcb_destroy_io_ops(ins->ioops);

    hm_pfree(ins->pool, ins);
}

static int storage_init(struct conn_data_s *cd, struct conn_node_s *cn, struct connstr_s *cs, const int idx)
{
    lcb_error_t error;
    struct lcb_create_io_ops_st ciops;
    struct lcb_create_st copts;
    char connstr[CONNSTR_SIZE];
    struct instance_s *ins;
    struct cluster_s *cl = cn->data;
    int nconnstr;

    memset(&ciops, 0, sizeof(ciops));
    memset(&copts, 0, sizeof(copts));
    ins = hm_palloc(cd->pool, sizeof(*ins));

    ciops.v.v0.type = LCB_IO_OPS_LIBEV;
    ciops.v.v0.cookie = cd->loop;

    assert(cl != NULL);

    error = lcb_create_io_ops(&ins->ioops, &ciops);
    if(error != LCB_SUCCESS) {
        hm_pfree(cd->pool, ins);
        return CB_ERR_LCBCREATEIO;
    }

    snprintf(connstr, sizeof(connstr), "couchbase://%.*s/%s", cs->len - 1, cs->host, cl->bucket[idx]);
    hm_log(LOG_DEBUG, cd->log, "{Connector}: connstr: [%s]", connstr);
    copts.version = 3;
    copts.v.v3.connstr = connstr;
    copts.v.v3.username = cl->bucket[idx];
    copts.v.v3.passwd = cl->passwd[idx];
    copts.v.v3.io = ins->ioops;

    error = lcb_create(&cl->instance[idx], &copts);
    if(error != LCB_SUCCESS) {
        hm_pfree(cd->pool, ins);
        return CB_ERR_LCBCREATE;
    }

    nconnstr = strlen(connstr);
    ins->group = cs->id;
    ins->index = idx;
    memcpy(ins->connstr, connstr, nconnstr);
    ins->connstr[nconnstr] = 0;
    ins->loop = cd->loop;
    ins->log = cd->log;
    ins->pool = cd->pool;
    lcb_set_cookie(cl->instance[idx], ins);

    lcb_set_bootstrap_callback(cl->instance[idx], bootstrap_callback);
    lcb_set_get_callback(cl->instance[idx], get_callback);
    lcb_set_store_callback(cl->instance[idx], store_callback);
    lcb_set_remove_callback(cl->instance[idx], remove_callback);
    lcb_set_arithmetic_callback(cl->instance[idx], arithmetic_callback);
    lcb_set_destroy_callback(cl->instance[idx], destroy_callback);

    if((error = lcb_connect(cl->instance[idx])) != LCB_SUCCESS) {
        lcb_destroy(cl->instance[idx]);
        hm_pfree(cd->pool, ins);
        return CB_ERR_LCBCONNECT;
    }

    return CB_ERR_OK;
}

static void buildconnstr(struct conn_data_s *c, struct connstr_s **groups, const char *host, const int group)
{
    int len;

    if(group >= MAX_GROUPS || group < 0) {
        hm_log(LOG_ERR, c->log, "{Couchbase}: groups count error: %d", group);
        return;
    }

    if(groups[group] == NULL) {
        groups[group] = hm_palloc(c->pool, sizeof(struct connstr_s));
        groups[group]->host = hm_palloc(c->pool, CONNSTR_SIZE);       /**< max size */
        groups[group]->len = strlen(host);
        groups[group]->id = group;

        memcpy(groups[group]->host, host, groups[group]->len);
        groups[group]->host[groups[group]->len++] = ',';     /**< connstr delimiter */

        grcount++;
    } else {
        len = strlen(host);
        memcpy(&(groups[group]->host[groups[group]->len]), host, len);
        groups[group]->len += len;
        groups[group]->host[groups[group]->len++] = ',';     /**< connstr delimiter */
    }
}

static void connector_cb(struct conn_data_s *c)
{
    int i, j, error, connected = 0;

    for(i = 0; i < c->size; i++) {
        if(c->src[i] && (c->src[i]->status & CONN_F_CONNECTED)) {
            hm_log(LOG_DEBUG, c->log, "{Connector}: Connected to %s:%d", c->src[i]->host, c->src[i]->port);
            buildconnstr(c, groups, c->src[i]->host, c->src[i]->group);
            connected = 1;
        } else {
            hm_log(LOG_NOTICE, c->log, "{Connector}: Failed to connect to %s:%d", c->src[i]->host, c->src[i]->port);
        }
    }

    /**< No available hosts found */
    if(connected != 1) {
        callback(NULL);
        return;
    }

    for(i = 0; i < MAX_GROUPS; i++) {
        if(groups[i] != NULL) {
            for(j = 0; j < BUCKETS_PER_CLUSTER; j++) {
                error = storage_init(c, c->src[i], groups[i], j);
                if(error != CB_ERR_OK) {
                    hm_log(LOG_ERR, c->log, "{Couchbase}: storage_init failed for %s:%d with code 0x%x", c->src[i]->host, c->src[i]->port, error);
                }
            }
        }
    }
}

int couchbase_init(struct couchbase_data_s *cbd)
{
    int i, j;
    struct cluster_s *cl;

    groups = hm_palloc(cbd->pool, sizeof(void *) * MAX_GROUPS);
    memset(groups, 0, sizeof(void *) * MAX_GROUPS);

    BUCKETS_PER_CLUSTER = cbd->nbuckets;
    HOSTS = cbd->nhosts;

    assert(cbd->pool != NULL && cbd->log != NULL);

    cd = hm_palloc(cbd->pool, sizeof(*cd));
    cd->size = HOSTS;
    cd->loop = cbd->loop;
    cd->callback = connector_cb;
    cd->log = cbd->log;
    cd->pool = cbd->pool;
    cd->timeout = 1.0;
    cd->exptime = 2.0;
    callback = cbd->callback;
    cb_log = cbd->log;

    active_instance = hm_palloc(cd->pool, BUCKETS_PER_CLUSTER * sizeof(lcb_t));
    for(i = 0; i < BUCKETS_PER_CLUSTER; i++) {
        active_instance[i] = NULL;
    }

    c = hm_palloc(cd->pool, HOSTS * sizeof(void *));
    for( i = 0; i < HOSTS; i++ ) {
        c[i] = hm_palloc(cd->pool, sizeof(*(c[i])));
        bzero(c[i], sizeof(*c[i]));
        c[i]->port = 8091;
        c[i]->status = CONN_F_COUCHBASE;   /**< Couchbase opens its own FD */
        c[i]->group = cbd->hgroups[i];

        memcpy( c[i]->host, cbd->hosts[i], strlen(cbd->hosts[i])+1 );

        cl = hm_palloc(cd->pool, sizeof(*cl));
        cl->bucket = hm_palloc(cd->pool, sizeof(void *) * BUCKETS_PER_CLUSTER);
        cl->passwd = hm_palloc(cd->pool, sizeof(void *) * BUCKETS_PER_CLUSTER);
        cl->instance = hm_palloc(cd->pool, sizeof(lcb_t) * BUCKETS_PER_CLUSTER);
        memset(cl->instance, 0, sizeof(lcb_t) * BUCKETS_PER_CLUSTER);

        for( j = 0; j < BUCKETS_PER_CLUSTER; j++ ) {
            cl->bucket[j] = hm_palloc(cd->pool, strlen(cbd->buckets[j])+1);
            cl->passwd[j] = hm_palloc(cd->pool, strlen(cbd->bpasswd[j])+1);
            memcpy(cl->bucket[j], cbd->buckets[j], strlen(cbd->buckets[j])+1);
            memcpy(cl->passwd[j], cbd->bpasswd[j], strlen(cbd->bpasswd[j])+1);
        }

        c[i]->data = cl;
    }

    cd->src = c;
    connector( cd );

    return CB_ERR_OK;
}

int couchbase_bucket_index(const char *bucketName) {
    if(HOSTS == 0) {
        return -1;
    }

    struct cluster_s *cl = (struct cluster_s*)c[0]->data;
    int i;
    for(i=0; i<BUCKETS_PER_CLUSTER; i++) {
        if(strcmp(cl->bucket[i], bucketName) == 0) {
            return i;
        }
    }
    return -1;
}

int couchbase_operation(struct cbop_s *cbop)
{
    // TODO: check if bucket index in range
    if(active_instance[cbop->bucket] == NULL) {
        hm_log(LOG_ERR, cb_log, "{Couchbase}: No active instance for bucket: %d", cbop->bucket);
        return CB_ERR_NOINSTANCE;
    }

    switch(cbop->operation) {
        case CB_GET:
            return cb_get(cbop);
            break;
        case CB_STORE:
            return cb_store(cbop);
            break;
        case CB_REMOVE:
            return cb_remove(cbop);
            break;
        case CB_ARITH:
            return cb_arithmetic(cbop);
            break;
        case CB_VIEW:
            return cb_view(cbop);
            break;
        default:
            hm_log(LOG_ERR, cb_log, "{Couchbase}: Operation not supported: %d", cbop->operation);
            return CB_ERR_OPNOT;
            break;
    }
}

void couchbase_deinit()
{
    int i, j;
    //lcb_t instance;
    struct cluster_s *cl;
    const void *cookie;

    for(i = 0; i < MAX_GROUPS; i++) {
        if(groups[i]) {
            hm_pfree(cd->pool, groups[i]->host);
            hm_pfree(cd->pool, groups[i]);
        }
    }
    hm_pfree(cd->pool, groups);     /**< groups */
    hm_pfree(cd->pool, active_instance); /**< active instances */

    for(i = 0; i < HOSTS; i++) {
        cl = cd->src[i]->data;

        for(j = 0; j < BUCKETS_PER_CLUSTER; j++) {
            hm_pfree(cd->pool, cl->bucket[j]);
            hm_pfree(cd->pool, cl->passwd[j]);
            if(cl->instance[j]) {
                cookie = lcb_get_cookie(cl->instance[j]);
                lcb_destroy_async(cl->instance[j], cookie);
                //lcb_destroy_io_ops(((struct instance_s *)cookie)->ioops);
            }
        }
        hm_pfree(cd->pool, cl->bucket);
        hm_pfree(cd->pool, cl->passwd);
        hm_pfree(cd->pool, cl->instance);
        hm_pfree(cd->pool, cl);
    }

    connector_free(cd);
}
