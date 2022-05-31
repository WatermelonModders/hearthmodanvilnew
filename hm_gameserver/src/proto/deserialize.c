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
#include <proto.h>

#define MAP_PACKET_MAX  12

typedef int (*f)(void *c);

static struct packet_deserialize_s map_packet[MAP_PACKET_MAX] = {
    {P_GETGAMESTATE,    NULL,                       NULL,                       NULL},
    {P_CHOOSEOPTION,    chooseoption_deserialize,   NULL,                       (f)chooseoption_free},
    {P_TURNTIMER,       NULL,                       turntimer_serialize,        (f)turntimer_free},
    {P_CHOOSEENTITIES,  chooseentities_deserialize, NULL,                       (f)chooseentities_free},
    {P_ALLOPTIONS,      NULL,                       alloptions_serialize,       (f)alloptions_free},
    {P_ENTITYCHOICES,   NULL,                       entitychoices_serialize,    (f)entitychoices_free},
    {P_GAMESETUP,       NULL,                       gamesetup_serialize,        (f)gamesetup_free},
    {P_USERUI,          userui_deserialize,         userui_serialize,           (f)userui_free},
    {P_POWERHISTORY,    powerhistory_deserialize,   powerhistory_serialize,     (f)powerhistory_free},
    {P_HANDSHAKE,       handshake_deserialize,      NULL,                       (f)handshake_free},
    {P_PING,            NULL,                       NULL,                       NULL},
    {P_PONG,            NULL,                       NULL,                       NULL},
};

struct packet_s *deserialize(char **dst, const char *maxdst)
{
    struct packet_s *p;
    int i;

    if(!(dst != NULL && *dst != NULL && *dst < maxdst)) {
        return NULL;
    }

    p = malloc(sizeof(*p));

    p->id = read_mem_int(dst, maxdst);
    p->len = read_mem_int(dst, maxdst);

    for(i = 0; i < MAP_PACKET_MAX; i++) {
        if(map_packet[i].id == p->id) {
            if(map_packet[i].deserialize) {
                p->data = map_packet[i].deserialize(dst, maxdst);
                assert(p->data);
            }
            return p;
        }
    }

    free(p);

    return NULL;
}

void packet_free(struct packet_s *p)
{
    int i;

    for(i = 0; i < MAP_PACKET_MAX; i++) {
        if(map_packet[i].id == p->id) {
            if(map_packet[i].free) {
                map_packet[i].free(p->data);
            }

            free(p);
            break;
        }
    }
}

int serialize(struct packet_s *p, char **dst, const char *maxdst)
{
    int i, n = 0;
    char *off_len;

    write_mem_int(dst, maxdst, p->id);
    off_len = *dst;
    write_mem_int(dst, maxdst, 0);		// make space for length

    n += 2 * sizeof(int);

    for(i = 0; i < MAP_PACKET_MAX; i++) {
        if(map_packet[i].id == p->id) {
            if(map_packet[i].serialize) {
                n += map_packet[i].serialize(p->data, dst, maxdst);
            }

            write_mem_int(&off_len, maxdst, n - (2 * sizeof(int)));

            return n;
        }
    }

    return -1;
}

struct packet_s *packet_init(enum packet_e id, void *data)
{
    struct packet_s *p;

    p = malloc(sizeof(*p));

    p->id = id;
    p->data = data;

    return p;
}
