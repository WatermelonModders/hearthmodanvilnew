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
#include <mods.h>
#include <game.h>

enum board_e {
    BOARD_RANDOM	= 0x0c,
};

extern struct ht_s **games;

int ping(struct conn_client_s *c)
{
    struct packet_s *p;
    char output[512], *ptr;
    int len;

    ptr = output;
    p = packet_init(P_PONG, NULL);

    len = serialize(p, &ptr, ptr + sizeof(output));

    packet_free(p);

    hm_send(c, output, len);

    return 0;
}	

int game_setup(struct conn_client_s *p1, struct conn_client_s *p2, const int id)
{
    struct gamesetup_s *g;
    struct packet_s *p;
    char output[512], *ptr;
    int len;

    ptr = output;
    g = malloc(sizeof(*g));

    int rand = random_number(12);
    // board indexes 0 and 9 not supported
    g->board = (rand == 0 || rand == 9) ? 1 : rand;
    g->maxsecrets = 5;
    g->maxfriendlyminions = 7;
    g->keepalive = 900;
    g->stuckdisconnect = 25;

    p = packet_init(P_GAMESETUP, g);

    len = serialize(p, &ptr, ptr + sizeof(output));

    packet_free(p);

    hm_log(LOG_DEBUG, p1->log, "starting game %d", id);

    assert(p1->data == NULL && p2->data == NULL);

    append_decks(p1, p2);

    hm_send(p1, output, len);
    hm_send(p2, output, len);

    return 0;
}

static void retrieve_deck_cb(struct cbop_s *cbop)
{
    struct watcher_s *w = cbop->data;
    int cards_count = 0;
    const char *hp = NULL;

    if(cbop->lcb_error == LCB_SUCCESS) {
        json_tokener *tok;
        struct json_object *obj;
        int i;
        struct player_deck_s *pd = NULL;

        tok = json_tokener_new();
        obj = json_tokener_parse_ex(tok, CBGR(bytes), CBGR(nbytes));

        json_object_object_foreach(obj, key, val) {
            if(strcmp(key, "hero") == 0) {
                int type = json_object_get_type(val);
                if(type == json_type_string) {
                }
            } else if(strcmp(key, "hp") == 0) {
                int type = json_object_get_type(val);
                if(type == json_type_string) {
                    hp = json_object_get_string(val);
                }
            } else if(strcmp(key, "cards") == 0) {
                int type = json_object_get_type(val);
                if(type == json_type_array) {
                    int l = json_object_array_length(val);

                    for(i = 0; i < l; i++) {
                        struct player_deck_s *pdc = malloc(sizeof(*pdc));
                        pdc->next = pd;
                        pd = pdc;

                        json_object *o = json_object_array_get_idx(val, i);
                        int subtype = json_object_get_type(o);
                        if(subtype == json_type_object) {
                            json_object *code;
                            json_object_object_get_ex(o, "code", &code);
                            if(json_object_get_type(code) == json_type_string) {
                                snprintf(pdc->code, sizeof(pdc->code), "%s", json_object_get_string(code));
                            }

                            json_object *count;
                            json_object_object_get_ex(o, "count", &count);
                            if(json_object_get_type(count) == json_type_int) {
                                pdc->count =  json_object_get_int(count);
                                cards_count += pdc->count;
                            }
                        }
                    }
                }
            }
        }


        char key1[128], key2[128];
        snprintf(key1, sizeof(key1), "u:deck_%s_%s", MOD_URL, w->p1->hbs_id);
        snprintf(key2, sizeof(key2), "u:deck_%s_%s", MOD_URL, w->p2->hbs_id);

        assert(hp);

        if(CBGR(nkey) == strlen(key1) && memcmp(key1, CBGR(key), strlen(key1)) == 0 && !(w->p1->cards)) {
            hm_log(LOG_DEBUG, lg, "Player1 deck: %d", cards_count);
            w->p1->cards = pd;
            w->p1->ncards = cards_count;
            snprintf(w->p1->hp, sizeof(w->p1->hp), "%s", hp);
            //snprintf(w->p1->hero, sizeof(w->p1->hero), "%s", hero);
        } else if(CBGR(nkey) == strlen(key2) && memcmp(key2, CBGR(key), strlen(key2)) == 0 && !(w->p2->cards)) {
            hm_log(LOG_DEBUG, lg, "Player2 deck: %d", cards_count);
            w->p2->cards = pd;
            w->p2->ncards = cards_count;
            snprintf(w->p2->hp, sizeof(w->p2->hp), "%s", hp);
            //snprintf(w->p2->hero, sizeof(w->p2->hero), "%s", hero);
        } else {
            abort();
        }

        json_object_put(obj);
        json_tokener_free(tok);
    } else {
        hm_log(LOG_DEBUG, lg, "Couchbase key [%.*s] failed with error 0x%x", (int)CBGR(nkey), (char *)CBGR(key), cbop->lcb_error);
    }

    if(++(w->counter) == 2) {
        //assert(w->p1->cards && w->p2->cards);
        //assert(w->p1->ncards != 0);
        //assert(w->p2->ncards != 0);
        if(w->p1->cards == NULL || w->p2->cards == NULL ||
                w->p1->ncards != 30 || w->p2->ncards != 30) {
            hm_log(LOG_DEBUG, lg, "Droping both players as deck1: %p:%d  deck2: %p:%d", w->p1->cards, w->p1->ncards, w->p2->cards, w->p2->ncards);
            async_shutdown_client(w->p1);
            async_shutdown_client(w->p2);
            //FIXME: memory leak if any of the players has deck allocated
        } else {

            hm_log(LOG_DEBUG, lg, "Retrieved deck(30 cards) for both players");
            game_setup(w->p1, w->p2, 0);
        }
        free(w);
    }

    free(cbop);
}

