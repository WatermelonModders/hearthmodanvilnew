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
#ifndef DEF_H_
#define DEF_H_

#define error()\
    hm_log(LOG_EMERG, lg, "dst: %p end: %p n: %d", *dst, maxdst, n);\
abort();

struct powerhistory_tag_s {
    u64 name;
    u64 value;

    struct powerhistory_tag_s *next;
};

struct powerhistory_hide_s {
    u64 entity;
    u64 zone;
};

struct powerhistory_tagchange_s {
    u64 entity;
    u64 tag;
    u64 value;
};

struct powerhistory_entity_s {
    u64 entity;
    char *name;
    int nname;

    struct powerhistory_tag_s *tag, *tag_tail;
};

struct powerhistory_powerstart_s {
    u64 type;
    u64 index;
    u64 source;
    u64 target;

    int ncard_id;
    char *card_id;
};

struct powerhistory_powerend_s {
    int i;
};

struct powerhistory_game_entity_s {
    u64 id;
    struct powerhistory_tag_s *tag, *tag_tail;
};

struct powerhistory_s {
    struct powerhistory_data_s *data, *data_tail;
};

struct powerhistory_info_s {
    u64 id;
    struct powerhistory_info_s *next;
};

struct powerhistory_meta_s {
    u64 type;
    u64 data;
    struct powerhistory_info_s *info, *info_tail;
};

struct powerhistory_player_s {
    u64 id;
    u64 bnet_hi;
    u64 bnet_lo;
    u64 cardback;
    struct powerhistory_game_entity_s *entity;

    struct powerhistory_player_s *next;
};

struct powerhistory_creategame_s {
    struct powerhistory_game_entity_s *game_entity;
    struct powerhistory_player_s *player, *player_tail;
};

struct powerhistory_data_s {
    struct powerhistory_entity_s *full;
    struct powerhistory_entity_s *show;
    struct powerhistory_hide_s *hide;
    struct powerhistory_tagchange_s *tagchange;
    struct powerhistory_creategame_s *creategame;
    struct powerhistory_powerstart_s *powerstart;
    struct powerhistory_powerend_s *powerend;
    struct powerhistory_entity_s *change_entity;
    struct powerhistory_meta_s *meta;

    struct powerhistory_data_s *next, *tail;
};

void powerhistory_free(struct powerhistory_s *p);
void *powerhistory_deserialize(char **dst, const char *maxdst);
int powerhistory_serialize(void *data, char **dst, const char *maxdst);
int powerhistory_size(struct powerhistory_s *ph);

struct powerhistory_data_s *powerhistory_data_deserialize(char **dst, const char *maxdst);
int powerhistorydata_serialize(struct powerhistory_data_s *ph, char **dst, const char *maxdst);
int powerhistorydata_size(struct powerhistory_data_s *ph);
void powerhistory_data_free(struct powerhistory_data_s *d);

struct powerhistory_meta_s *meta_deserialize(char **dst, const char *maxdst);
int meta_serialize(struct powerhistory_meta_s *ph, char **dst, const char *maxdst);
int meta_size(struct powerhistory_meta_s *ph);
void meta_free(struct powerhistory_meta_s *t);

struct powerhistory_powerend_s *powerend_deserialize(char **dst, const char *maxdst);
int powerend_serialize(struct powerhistory_powerend_s *ph, char **dst, const char *maxdst);
int powerend_size(struct powerhistory_powerend_s *ph);
void powerend_free(struct powerhistory_powerend_s *p);

struct powerhistory_powerstart_s *powerstart_deserialize(char **dst, const char *maxdst);
int powerstart_serialize(struct powerhistory_powerstart_s *ph, char **dst, const char *maxdst);
int powerstart_size(struct powerhistory_powerstart_s *ph);
void powerstart_free(struct powerhistory_powerstart_s *p);

struct powerhistory_creategame_s *creategame_deserialize(char **dst, const char *maxdst);
int creategame_serialize(struct powerhistory_creategame_s *ph, char **dst, const char *maxdst);
int creategame_size(struct powerhistory_creategame_s *ph);
void creategame_free(struct powerhistory_creategame_s *c);

struct powerhistory_tagchange_s *tagchange_deserialize(char **dst, const char *maxdst);
int tagchange_serialize(struct powerhistory_tagchange_s *ph, char **dst, const char *maxdst);
int tagchange_size(struct powerhistory_tagchange_s *ph);
void tagchange_free(struct powerhistory_tagchange_s *t);

struct powerhistory_hide_s *hide_deserialize(char **dst, const char *maxdst);
int hide_serialize(struct powerhistory_hide_s *ph, char **dst, const char *maxdst);
int hide_size(struct powerhistory_hide_s *ph);
void hide_free(struct powerhistory_hide_s *t);

struct powerhistory_game_entity_s *game_entity_deserialize(char **dst, const char *maxdst);
int game_entity_serialize(struct powerhistory_game_entity_s *ph, char **dst, const char *maxdst);
int game_entity_size(struct powerhistory_game_entity_s *ph);
void game_entity_free(struct powerhistory_game_entity_s *g);

struct powerhistory_entity_s *entity_deserialize(char **dst, const char *maxdst);
int entity_serialize(struct powerhistory_entity_s *ph, char **dst, const char *maxdst);
int entity_size(struct powerhistory_entity_s *ph);
void entity_free(struct powerhistory_entity_s *e);

struct powerhistory_tag_s *tag_deserialize(char **dst, const char *maxdst);
int tag_serialize(struct powerhistory_tag_s *ph, char **dst, const char *maxdst);
int tag_size(struct powerhistory_tag_s *ph);
void tag_free(struct powerhistory_tag_s *t);

void player_free(struct powerhistory_player_s *p);
struct powerhistory_player_s *player_deserialize(char **dst, const char *maxdst);
int player_serialize(struct powerhistory_player_s *ph, char **dst, const char *maxdst);
int player_size(struct powerhistory_player_s *ph);

int bnet_size(struct powerhistory_player_s *ph);

void powerhistory_dump(struct powerhistory_s *p);

#endif
