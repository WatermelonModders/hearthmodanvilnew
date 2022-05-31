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
#ifndef COMMON_H_
#define COMMON_H_

const struct ent_s *find_entity(const char *name, const int nname);
struct powerhistory_tag_s *add_tag(struct powerhistory_tag_s **dst, const int key, const int value);
struct powerhistory_game_entity_s *add_game_entity(struct powerhistory_game_entity_s **dst, struct powerhistory_tag_s *tag, const int id);
struct powerhistory_tag_s *default_tags(struct card_s *card, const struct ent_s *ent, int zone_override);
void add_tagchange(struct powerhistory_data_s **dst, u64 key, u64 value, u64 id);
void add_powerstart(struct powerhistory_data_s **dst, u64 type, u64 index, u64 source, u64 target, char *card_id, int ncard_id);
void add_tagschange(struct powerhistory_data_s **dst, const struct ent_s *ent, u64 id);
void add_player(struct powerhistory_player_s **dst, u64 id, u64 bhi, u64 blo, u64 cardback, struct powerhistory_game_entity_s *ent);
void add_game_start(struct powerhistory_creategame_s **dst, struct powerhistory_game_entity_s *ge, struct powerhistory_player_s *p);
void add_ph_data(struct powerhistory_data_s **dst, struct powerhistory_entity_s *full, struct powerhistory_creategame_s *cg);
void add_ph_data(struct powerhistory_data_s **dst, struct powerhistory_entity_s *full, struct powerhistory_creategame_s *cg);
void add_ph(struct powerhistory_s **dst, struct powerhistory_data_s *data);
void add_packet(struct packet_s **dst, void *ph, enum packet_e n);
void add_hide(struct powerhistory_data_s **dst, u64 entity, u64 zone);
void add_powerend(struct powerhistory_data_s **dst);
void add_full_entity(struct powerhistory_entity_s **full, struct powerhistory_tag_s *tags, u64 id, const char *name, const int nname);
void add_ph_data_show(struct powerhistory_data_s **dst, struct powerhistory_entity_s *show);
void add_meta(struct powerhistory_data_s **dst, u64 type, u64 d, u64 infoid);
void add_info(struct powerhistory_info_s **dst, u64 infoid);
void add_meta_info(struct powerhistory_data_s **dst, u64 type, u64 d, struct powerhistory_info_s *info);
void add_ph_data_full(struct powerhistory_data_s **dst, struct powerhistory_entity_s *full);

void bin_dump(char *buf, const int len);
int random_number(int max);
void register_turntimer(struct conn_client_s *c);

extern struct ht_s **clients;
extern struct ev_loop *loop;

#endif
