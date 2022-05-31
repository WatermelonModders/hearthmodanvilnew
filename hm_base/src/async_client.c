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
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/fcntl.h> // fcntl
#include <sys/ioctl.h>
#include <unistd.h> // close
#include <ev.h>
#include <errno.h>
#include <time.h>
#include <math.h>

#include <assert.h>

#include <hmbase.h>

#define ASYNC_HANG_TIMEOUT        14

struct ht_s **async_clients;

static unsigned long long client_index = 0;

static int async_client_accept(struct conn_client_s *client);
static void *connector_addclient(struct conn_server_s *cs, struct conn_client_s *cc);

void shutdown_server(struct conn_server_s *cs)
{
    struct conn_client_holder_s *c, *dc;
    struct hm_pool_s *p;

    hm_log(LOG_INFO, cs->log, "{Connector}: shutting down server, clients: %d", cs->clients);

    for(c = cs->clients_head; c != NULL; ) {
        if(c->signal_shutdown == 0 && c->client) {
            c->client->error_callback(c->client, CL_SERVERSHUTDOWN_ERR);
        }

        dc = c;
        c = c->next;
        hm_pfree(cs->pool, dc);
    }

    p = cs->pool;
    ev_io_stop(cs->loop, &cs->listener);
    connector_fd_close(cs->fd);
    hm_pfree(p, cs);
}

static void client_error(struct conn_client_s *c, enum clerr_e err)
{
    async_shutdown_client(c);
}

int setnonblock(int fd)
{
    int nb;

    nb = 1;
    return ioctl(fd, FIONBIO, &nb);
}

#ifdef HM_LOBBYSERVER
static char read_byte(char **dst, const char *end)
{
    char out;

    if(*dst + sizeof(char) > end) {
        return -1;
    }

    out = *((char *)(*dst));

    (*dst)++;
    return out;
}

static int read_uint(char **dst, const char *end)
{
    int num = 0, num2;
    int i;

    for(i = 0; (i < 5 && *dst < end); i++) {
        num2 = read_byte(dst, end);

        if(i == 4 && (num2 & 240) != 0) {
            abort();
        }

        if((num2 & 128) == 0) {
            return num | (unsigned int)((unsigned int)num2 << 7 * i);
        }

        num |= (unsigned int)((unsigned int)(num2 & 127) << 7 * i);
    }

    return num;
}

static void recv_append_lobby(struct conn_client_s *c, char *src, int nsrc)
{
    unsigned short h;
    int i, total;
    struct conn_server_s *cs = c->parent;

    if(nsrc < 2) {
        return;
    }

    hm_log(LOG_DEBUG, c->log, "{Connector}: Received %d bytes", nsrc);

    /** hbs lobby client */
    if(nsrc > 12 && *(int *)src == -1) {
        hm_log(LOG_DEBUG, c->log, "{Connector}: HBS lobby client packet");
        cs->recv(c, src, nsrc);
        return;
    }

    /** hbs game client */
    hm_log(LOG_DEBUG, c->log, "{Connector}: HBS game client packet");
    h = (unsigned char )src[0];
    h <<= 8;
    h |= (unsigned char )src[1];

    if(h == 0) {
        return;
    }

    h += 2;

    for(total = 0, i = 2; i < h; i++) {
        if(src[i] == 0x28 && (i + 1) < h) {
            char *ptr = &src[i + 1];
            total = read_uint(&ptr, ptr + nsrc - h);
            break;
        }
    }

    total += h;

    cs->recv(c, src, total);

    if(total < nsrc) {
        hm_log(LOG_DEBUG, c->log, "{Connector}: Next packet of %d bytes prepared", nsrc - total);
        recv_append_lobby(c, src + total, nsrc - total);
    }
}
#endif	// HM_LOBBYSERVER

static void recv_append(struct conn_client_s *c)
{
#ifdef HM_GAMESERVER
    int sz;
    char *next;
    struct conn_server_s *cs = c->parent;

    next = rb_recv_read(&c->rb, &sz);
    rb_recv_pop(&c->rb);
    cs->recv(c, next, sz);
#elif defined HM_LOBBYSERVER
    int sz;
    char *next;

    next = rb_recv_read(&c->rb, &sz);
    c->net_nbuf = sz;
    memcpy(c->net_buf, next, sz);
    rb_recv_pop(&c->rb);
    recv_append_lobby(c, c->net_buf, c->net_nbuf);
#endif
}

