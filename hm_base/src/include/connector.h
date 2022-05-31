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
#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#define CONN_F_CONNECTED    0x1 /**< connected */
#define CONN_F_FDOPEN       0x2 /**< keep FD open */
#define CONN_F_SINGLECHECK    0x4 /**< interrupt watcher when single client connection est */
#define CONN_F_COUCHBASE    0x8 /**< couchbase connection */

struct conn_data_s;
typedef void (*conn_cb_t)(struct conn_data_s *c);

struct conn_node_s {
    struct conn_data_s *parent;
    int group;          /**< Group ID */
    char host[128];     /**< Hostname that must be recognized by inet_addr */
    int port;           /**< Port number */

    int fd;             /**< File descriptor */

    unsigned int status;         /**< Hostname:port's flag status */

    struct ev_io io;    /**< Event that is pushed to an external event loop. */
    struct ev_timer timer;  /**< Timer which stops 'io'. */

    unsigned long long logs;

    void *data; /**< Passed data */

    struct hm_log_s *log;
};

struct conn_data_s {
    struct ev_loop *loop;   /**< An external event loop. */
    struct hm_pool_s *pool;
    struct hm_log_s *log;

    unsigned long long logs;

    struct conn_node_s **src;   /**< An array of conn_node_s to be watched. */
    int size;           /**< Number of connectors to watch. */
    int established;
    conn_cb_t callback; /**< Callback function to call when watcher finishes all jobs. */
    struct ev_timer timer;  /**< Timer which finishes all jobs and invokes callback. */
    float exptime;    /**< Expiration time which terminates all jobs */
    float timeout;    /**< Timeout for specific host */
};

int connector(struct conn_data_s *c);
void connector_fd_close(int fd);
void connector_free(struct conn_data_s *c);
int setkeepalive(int fd) ;

#endif
