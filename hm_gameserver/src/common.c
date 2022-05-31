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

const struct ent_s *find_entity(const char *name, const int nname)
{
    int i;

    for(i = 0; i < MAX_ENTITIES; i++) {
        if(nname == entities[i].nname && memcmp(entities[i].name, name, nname) == 0) {
            return &entities[i];
        }
    }

    hm_log(LOG_EMERG, lg, "Entity [%.*s] not found", nname, name);

    return NULL;
}

struct powerhistory_tag_s *add_tag(struct powerhistory_tag_s **dst, const int key, const int value)
{
    struct powerhistory_tag_s *t;

    t = malloc(sizeof(*t));
    if(t == NULL) {
        return NULL;
    }

    t->name = key;
    t->value = value;

    t->next = *dst;
    *dst = t;

    return t;
}

struct powerhistory_game_entity_s *add_game_entity(struct powerhistory_game_entity_s **dst, struct powerhistory_tag_s *tag, const int id)
{
    struct powerhistory_game_entity_s *g;

    g = malloc(sizeof(*g));
    if(g == NULL) {
        return NULL;
    }

    g->id = id;
    g->tag = g->tag_tail = tag;

    *dst = g;

    return g;
}

struct powerhistory_tag_s *default_tags(struct card_s *card, const struct ent_s *ent, int zone_override)
{
    int i;
    struct powerhistory_tag_s *tag = NULL;

    for(i = 0; i < ent->nprop; i++) {
        //#define TEST_COST_
#undef TEST_COST_
#ifdef TEST_COST_
        if(ent->p[i].key == COST) {
            add_tag(&tag, COST, 0);
            continue;
        }
#endif

        // buffed summoned unit
        if(ent->p[i].key == HEALTH && card->health != ent->p[i].value) {
            add_tag(&tag, ent->p[i].key, card->health);
            continue;
        }

        // buffed summoned unit
        if(ent->p[i].key == ATK && card->attack != ent->p[i].value) {
            add_tag(&tag, ent->p[i].key, card->attack);
            continue;
        }

        // don't add default zone 3
        if(ent->p[i].key == ZONE && zone_override > 0) {
            add_tag(&tag, ent->p[i].key, zone_override);
        } else {
            add_tag(&tag, ent->p[i].key, ent->p[i].value);
        }
    }

    if(card) {
        int *f = NULL;
        int c;
        c = flag_get(&card->state, &f);

        for(i = 0; i < c; i++) {
            switch(f[i]) {
                case CARD_TAUNT:
                    add_tag(&tag, TAUNT, 1);
                    break;
                case CARD_WINDFURY:
                    add_tag(&tag, WINDFURY, 1);
                    break;
                case CARD_DIVINE_SHIELD:
                    add_tag(&tag, DIVINE_SHIELD, 1);
                    break;
                case CARD_STEALTH:
                    add_tag(&tag, STEALTH, 1);
                    break;
                case CARD_FREEZE:
                    add_tag(&tag, FREEZE, 1);
                    break;
                case CARD_SPELLPOWER:
                    add_tag(&tag, SPELLPOWER, 1);
                    break;

            }
        }

        if(flag(&card->state, CARD_HEROPOWER, FLAG_ISSET) || flag(&card->state, CARD_HEROPOWER, FLAG_ISSET)) {
            add_tag(&tag, ZONE, 1);
        }
        // add Premium
        //add_tag(&tag, PREMIUM, 1);

        if(f) free(f);
    }

    return tag;
}

void add_hide(struct powerhistory_data_s **dst, u64 entity, u64 zone)
{
    struct powerhistory_data_s *data;
    struct powerhistory_hide_s *hide = NULL;

    hide = malloc(sizeof(*hide));
    hide->entity = entity;
    hide->zone = zone;

    data = malloc(sizeof(*data));
    memset(data, 0, sizeof(*data));
    data->hide = hide;

    if(*dst && (*dst)->tail) {
        (*dst)->tail->next = data;
        (*dst)->tail = data;
    } else {
        *dst = data;
        (*dst)->tail = (*dst)->next = data;
    }

    /*
       data->next = *dst;
     *dst = data;
     */
}