static void connector_client(struct ev_loop *loop, ev_io *w, int revents)
{
    (void) revents;
    struct sockaddr_storage addr;
    socklen_t sl = sizeof(addr);
    int client;
    struct conn_client_s *cc;
    struct conn_server_s *cs = w->data;

    assert(cs);

    client = accept(w->fd, (struct sockaddr *) &addr, &sl);
    if(client == -1) {
        switch (errno) {
            case EMFILE:
                hm_log(LOG_ERR, cs->log, "{Connector}: accept() failed; too many open files for this process");
                break;

            case ENFILE:
                hm_log(LOG_ERR, cs->log, "{client} accept() failed; too many open files for this system");
                break;

            default:
                assert(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN);
                break;
        }
        return;
    }

    int flag = 1;
    int ret = setsockopt(client, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
    if(ret == -1) {
        hm_log(LOG_ERR, cs->log, "{Connector}: Couldn't setsockopt on client (TCP_NODELAY)");
    }
#ifdef TCP_CWND
    int cwnd = 10;
    ret = setsockopt(client, IPPROTO_TCP, TCP_CWND, &cwnd, sizeof(cwnd));
    if(ret == -1) {
        hm_log(LOG_ERR, cs->log, "Couldn't setsockopt on client (TCP_CWND)");
    }
#endif

    setnonblock(client);
    setkeepalive(client);

#define PEER_NAME
#ifdef PEER_NAME
    socklen_t len;
    struct sockaddr_storage paddr;
    char ipstr[INET6_ADDRSTRLEN];
    int pport, peer;

    len = sizeof(paddr);
    peer = getpeername(client, (struct sockaddr*)&paddr, &len);

    // deal with both IPv4 and IPv6:
    if(paddr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&paddr;
        pport = ntohs(s->sin_port);
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
    } else { // AF_INET6
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&paddr;
        pport = ntohs(s->sin6_port);
        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
    }

    if(peer == -1) {
        hm_log(LOG_WARNING, cs->log, "{Connector}: Couldn't retrieve peer name");
    }
#endif

    cc = hm_palloc(cs->pool, sizeof(struct conn_client_s));
    if(cc == NULL) {
        hm_log(LOG_WARNING, cs->log, "{Connector}: Memory allocation failed");
        return;
    }

    memset(cc, 0, sizeof(struct conn_client_s));

    cc->loop = loop;
    cc->fd = client;
    cc->pool = cs->pool;
    cc->log = cs->log;
    cc->read.data = cc;
    cc->write.data = cc;
    cc->parent = cs;
    cc->error_callback = client_error;
    cc->client_dc = cs->client_dc;
#ifdef PEER_NAME
    cc->nip = strlen(ipstr);
    memcpy(cc->ip, ipstr, cc->nip);
    cc->port = pport;
#endif

    if(connector_addclient(cs, cc) == NULL) {
        hm_pfree(cs->pool, cc);
        return;
    }

    cc->recv = recv_append;
    async_client_accept(cc);
}

int connector_server(struct conn_server_s *cs)
{
    int t = 1;
    int timeout = 1;
    struct addrinfo *ai, hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    const int gai_err = getaddrinfo(cs->host, cs->port, &hints, &ai);

    if(gai_err != 0) {
        hm_log(LOG_CRIT, cs->log, "{Connector}: [%s]", gai_strerror(gai_err));
        return -1;
    }

    cs->fd = socket(ai->ai_family, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if(cs->fd == -1) {
        hm_log(LOG_CRIT, cs->log, "{Connector}: socket() failed");
        return -2;
    }

    hm_log(LOG_INFO, cs->log, "{Connector}: opening server on %s:%s fd: %d", cs->host, cs->port, cs->fd);
#ifdef SO_REUSEADDR
    setsockopt(cs->fd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(int));
#endif
#ifdef SO_REUSEPORT
    setsockopt(cs->fd, SOL_SOCKET, SO_REUSEPORT, &t, sizeof(int));
#endif

    if(bind(cs->fd, ai->ai_addr, ai->ai_addrlen)) {
        hm_log(LOG_CRIT, cs->log, "{Connector}: bind() failed");
        return -3;
    }

#if TCP_DEFER_ACCEPT
    setsockopt(cs->fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &timeout, sizeof(int));
#endif

    freeaddrinfo(ai);
    listen(cs->fd, 128);

    ev_io_init(&cs->listener, connector_client, cs->fd, EV_READ);
    cs->listener.data = cs;
    ev_io_start(cs->loop, &cs->listener);

    return 0;
}


static void *connector_addclient(struct conn_server_s *cs, struct conn_client_s *cc)
{
    struct conn_client_holder_s *cch;

    cch = hm_palloc(cs->pool, sizeof(*cch));

    if(cch == NULL) {
        hm_log(LOG_WARNING, cs->log, "{Connector}: Memory allocation failed");
        return NULL;
    }

    memset(cch, 0, sizeof(*cch));

    cch->client = cc;
    cch->next = cs->clients_head;

    cc->shutdown_signal_holder = &cch->signal_shutdown;

    cs->clients_head = cch;
    cs->clients++;

    cc->holder = cch;

    hm_log(LOG_NOTICE, cs->log, "{Connector}: adding client %.*s:%d, total: %d", cc->nip, cc->ip, cc->port, cs->clients);

    return cch;
}

int async_shutdown_client(struct conn_client_s *c)
{
    if(c == NULL) {
        return -1;
    }

    ev_io_stop(c->loop, &c->read);
    ev_io_stop(c->loop, &c->write);
    ev_timer_stop(c->loop, &c->hang_timer);

    /** pop in case it wasn't sent */
    rb_send_pop(&c->rb, c->pool);

    /** let holder know we're about to shutdown */
    if(c->shutdown_signal_holder) {
        *c->shutdown_signal_holder = 1;
    }

    if(c->monitor == 1 && c->parent != NULL) {
        c->parent->clients--;
    }

    hm_log(LOG_DEBUG, c->log, "{Connector}: async removing client %.*s:%d fd: %d type: %d alive since: %s", c->nip, c->ip, c->port, c->fd, c->type, c->date);

    connector_fd_close(c->fd);

    if(c->client_dc) {
        c->client_dc(c->data, c->foreign_client_index, c->hbs_id);
    }

#ifdef HM_GAMESERVER
    HT_REM(async_clients, c->client_index, strlen(c->client_index), c->pool);
#endif

    hm_pfree(c->pool, c);

    return 0;
}

void async_handle_socket_errno(struct conn_client_s *c)
{
    if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
        return;
    }

    if(c == NULL) {
        return;
    }

    if(errno == ECONNRESET) {
        hm_log(LOG_ERR, c->log, "{Connector} Connection reset by peer");
    } else if(errno == ETIMEDOUT) {
        hm_log(LOG_ERR, c->log, "{Connector} Connection to backend timed out");
    } else if(errno == EPIPE) {
        hm_log(LOG_ERR, c->log, "{Connector} Broken pipe to backend (EPIPE)");
    } else {
        hm_log(LOG_ERR, c->log, "{Connector} errno: %d", errno);
    }
}

static void async_hang_timeout(struct ev_loop *loop, struct ev_timer *timer, int revents)
{
    struct conn_client_s *c = (struct conn_client_s *)timer->data;

    assert(c);

    hm_log(LOG_DEBUG, c->log, "{Connector}: read timeout");

    if(c->error_callback) {
        c->error_callback(c, CL_HANGTIMEOUT_ERR);
    }
}

#define NETDUMP(type, _buf, qq)\
    int i;\
if(1 == 1) {\
    if(type == 0) {\
        hm_log(LOG_DEBUG, c->log, "sent: [%d] bytes %d", qq, fd);\
    } else {\
        hm_log(LOG_DEBUG, c->log, "received: [%d] bytes to fd %d", qq, fd);\
    }\
    printf("Ascii:\n--------\n");\
    for(i = 0; i < qq; i++) {\
        printf("%c", _buf[i] );\
    }\
    printf("Binary: ");\
    for( i = 0; i < qq; i++ ) {\
        printf("%.2x", (unsigned char)_buf[i] );\
    }\
    printf("\n");\
}

static void async_read(struct ev_loop *loop, ev_io *w, int revents)
{
    (void) revents;
    int t = 0;
    struct conn_client_s *c = (struct conn_client_s *)w->data;
    int fd = w->fd, used;

    char *buf = rb_recv_ptr(&c->rb, &used);

    if(c->want_shutdown) {
        if(c->error_callback) {
            c->error_callback(c, CL_WANTSHUTDOWN_ERR);
        }

        return;
    }

    hm_log(LOG_DEBUG, c->log, "Received from fd %d", fd);

    t = recv(fd, buf, RB_SLOT_SIZE - used, 0);

    if(t > 0) {
        //NETDUMP(1, buf, t)
        rb_recv_append(&c->rb, t);

        if(rb_recv_is_full(&c->rb)) {
            ev_io_stop(c->loop, &c->read);
            if(c->error_callback) {
                c->error_callback(c, CL_READRBFULL_ERR);
            }
            return;
        }

        if(c->hang_timer_enabled == 1) {
            ev_timer_stop(c->loop, &c->hang_timer);
            ev_timer_again(c->loop, &c->hang_timer);
        }

        c->recv(c);

    } else if(t == 0) {
        async_handle_socket_errno(c);
        if(c->error_callback) {
            c->error_callback(c, CL_READZERO_ERR);
        }
    } else {
        async_handle_socket_errno(c);
        if(c->error_callback) {
            c->error_callback(c, CL_READ_ERR);
        }
    }
}

static void async_write(struct ev_loop *loop, ev_io *w, int revents)
{
    (void)revents;
    int t;
    struct conn_client_s *c = (struct conn_client_s *)w->data;
    int fd = w->fd;
    int sz;

    assert(c);

    char *next = rb_send_next(&c->rb, &sz);

    if(sz == 0) {
        ev_io_stop(loop, &c->write);
        return;
    }

    t = send(fd, next, sz, MSG_NOSIGNAL);
    if(t > 0) {
        //NETDUMP(0, next, t)
        rb_send_skip(&c->rb, t);
        if(rb_send_is_empty(&c->rb)) {
            ev_io_stop(loop, &c->write);
        }
    } else {
        //assert(t == -1);
        async_handle_socket_errno(c);
        if(c->error_callback) {
            c->error_callback(c, CL_WRITE_ERR);
        }
    }
}

static void timestring(char *b, const int nb)
{
    char    buf[128];
    time_t          s;
    struct timespec spec;
    long long       ms;
    struct tm       ts;

    clock_gettime(CLOCK_REALTIME, &spec);
    s = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6);

    ts = *localtime(&s);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts);

    snprintf(b, nb, "[%s.%03lld] ", buf, ms);
}

