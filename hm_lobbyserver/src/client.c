/*
   hm_lobbyserver - HearthStone HearthMod lobbyserver
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
#include <malloc.h>
#include <json.h>
#include <string.h>

#include <libcouchbase/couchbase.h>

#define HM_LOBBYSERVER

#include <hmbase.h>
#include <mods.h>

#include <client.h>

char *read_str(char **buf, char *end, int *len);

void login_reply(struct conn_client_s *cs, int error)
{
    struct json_object *obj;

    obj = json_object_new_object();
    json_object_object_add(obj, "type", json_object_new_int(CLIENT_LOGIN));
    json_object_object_add(obj, "error", json_object_new_int(error));
    json_object_object_add(obj, "secret", json_object_new_string(cs->login.secret));
    json_object_object_add(obj, "mod_name", json_object_new_string(MOD_NAME));
    json_object_object_add(obj, "mod_url", json_object_new_string(MOD_URL));
    const char *str = json_object_get_string(obj);

    hm_send(cs, str, strlen(str));
}

void client_login_3(struct cbop_s *cbop)
{
    struct conn_client_s *cs = cbop->data;

    if(cbop->lcb_error == LCB_SUCCESS) {
        hm_log(LOG_DEBUG, cs->log, "{Client}: Deck [%.*s] retrieval succeeded", (int)CBGR(nkey), (char *)CBGR(key));
        login_reply(cs, 0);
    } else {
        hm_log(LOG_DEBUG, cs->log, "{Client}: Deck [%.*s] retrieval failed with error %d", (int)CBGR(nkey), (char *)CBGR(key), cbop->lcb_error);
        login_reply(cs, 3);
    }

    free(cbop);
}

void client_login_2(struct cbop_s *cbop)
{
    struct conn_client_s *cs = cbop->data;

    if(cbop->lcb_error == LCB_SUCCESS) {
        struct json_object *obj;

        struct json_tokener *tok = json_tokener_new();
        obj = json_tokener_parse_ex(tok, CBGR(bytes), CBGR(nbytes));

        if(obj == NULL) {
            hm_log(LOG_DEBUG, cs->log, "Parsing [%.*s] failed", (int)CBGR(nkey), (char *)CBGR(key));
            free(cbop);
            return;
        }

        hm_log(LOG_DEBUG, cs->log, "{Client}: requesting key [%s] succeeded", cs->login.user);

        json_object_object_foreach(obj, key0, val0) {
            int type = json_object_get_type(val0);
            if(type == json_type_string) {
                const char *v = json_object_get_string(val0);
                if(strcmp(key0, "secret") == 0) {
                    snprintf(cs->login.secret, sizeof(cs->login.secret), "%s", v);
                }
            }
        }

        json_object_object_foreach(obj, key, val) {
            int type = json_object_get_type(val);
            if(type == json_type_string) {
                const char *v = json_object_get_string(val);
                if(strcmp(key, "password") == 0) {
                    if(strcmp(v, cs->login.pass) == 0) {
                        memset(cbop, 0, sizeof(*cbop));
                        cbop->data = cs;

                        char k1[128];
                        snprintf(k1, sizeof(k1), "u:deck_%s_%s", MOD_URL, cs->login.secret);
                        CBGQ_V0(0, k1, strlen(k1), client_login_3, 0, 0);
                        return;
                    } else {
                        hm_log(LOG_DEBUG, cs->log, "{Client}: passwords do not match %s %s", cs->login.pass, v);
                        free(cbop);
                        login_reply(cs, 2);
                        return;
                    }
                }
            }
        }

        hm_log(LOG_DEBUG, cs->log, "{Client}: password doesn't match");
        free(cbop);
        login_reply(cs, 2);

    } else {
        hm_log(LOG_DEBUG, cs->log, "{Client}: requesting key [%s] failed with error %d", cs->login.user, cbop->lcb_error);
        free(cbop);
        login_reply(cs, 1);
    }
}

void client_login(struct conn_client_s *cs, const char *buf, int len)
{
    const char *username, *password;
    struct cbop_s *cbop;

    json_tokener *tok;
    struct json_object *obj;

    tok = json_tokener_new();
    obj = json_tokener_parse_ex(tok, buf, len);

    if(obj == NULL) {
        return;
    }

    json_object_object_foreach(obj, key, val) {
        if(strcmp(key, "user") == 0) {
            int type = json_object_get_type(val);
            if(type == json_type_string) {
                username = json_object_get_string(val);
            }
        } else 	if(strcmp(key, "pass") == 0) {
            int type = json_object_get_type(val);
            if(type == json_type_string) {
                password = json_object_get_string(val);
            }
        }
    }

    if(!(strlen(username) > 0 && strlen(username) < 32 && strlen(password) > 0 && strlen(password) < 32)) {
        return;
    }

    char k[128];
    snprintf(k, sizeof(k), "u:%s", username);
    snprintf(cs->login.user, sizeof(cs->login.user), "%s", username);
    snprintf(cs->login.pass, sizeof(cs->login.pass), "%s", password);

    cbop = malloc(sizeof(*cbop));
    memset(cbop, 0, sizeof(*cbop));
    cbop->data = cs;
    CBGQ_V0(0, k, strlen(k), client_login_2, 0, 0);

    json_object_put(obj);
    json_tokener_free(tok);
}
