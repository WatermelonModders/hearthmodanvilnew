/*
   hm_gameserver -  hearthmod gameserver
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
#include <unistd.h>
#include <game.h>

#include <errno.h>

struct ht_s **games = NULL;

struct conn_server_s *user_cs;
struct hm_log_s *lg;

struct ev_loop *loop = NULL;
static struct hm_log_s l;
static struct hm_pool_s *pool;

void set_result(const char *k, int r);

static void client_disconnected(void *data, const char *foreign_client_index, const char *hbs_id)
{
    struct hs_holder_s *p = data;
    struct game_s *g;

    if(p == NULL) {
        return;
    }

    struct packet_s *packet = net_game_over(p, p->entity_id == 2 ? 3 : 2);

    struct ht_s *opponent = ht_get(async_clients, foreign_client_index, strlen(foreign_client_index));

    if(opponent && opponent->n == sizeof(void *)) {
        // loser
        hm_log(LOG_DEBUG, lg, "Client disconnect callback, opponent exists");
        net_send((void *)opponent->s, packet);
        set_result(hbs_id, 0);
    } else {
        // winner
        hm_log(LOG_DEBUG, lg, "Client disconnect callback, opponent doesn't exist, cleaning game");
        set_result(hbs_id, 1);
        g = p->game;
        game_free(g);
    }

    packet_free(packet);
    holder_free(p);
}

static void hm_recv(struct conn_client_s *cs, const char *buf, const int len)
{
    hs(cs, buf, len);
}

static void server_shutdown()
{
    shutdown_server(user_cs);

    couchbase_deinit();

    ht_free(games, pool);
    ht_free(async_clients, pool);
}

int start_server()
{
    struct conn_server_s *cs;

    entities_init();
    entities_init2();

    lg = &l;
    games = ht_init(pool);
    async_clients = ht_init(pool);

    cs = malloc(sizeof(*cs));
    memset(cs, 0, sizeof(*cs));

    cs->loop = loop;
    cs->host = "0.0.0.0";
    cs->port = "3724";
    cs->log = &l;
    cs->pool = pool;
    cs->recv = hm_recv;
    cs->client_dc = client_disconnected;
    cs->shutdown = server_shutdown;
    connector_server(cs);

    user_cs = cs;

    return 0;
}

void callback(struct instance_s *data)
{
    struct instance_s *i;

    if(data == NULL) {
        couchbase_deinit();
        hm_log(LOG_DEBUG, lg, "{Connector}: all destinations are offline");
        return;
    }

    for(i = data; i != NULL; i = i->next) {
        hm_log(LOG_DEBUG, lg, "{Connector}: connstr: [%s] online: %d group: %d bucket index: %d", i->connstr, i->status, i->group, i->index);
    }

    start_server();
}

static void sigh_terminate(int __attribute__ ((unused)) signo)
{
    user_cs->shutdown();
}

static void initsignals()
{
    struct sigaction act;

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = SIG_IGN;

    if(sigaction(SIGPIPE, &act, NULL) < 0) {
        hm_log(LOG_CRIT, lg, "{Connector}: sigaction sigpipe");
    }

    /** We don't care if someone stops and starts a child process with kill (1) */
    act.sa_flags = SA_NOCLDSTOP;

    /* catch INT and TERM signals */
    act.sa_flags = 0;
    act.sa_handler = sigh_terminate;
    if(sigaction(SIGINT, &act, NULL) < 0) {
        hm_log(LOG_CRIT, lg, "{Connector}: Unable to register SIGINT signal handler: %s", strerror(errno));
        exit(1);
    }

    if(sigaction(SIGTERM, &act, NULL) < 0) {
        hm_log(LOG_CRIT, lg, "{Connector}: Unable to register SIGTERM signal handler: %s", strerror(errno));
        exit(1);
    }
}

void daemonize () {
    if(chdir("/") != 0) {
        hm_log(LOG_DEBUG, lg, "Unable change directory to /: %s", strerror(errno));
        exit(1);
    }

    pid_t pid = fork();
    if(pid < 0) {
        hm_log(LOG_DEBUG, lg, "Unable to daemonize: fork failed: %s", strerror(errno));
        exit(1);
    }

    if(pid != 0) {
        hm_log(LOG_DEBUG, lg, "Daemonized as pid %d.", pid);
        exit(0);
    }

    fclose(stdin);
    fclose(stdout);
    fclose(stderr);

#define NULL_DEV	"/dev/null"

    stdin = fopen(NULL_DEV, "r");
    if(stdin == NULL) {
        hm_log(LOG_EMERG, lg, "Unable to reopen stdin to %s: %s", NULL_DEV, strerror(errno));
        exit(1);
    }

    stdout = fopen(NULL_DEV, "w");
    if(stdout == NULL) {
        hm_log(LOG_EMERG, lg, "Unable to reopen stdout to %s: %s", NULL_DEV, strerror(errno));
        exit(1);
    }

    stderr = fopen(NULL_DEV, "w");
    if(stderr == NULL) {
        hm_log(LOG_EMERG, lg, "Unable to reopen stderr to %s: %s", NULL_DEV, strerror(errno));
        exit(1);
    }

    pid_t s = setsid();
    if(s < 0) {
        hm_log(LOG_EMERG, lg, "Unable to create new session, setsid(2) failed: %s :: %d", strerror(errno), s);
        exit(1);
    }

    hm_log(LOG_DEBUG, lg, "Successfully daemonized as pid %d.", getpid());
}

int main(int argc, char **argv)
{
    struct couchbase_data_s cd;
    const char *hosts[1] = { "localhost" };
    const int groups[1] = { 0 };  // Must start with 0
    const char *buckets[1] = { "hbs" };
    const char *passwd[1] = { "aci" };
    int i, daemon = 0;

    memset(&l, 0, sizeof(l));

    for(i = 1; i < argc; i++) {
        if(strlen(argv[i]) > 6 && memcmp(argv[i], "--log=", 6) == 0) {
            daemon = 1;
            hm_log_open(&l, argv[i] + 6, LOG_DEBUG);
        }
    }

    if(daemon == 0) {
        hm_log_open(&l, NULL, LOG_DEBUG);
    }

    lg = &l;

    loop = ev_default_loop(0);
    pool = hm_create_pool(&l);

    hm_log(LOG_DEBUG, lg, "\nhm_gameserver  Copyright (C) 2016  Filip Pancik\n\
            This program comes with ABSOLUTELY NO WARRANTY.\n\
            This is free software, and you are welcome to redistribute it\n\
            under certain conditions.");

    initsignals();

    if(daemon == 1) {
        daemonize();
    }

    cd.loop = loop;
    cd.log = &l;
    cd.pool = pool;
    cd.callback = callback;

    cd.hosts = hosts;
    cd.hgroups = groups;
    cd.nhosts = 1;
    cd.buckets = buckets;
    cd.bpasswd = passwd;
    cd.nbuckets = 1;

    couchbase_init(&cd);

    ev_run(loop, 0);

    hm_destroy_pool(pool);
    hm_log_close(&l);

    ev_default_destroy();

    return 0;
}