int async_client(struct conn_client_s *client, const int monitor, const int hang)
{
    struct sockaddr_in servaddr;
    char ip[32];

    assert(client);

    /** TCP & non-blocking */
    client->fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if(-1 == client->fd) {
        hm_log(LOG_ERR, client->log, "{Connector}: node init socket error: %d", client->fd);
        client->error_callback(client, CL_SOCKET_ERR);
        return -1;
    }

    snprintf(ip, sizeof(ip), "%.*s", client->nip, client->ip);

    setkeepalive(client->fd);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(client->port);

    ev_io_init(&client->write, async_write, client->fd, EV_WRITE);
    ev_io_init(&client->read, async_read, client->fd, EV_READ);
    ev_init(&client->hang_timer, async_hang_timeout);

    if(hang == 1) {
        client->hang_timer.repeat = ASYNC_HANG_TIMEOUT;
        client->hang_timer.data = client;
        client->hang_timer_enabled = 1;
    } else {
        client->hang_timer_enabled = 0;
    }
    client->read.data = client;
    client->write.data = client;
    timestring(client->date, sizeof(client->date));

    if(monitor == 1) {
        connector_addclient(client->parent, client);
        client->monitor = 1;
    }

    ev_io_start(client->loop, &client->read);
    ev_timer_again(client->loop, &client->hang_timer);
    if(connect(client->fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != -1 && errno != EINPROGRESS) {
        async_shutdown_client(client);
        hm_log(LOG_ERR, client->log, "{Connector}: connect() errno: %d", errno);
        return -1;
    }

    return 0;
}

static int async_client_accept(struct conn_client_s *client)
{
    ev_io_init(&client->write, async_write, client->fd, EV_WRITE);
    ev_io_init(&client->read, async_read, client->fd, EV_READ);
    //ev_init(&client->hang_timer, async_hang_timeout);

    client->hang_timer.repeat = ASYNC_HANG_TIMEOUT;
    //client->hang_timer.data = client;
    client->read.data = client;
    client->write.data = client;

    ev_io_start(client->loop, &client->read);

    timestring(client->date, sizeof(client->date));
    //ev_timer_again(client->loop, &client->hang_timer);

    snprintf(client->client_index, sizeof(client->client_index), "%lld", client_index++);

#ifdef HM_GAMESERVER
    HT_ADD_WA(async_clients, client->client_index, strlen(client->client_index), client, sizeof(client), client->pool);
#endif

    return 0;
}