void add_info(struct powerhistory_info_s **dst, u64 infoid)
{
    struct powerhistory_info_s *info = NULL;

    info = malloc(sizeof(*info));
    info->id = infoid;
    info->next = *dst;

    if(dst) *dst = info;
}

void add_meta_info(struct powerhistory_data_s **dst, u64 type, u64 d, struct powerhistory_info_s *info)
{
    struct powerhistory_data_s *data;
    struct powerhistory_meta_s *meta = NULL;

    meta = malloc(sizeof(*meta));
    meta->type = type;
    meta->data = d;

    meta->info = info;

    data = malloc(sizeof(*data));
    memset(data, 0, sizeof(*data));
    data->meta = meta;

    if(*dst && (*dst)->tail) {
        (*dst)->tail->next = data;
        (*dst)->tail = data;
    } else {
        *dst = data;
        (*dst)->tail = (*dst)->next = data;
    }
}

void add_meta(struct powerhistory_data_s **dst, u64 type, u64 d, u64 infoid)
{
    struct powerhistory_data_s *data;
    struct powerhistory_meta_s *meta = NULL;
    struct powerhistory_info_s *info = NULL;

    meta = malloc(sizeof(*meta));
    meta->type = type;
    meta->data = d;

    info = malloc(sizeof(*info));
    memset(info, 0, sizeof(*info));
    info->id = infoid;

    meta->info = info;

    data = malloc(sizeof(*data));
    memset(data, 0, sizeof(*data));
    data->meta = meta;

    if(*dst && (*dst)->tail) {
        (*dst)->tail->next = data;
        (*dst)->tail = data;
    } else {
        *dst = data;
        (*dst)->tail = (*dst)->next = data;
    }
}

void add_tagchange(struct powerhistory_data_s **dst, u64 key, u64 value, u64 id)
{
    struct powerhistory_data_s *data;
    struct powerhistory_tagchange_s *tag = NULL;

    tag = malloc(sizeof(*tag));
    tag->entity = id;
    tag->tag = key;
    tag->value = value;

    data = malloc(sizeof(*data));
    memset(data, 0, sizeof(*data));
    data->tagchange = tag;

    if(*dst && (*dst)->tail) {
        (*dst)->tail->next = data;
        (*dst)->tail = data;
    } else {
        *dst = data;
        (*dst)->tail = (*dst)->next = data;
    }

    /*
       data->next = *dst;
     *dst = data;
     */
}

void add_powerend(struct powerhistory_data_s **dst)
{
    struct powerhistory_data_s *data;
    struct powerhistory_powerend_s *ps = NULL;

    ps = malloc(sizeof(*ps));
    memset(ps, 0, sizeof(*ps));

    data = malloc(sizeof(*data));
    memset(data, 0, sizeof(*data));
    data->powerend = ps;

    if(*dst && (*dst)->tail) {
        (*dst)->tail->next = data;
        (*dst)->tail = data;
    } else {
        *dst = data;
        (*dst)->tail = (*dst)->next = data;
    }

    /*
       data->next = *dst;
     *dst = data;
     */
}

void add_powerstart(struct powerhistory_data_s **dst, u64 type, u64 index, u64 source, u64 target, char *card_id, int ncard_id)
{
    struct powerhistory_data_s *data;
    struct powerhistory_powerstart_s *ps = NULL;

    ps = malloc(sizeof(*ps));
    ps->type = type;
    ps->index = index;
    ps->source = source;
    ps->target = target;
    ps->ncard_id = ncard_id;
    ps->card_id = card_id;

    data = malloc(sizeof(*data));
    memset(data, 0, sizeof(*data));
    data->powerstart = ps;

    if(*dst && (*dst)->tail) {
        (*dst)->tail->next = data;
        (*dst)->tail = data;
    } else {
        *dst = data;
        (*dst)->tail = (*dst)->next = data;
    }

    /*
       data->next = *dst;
     *dst = data;
     */
}

