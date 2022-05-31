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

#include <assert.h>

#include <hmbase.h>
#include <connector.h>

#define CL(a, b) case a: hm_log(LOG_ERR, log, "{Connector}: fd:%d, "b, fd); break;

void connector_fd_close(int fd)
{
    if(fd > STDERR_FILENO) {
        close(fd);
    }
}

static int getsockerr(int fd, int name, void *optval, const int l, struct hm_log_s *log)
{
    int r;
    socklen_t len = l;

    r = getsockopt(fd, SOL_SOCKET, name, optval, &len);
    if(r < 0) {
        switch(errno) {
            CL(EBADF, "The argument sockfd is not a valid descriptor.");
            CL(EFAULT, "The address pointed to by optval is not in a valid part of the process address space.");
            CL(EINVAL, "optlen invalid in setsockopt()");
            CL(ENOPROTOOPT, "The option is unknown at the level indicated.");
            CL(ENOTSOCK, "The argument sockfd is a file, not a socket.");
        }
        return -1;
    }
    return 0;
}

static int getsoerror(int fd, struct hm_log_s *log)
{
    int error = 0;

    if(getsockerr(fd, SO_ERROR, &error, sizeof(int), log) < 0) {
        return -1;
    }

    switch(error) {
        case 0: return 0; break;
                CL(EACCES, "For UNIX domain sockets, which are identified by pathname: Write permission is denied on the socket file, or search permission is denied for one of the directories in the path prefix.  (See also path_resolution(7).)");         CL(EPERM, "The user tried to connect to a broadcast address without having the socket broadcast flag enabled or the connection request failed because of a  local  firewall rule.");
                CL(EADDRINUSE, "Local address is already in use.");
                CL(EAFNOSUPPORT, "The passed address didn't have the correct address family in its sa_family field.");
                CL(EAGAIN, "No  more  free  local  ports  or insufficient entries in the routing cache.  For AF_INET see the description of /proc/sys/net/ipv4/ip_local_port_range ip(7) for information on how to increase the number of local ports.");
                CL(EALREADY, "The socket is nonblocking and a previous connection attempt has not yet been completed.");
                CL(EBADF, "The file descriptor is not a valid index in the descriptor table.");
                CL(ECONNREFUSED, "No-one listening on the remote address.");
                CL(EFAULT, "The socket structure address is outside the user's address space.");
                CL(EINPROGRESS, "The socket is nonblocking and the connection cannot be completed immediately.  It is possible to select(2) or poll(2) for completion by selecting the socket forwriting.  After select(2) indicates writability, use getsockopt(2) to read the SO_ERROR option at level SOL_SOCKET to determine whether connect() completed successfully (SO_ERROR is zero) or unsuccessfully (SO_ERROR is one of the usual error codes listed here, explaining the reason for the failure).");
                CL(EINTR, "The system call was interrupted by a signal that was caught; see signal(7).");
                CL(EISCONN, "The socket is already connected.");
                CL(ENETUNREACH, "Network is unreachable.");
                CL(ENOTSOCK, "The file descriptor is not associated with a socket.");
                CL(ETIMEDOUT, "Timeout while attempting connection.  The server may be too busy to accept new connections.  Note that for IP sockets the timeout may be very long when syncookies are enabled on the server.");
    }
    return -2;
}

static int islinger(struct conn_node_s *c)
{
    struct linger l;

    if(getsockerr(c->fd, SO_LINGER, &l, sizeof(l), c->log) < 0) {
        return -1;
    }

    return l.l_onoff;
}

static void failed_cb(struct ev_loop *loop, ev_timer *w, int revents)
{
    int r;
    struct conn_node_s *c = (struct conn_node_s *)w->data;
#ifdef D_ASSERT
    assert(c != NULL);
#endif

    ev_io_stop(loop, &c->io);

    r = islinger(c);
    if(r < 0) {
        hm_log(LOG_ERR, c->log, "{Connector}: error on socket %d", c->fd);
    } else if(r > 0) {
        hm_log(LOG_ERR, c->log, "{Connector}: SO_LINGER set on fd %d", c->fd);
    }
    connector_fd_close(c->fd);
}