void retrieve_deck(struct watcher_s *w, int player)
{
    struct cbop_s *cbop;
    char key[128];
    char *k;

    cbop = malloc(sizeof(*cbop));

    memset(cbop, 0, sizeof(*cbop));

    cbop->data = w;

    if(player == 0) {
        k = w->p1->hbs_id;
    } else {
        k = w->p2->hbs_id;
    }

    snprintf(key, sizeof(key), "u:deck_%s_%s", MOD_URL, k);

    hm_log(LOG_DEBUG, lg, "Retrieving deck for player: %d", player);

    CBGQ_V0(0, key, strlen(key), retrieve_deck_cb, 0, 0)
}


void get_decks(struct conn_client_s *p1, struct conn_client_s *p2, const int id)
{
    struct watcher_s *watcher;
    int i;

    watcher = malloc(sizeof(*watcher));

    watcher->p1 = p1;
    watcher->p2 = p2;
    watcher->counter = 0;
    watcher->target = 2;

    for(i = 0; i < 2; i++) {
        retrieve_deck(watcher, i);
    }
}

void find_game(struct conn_client_s *p1, struct handshake_s *h)
{
    char key[32];
    struct ht_s *opponent;
    struct conn_client_s *p2;
    int n, k;

    handshake_dump(h);

    // client id
    snprintf(p1->hbs_id, sizeof(p1->hbs_id), "%.*s", h->nversion, h->version);

    k = *(int *)h->password;
    n = snprintf(key, sizeof(key), "%d", k);

    opponent = ht_get(games, key, strlen(key));

    if(opponent == NULL) {
        HT_ADD_WA(games, key, n, p1, sizeof(p1), p1->pool);
    } else {
        p2 = (struct conn_client_s *)(opponent->s);
        HT_REM(games, key, n, p1->pool);

        // bind clients
        snprintf(p1->foreign_client_index, sizeof(p1->foreign_client_index), "%s", p2->client_index);
        snprintf(p2->foreign_client_index, sizeof(p2->foreign_client_index), "%s", p1->client_index);

        get_decks(p1, p2, k);
    }
}

void hs(struct conn_client_s *c, const char *buf, const int len)
{
    int l = 8;
    struct packet_s *p;
    char *ptr, *next = NULL;
    int player = 0;
    struct hs_holder_s *pd;
    struct ht_s *opponent;

    ptr = (char *)buf;

    if(len > 8) {
        l += *(int *)(buf + 4);
    }

    if(l < len) {
        next = (char *)buf + l;
    }

    p = deserialize(&ptr, ptr + l);
    if(p == NULL) {
        hm_log(LOG_DEBUG, lg, "Parsing %d bytes from fd %d failed, removing client", len, c->fd);
        async_shutdown_client(c);
        return;
    }

    if(c->data) {
        pd = c->data;
        player = pd->entity_id;
    }

    if(strlen(c->foreign_client_index) > 0) {
        opponent = ht_get(async_clients, c->foreign_client_index, strlen(c->foreign_client_index));
        if(opponent) {

            hm_log(LOG_DEBUG, lg, "Opponent found for fd %d", c->fd);
            if(opponent->n != sizeof(void *) || (void *)opponent->s != pd->opponent.client) {
                abort();
            }
        } else {
            hm_log(LOG_DEBUG, lg, "Opponent not found for fd %d, exitting", c->fd);
            packet_free(p);
            memset(&pd->opponent, 0, sizeof(pd->opponent));
            async_shutdown_client(c);
            return;
        }
    } else {
        hm_log(LOG_DEBUG, lg, "Foreign client not specified for client fd %d", c->fd);
        //async_shutdown_client(c);
        //return;
    }

    if(p->id == P_PING) {
        ping(c);
    } else if(p->id == P_HANDSHAKE) {
        find_game(c, p->data);
    } else if(p->id == P_GETGAMESTATE) {
        hm_log(LOG_DEBUG, lg, "Client command: gamestate player entity: %d", player);
        game_start(c);
    } else if(p->id == P_USERUI) {
        hm_log(LOG_DEBUG, lg, "Client command: userui player entity: %d", player);
        user_ui(c, p->data);
    } else if(p->id == P_CHOOSEENTITIES) {
        hm_log(LOG_ALERT, lg, "Client command: choose entities player entity: %d", player);
        if(pd && pd->choose_one == 1) {
            pd->choose_one = 0;
            choose_one_entity(c, p->data);
        } else {
            choose_entities(c, p->data);
        }
    } else if(p->id == P_CHOOSEOPTION) {
        chooseoption_dump(p->data, ((struct hs_holder_s *)(c->data))->held_card);
        hm_log(LOG_DEBUG, lg, "Client command: choose option player entity: %d", player);
        choose_option(c, p->data);
    } else {
        hm_log(LOG_DEBUG, lg, "Client command: unsupported command");
        async_shutdown_client(c);
        return;
    }

    if(p->id != P_USERUI) packet_free(p);

    if(next) {
        hs(c, next, len - l);
    }
}
