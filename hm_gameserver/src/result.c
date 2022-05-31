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
#include <game.h>

static void result2(struct cbop_s *cbop)
{
    if(cbop->lcb_error == LCB_SUCCESS) {
        hm_log(LOG_DEBUG, lg, "Key [%.*s] updated successfully", (int )CBSR(nkey), (char *)CBSR(key));
    } else {
        hm_log(LOG_DEBUG, lg, "Key[%.*s] update failed, lcb error: %d", (int )CBSR(nkey), (char *)CBSR(key), cbop->lcb_error);
    }

    free(cbop);
}

static void result1(struct cbop_s *cbop)
{
    if(cbop->lcb_error == LCB_SUCCESS) {
        struct json_object *obj;

        struct json_tokener *tok = json_tokener_new();
        obj = json_tokener_parse_ex(tok, CBGR(bytes), CBGR(nbytes));

        if(obj == NULL) {
            hm_log(LOG_DEBUG, lg, "Parsing [%.*s] failed", (int)CBGR(nkey), (char *)CBGR(key));
            free(cbop);
            return;
        }

        json_object *count;
        const char *type;

        if((long long)cbop->data == 1) {
            hm_log(LOG_DEBUG, lg, "Player [%.*s] set as winner", (int)CBGR(nkey), (char *)CBGR(key));
            type = "w";
        } else {
            hm_log(LOG_DEBUG, lg, "Player [%.*s] set as loser", (int)CBGR(nkey), (char *)CBGR(key));
            type = "l";
        }

        json_object_object_get_ex(obj, type, &count);
        if(json_object_get_type(count) == json_type_int) {
            int new_count = json_object_get_int(count) + 1;
            json_object_object_add(obj, type, json_object_new_int(new_count));

            const char *updated = json_object_to_json_string(obj);
            char key[128];
            snprintf(key, sizeof(key), "%.*s", (int )CBGR(nkey), (char *)CBGR(key));

            memset(cbop, 0, sizeof(*cbop));
            CBSQ_V0(couchbase_bucket_index("hbs"), LCB_SET, key, strlen(key), updated, strlen(updated), result2, 0, 0, 0, 0)

            return;
        }

        free(cbop);

    } else {
        hm_log(LOG_DEBUG, lg, "Key [%.*s] lcb error %d", (int)CBGR(nkey), (char *)CBGR(key), cbop->lcb_error);
        free(cbop);
    }
}

void set_result(const char *k, long long r)
{
    struct cbop_s *cbop;
    char key[128];

    cbop = malloc(sizeof(*cbop));

    memset(cbop, 0, sizeof(*cbop));

    cbop->data = (void *)r;

    snprintf(key, sizeof(key), "u:%s", k);

    hm_log(LOG_DEBUG, lg, "Updating player: [%s]", key);

    CBGQ_V0(couchbase_bucket_index("hbs"), key, strlen(key), result1, 0, 0)
}