static void established_cb(struct ev_loop *loop, ev_io *w, int revents)
{
    int r;
    struct conn_node_s *c = (struct conn_node_s *)w->data;

#ifdef D_ASSERT
    assert(c != NULL);
#endif

    ev_timer_stop(loop, &c->timer);
    ev_io_stop(loop, w);

    r = getsoerror(c->fd, c->log);

    if(r == 0) {
        c->status |= CONN_F_CONNECTED;

        if(!(c->status & CONN_F_FDOPEN)) {
            r = islinger(c);
            if(r < 0) {
                hm_log(LOG_ERR, c->log, "{Connector}: error on socket %d", c->fd);
            } else if(r > 0) {
                hm_log(LOG_ERR, c->log, "{Connector}: SO_LINGER set on fd %d", c->fd);
            }
            connector_fd_close(c->fd);
        }

        /**< return immediately when single check is performed or all nodes are connected */
        if(c->status & CONN_F_SINGLECHECK || ++(c->parent->established) == c->parent->size) {
            ev_timer_stop(loop, &c->parent->timer);
            c->parent->callback(c->parent);
        }
    }
}

static int check_connection(struct conn_data_s *cd, struct conn_node_s *c)
{
    struct sockaddr_in servaddr;

#ifdef D_ASSERT
    assert(c != NULL && cd->loop != NULL);
#endif
    if(strlen(c->host) < 1 || c->port < 1 || c->port > ((1<<16) - 1)) {
        hm_log(LOG_ERR, c->log, "{Connector}: incorrect hostname or port: [%s]:%d", c->host, c->port);
        return -1;
    }

    c->fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if(-1 == c->fd) {
        hm_log(LOG_ERR, c->log, "{Connector}: FD = -1");
        return -1;
    }

    // TODO: setsockopt depending on c->status

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=inet_addr(c->host);
    servaddr.sin_port=htons(c->port);

    c->io.data = c;
    ev_io_init(&c->io, established_cb, c->fd, EV_WRITE);
    ev_io_start(cd->loop, &c->io);

    c->timer.data = c;
    ev_timer_init(&c->timer, failed_cb, cd->timeout, 0);
    ev_timer_start(cd->loop, &c->timer);

    hm_log(LOG_INFO, c->log, "{Connector}: connecting to %s:%d", c->host, c->port);
    connect(c->fd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    return 0;
}

static void terminator(struct ev_loop *loop, ev_timer *w, int revents)
{
    int i;
    struct conn_data_s *c = (struct conn_data_s *)w->data;

#ifdef D_ASSERT
    assert(c != NULL);
#endif
    /**< Terminate all connector events */
    for(i = 0; i < c->size; i++) {
        if(c && c->loop && c->src[i]) {
            ev_io_stop(c->loop, &c->src[i]->io);
            ev_timer_stop(c->loop, &c->src[i]->timer);
        }
    }

    c->callback(c);
}

int setkeepalive(int fd)
{
    int optval;
    socklen_t optlen = sizeof(optval);

    optval = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
        return -1;
    }

    return 0;
}

int connector(struct conn_data_s *c)
{
    int i;

#ifdef D_ASSERT
    assert(c != NULL);
#endif

    c->established = 0;
    for(i = 0; i < c->size; i++) {
        c->src[i]->parent = c;
        c->src[i]->log = c->log;
        check_connection(c, c->src[i]);
    }

    c->timer.data = c;

    ev_timer_init(&c->timer, terminator, c->exptime, 0);
    ev_timer_start(c->loop, &c->timer);

    return 0;
}

void connector_free(struct conn_data_s *c)
{
    if(!c) {
        return;
    }

    int i;
    for(i = 0; i < c->size; i++) {
        ev_io_stop(c->loop, &c->src[i]->io);
        ev_timer_stop(c->loop, &c->src[i]->timer);
        connector_fd_close(c->src[i]->fd);
        hm_pfree(c->pool, c->src[i]);
    }

    ev_timer_stop(c->loop, &c->timer);
    hm_pfree(c->pool, c->src);
    hm_pfree(c->pool, c);
}
