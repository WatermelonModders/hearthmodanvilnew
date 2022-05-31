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

void powerhistory_free(struct powerhistory_s *p)
{
    struct powerhistory_data_s *d, *del;

    for(d = p->data; d != NULL; del = d, d = d->next, powerhistory_data_free(del));

    free(p);
}

void *powerhistory_deserialize(char **dst, const char *maxdst)
{
    struct powerhistory_s *p;
    struct powerhistory_data_s *d;
    int n, len;

    p = malloc(sizeof(*p));
    p->data = NULL;
    p->data_tail = NULL;

    while(*dst < maxdst) {
        n = read_byte(dst, maxdst);
        len = read_uint(dst, maxdst);

        if(n == 10) {
            d = powerhistory_data_deserialize(dst, *dst + len);
            d->next = NULL;

            if(p->data == NULL && p->data_tail == NULL) {
                p->data = d;
                p->data_tail = d;
            } else {
                p->data_tail->next = d;
                p->data_tail = d;
            }

        } else {
            error();
            return NULL;
        }
    }

    return p;
}

int powerhistory_serialize(void *data, char **dst, const char *maxdst)
{
    int n, o = 0;
    struct powerhistory_data_s *d;
    struct powerhistory_s *ph = data;

    if(ph->data) {
        for(d = ph->data; d != NULL; d = d->next) {
            write_byte(dst, maxdst, 10);
            o += 1;
            n = powerhistorydata_size(d);
            write_uint(dst, maxdst, n);
            o += sizeofu32(n) + n;
            powerhistorydata_serialize(d, dst, maxdst);
        }
    }

    return o;
}

int powerhistory_size(struct powerhistory_s *ph)
{
    struct powerhistory_data_s *d;
    int n, num = 0;

    if(ph->data) {
        for(d = ph->data; d != NULL; d = d->next) {
            num += 1;
            n = powerhistorydata_size(d);
            num += n + sizeofu32(n);
        }
    }

    return num;
}

void powerhistory_dump(struct powerhistory_s *p)
{
    struct powerhistory_tag_s *t;
    struct powerhistory_data_s *d;
    struct powerhistory_game_entity_s *ge;
    struct powerhistory_player_s *player;
    struct powerhistory_info_s *i;
    int it = 0;

    assert(p);

    for(d = p->data; d != NULL; d = d->next) {
        hm_log(LOG_DEBUG, lg, "Data %d", it++);

        if(d->full) {
            hm_log(LOG_DEBUG, lg, "data->full:");
            hm_log(LOG_DEBUG, lg, "\tentity: %lld name: [%.*s]", d->full->entity, d->full->nname, d->full->name);
            for(t = d->full->tag; t != NULL; t = t->next) {
                hm_log(LOG_DEBUG, lg, "\t\ttag name: %lld value: %lld", t->name, t->value);
            }
        }

        if(d->show) {
            hm_log(LOG_DEBUG, lg, "data->show:");
            hm_log(LOG_DEBUG, lg, "\tentity: %lld name: [%.*s]", d->show->entity, d->show->nname, d->show->name);
            for(t = d->show->tag; t != NULL; t = t->next) {
                hm_log(LOG_DEBUG, lg, "\t\ttag name: %lld value: %lld", t->name, t->value);
            }
        }

        if(d->hide) {
            hm_log(LOG_DEBUG, lg, "data->hide:");
            hm_log(LOG_DEBUG, lg, "\tentity: %lld zone: %lld", d->hide->entity, d->hide->zone);
        }

        if(d->tagchange) {
            hm_log(LOG_DEBUG, lg, "data->tagchange:");
            hm_log(LOG_DEBUG, lg, "\tentity: %lld tag: %lld value: %lld", d->tagchange->entity, d->tagchange->tag, d->tagchange->value);
        }

        if(d->creategame) {
            ge = d->creategame->game_entity;
            hm_log(LOG_DEBUG, lg, "game entity id: %lld", ge->id);
            for(t = ge->tag; t != NULL; t = t->next) {
                hm_log(LOG_DEBUG, lg, "\ttag name: %lld value: %lld", t->name, t->value);
            }

            for(player = d->creategame->player; player != NULL; player = player->next) {
                hm_log(LOG_DEBUG, lg, "\t\tdata->creategame->player:");
                hm_log(LOG_DEBUG, lg, "\t\t\tid: %lld bhi: %lld blo: %lld cardback: %lld", player->id, player->bnet_hi, player->bnet_lo, player->cardback);
                ge = player->entity;
                hm_log(LOG_DEBUG, lg, "\t\t\tplayer->entity: %lld", ge->id);
                for(t = ge->tag; t != NULL; t = t->next) {
                    hm_log(LOG_DEBUG, lg, "\t\t\t\ttag name: %lld value: %lld", t->name, t->value);
                }
            }
        }

        if(d->powerstart) {
            hm_log(LOG_DEBUG, lg, "data->powerstart");
            hm_log(LOG_DEBUG, lg, "\ttype: %lld index: %lld source: %lld target: %lld card: [%.*s]", d->powerstart->type, d->powerstart->index, d->powerstart->source, d->powerstart->target, d->powerstart->ncard_id, d->powerstart->card_id);
        }

        if(d->powerend) {
            hm_log(LOG_DEBUG, lg, "data->powerend");
        }

        if(d->change_entity) {
            hm_log(LOG_DEBUG, lg, "data->change_entity:");
            hm_log(LOG_DEBUG, lg, "\tentity: %lld name: [%.*s]", d->change_entity->entity, d->change_entity->nname, d->change_entity->name);
            for(t = d->change_entity->tag; t != NULL; t = t->next) {
                hm_log(LOG_DEBUG, lg, "\t\ttag name: %lld value: %lld", t->name, t->value);
            }
        }

        if(d->meta) {
            hm_log(LOG_DEBUG, lg, "data->meta:");
            hm_log(LOG_DEBUG, lg, "\ttype: %lld data: %lld", d->meta->type, d->meta->data);
            for(i = d->meta->info; i != NULL; i = i->next) {
                hm_log(LOG_DEBUG, lg, "\t\t\tinfo id: %lld", i->id);
            }
        }
    }
}
