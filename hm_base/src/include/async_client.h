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
#ifndef HMCLIENT_H_
#define HMCLIENT_H_

#define hm_send(c, conn_buf, conn_len)\
    /** Only respond if client is still active */\
if(c) {\
    rb_send_init(&c->rb, (char *)conn_buf, conn_len, c->pool);\
    assert(c->loop);\
    ev_io_start(c->loop, &c->write);\
}

struct conn_client_s;

enum clerr_e {
    CL_NOERROR = 1,
    CL_HANGTIMEOUT_ERR,
    CL_WANTSHUTDOWN_ERR,
    CL_READRBFULL_ERR,
    CL_READZERO_ERR,
    CL_READ_ERR,
    CL_WRITE_ERR,
    CL_BUFFERFULL_ERR,
    CL_PACKETLEN_ERR,
    CL_PACKETEXPECT_ERR,
    CL_SERVERSHUTDOWN_ERR,
    CL_SOCKET_ERR
};

struct conn_client_holder_s {
    int signal_shutdown;                ///< signal from client struct if it's already gone
    struct conn_client_s *client;       ///< actual client
    struct conn_client_holder_s *next;  ///< next client
};

struct conn_server_s {
    struct ev_loop *loop;   /**< An external event loop. */
    struct hm_pool_s *pool;
    struct hm_log_s *log;

    struct conn_client_holder_s *clients_head;
    int clients;

    struct ev_io listener;
    int fd;

    const char *host;
    const char *port;

    void (*recv)(struct conn_client_s *data, const char *buf, const int len);
    void (*client_dc)(void *data, const char *foreign_client_index, const char *hbs_id);
    void (*shutdown)();
};

struct watcher_s {
    struct conn_client_s *p1;
    struct conn_client_s *p2;
    int counter;
    int target;
};

struct player_deck_s {
    char code[16];
    int count;
    struct player_deck_s *next;
};

struct conn_client_s {
    struct ev_loop *loop;
    struct hm_pool_s *pool;
    struct hm_log_s *log;

    struct ev_io read;
    struct ev_io write;
    struct ev_timer hang_timer;

    int hang_timer_enabled;     ///< TODO: move this to flag
    int monitor;                ///< TODO: move this to flag

    unsigned long long logs;    /**< log sequence number */

    int fd;
    int *shutdown_signal_holder;

    int want_shutdown;
    int type;

    struct rb_s rb;

    char net_buf[RB_SLOT_SIZE];
    int net_nbuf;
    int net_expect;

    char client_index[32];
    char foreign_client_index[32];

    char ip[64];
    int nip;
    int port;

    char date[32];

    void (*recv)(struct conn_client_s *client);
    void (*error_callback)(struct conn_client_s *client, enum clerr_e error);
    void (*client_dc)(void *data, const char *foregin_client_index, const char *hbs_id);

    void *data;

    struct conn_server_s *parent;
    struct conn_client_holder_s *holder;

    char hbs_id[16];
#ifdef HM_GAMESERVER
    struct player_deck_s *cards;
    int ncards;
    char hero[16];
    char hp[16];
#elif defined HM_LOBBYSERVER
    char token[32];
    int ntoken;

    struct {
        char user[32];
        char pass[32];
        char secret[32];
    } login;

    int skip;
#endif
};

struct ht_s **async_clients;      // ht of async clients

int async_shutdown_client(struct conn_client_s *c);
void shutdown_server(struct conn_server_s *cs);
int connector_server(struct conn_server_s *cs);

#endif