void add_tagschange(struct powerhistory_data_s **dst, const struct ent_s *ent, u64 id)
{
    int i;

    for(i = 0; i < ent->nprop; i++) {
        add_tagchange(dst, ent->p[i].key, ent->p[i].value, id);
    }

    add_tagchange(dst, ENTITY_ID, id, id);
}

void add_player(struct powerhistory_player_s **dst, u64 id, u64 bhi, u64 blo, u64 cardback, struct powerhistory_game_entity_s *ent)
{
    struct powerhistory_player_s *p;

    p = malloc(sizeof(*p));

    p->id = id;
    p->bnet_hi = bhi;
    p->bnet_lo = blo;
    p->cardback = cardback;

    p->entity = ent;
    p->next = *dst;
    *dst = p;
}

void add_game_start(struct powerhistory_creategame_s **dst, struct powerhistory_game_entity_s *ge, struct powerhistory_player_s *p)
{
    *dst = malloc(sizeof(**dst));

    (*dst)->game_entity = ge;
    (*dst)->player = p;
}

void add_ph_data_full(struct powerhistory_data_s **dst, struct powerhistory_entity_s *full)
{
    struct powerhistory_data_s *p;

    p = malloc(sizeof(*p));
    memset(p, 0, sizeof(*p));

    if(full) {
        p->full = full;
    }

    if(*dst && (*dst)->tail) {
        (*dst)->tail->next = p;
        (*dst)->tail = p;
    } else {
        *dst = p;
        (*dst)->tail = (*dst)->next = p;
    }
}


void add_ph_data_show(struct powerhistory_data_s **dst, struct powerhistory_entity_s *show)
{
    struct powerhistory_data_s *p;

    p = malloc(sizeof(*p));
    memset(p, 0, sizeof(*p));

    if(show) {
        p->show = show;
    }

    if(*dst && (*dst)->tail) {
        (*dst)->tail->next = p;
        (*dst)->tail = p;
    } else {
        *dst = p;
        (*dst)->tail = (*dst)->next = p;
    }
}

void add_ph_data(struct powerhistory_data_s **dst, struct powerhistory_entity_s *full, struct powerhistory_creategame_s *cg)
{
    struct powerhistory_data_s *p;

    p = malloc(sizeof(*p));
    memset(p, 0, sizeof(*p));

    if(cg) {
        p->creategame = cg;
    }

    if(full) {
        p->full = full;
    }

    if(*dst && (*dst)->tail) {
        (*dst)->tail->next = p;
        (*dst)->tail = p;
    } else {
        *dst = p;
        (*dst)->tail = (*dst)->next = p;
    }
    /*
     *dst->tail = p;
     p->next = *dst;
     *dst = p;
     */
}

void add_full_entity(struct powerhistory_entity_s **full, struct powerhistory_tag_s *tags, u64 id, const char *name, const int nname)
{
    *full = malloc(sizeof(**full));
    memset(*full, 0, sizeof(**full));

    (*full)->entity = id;
    (*full)->tag = tags;
    (*full)->nname = nname;
    (*full)->name = (char *)name;
}

void add_ph(struct powerhistory_s **dst, struct powerhistory_data_s *data)
{
    *dst = malloc(sizeof(**dst));
    memset(*dst, 0, sizeof(**dst));

    (*dst)->data = data;
}

void add_packet(struct packet_s **dst, void *ph, enum packet_e n)
{
    *dst = malloc(sizeof(**dst));
    memset(*dst, 0, sizeof(**dst));

    (*dst)->data = ph;
    (*dst)->id = n;
}

void bin_dump(char *buf, const int len)
{
    int i;

    printf("binary dump of %d:\n", len);
    for(i = 0; i < len; i++) {
        printf("%x|", (unsigned char)buf[i]);
    }
    printf("\n");
}
