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
#ifndef GAME_H_
#define GAME_H_

#define LOG_MECHANICS "{Mechanics}: "

#define HM_GAMESERVER

#include <hsl_cardname.h>
#include <levelup.h>
#include <sections.h>
#include <json.h>
#include <hmbase.h>
#include <async_client.h>
#include <flag_def.h>
#include <proto.h>

#include <common.h>
#include <hs.h>
#include <deck.h>
#include <choose_entities.h>
#include <entity_choices.h>
#include <choose_option.h>
#include <game_options.h>
#include <mechanics.h>
#include <net.h>
#include <level.h>

struct packet_s *game_reply(struct hs_holder_s *p, struct chooseoption_s *opt, struct card_s *attacker, int player);
int game_attack_aoe(struct hs_holder_s *p, struct card_s *attacker, struct card_list_s *defenders, struct card_s **fake_levelup);
void user_ui(struct conn_client_s *c, struct userui_s *u);
int game(struct conn_client_s *c, struct chooseoption_s *opt);
void game_free(struct game_s *g);
void next_turn(struct conn_client_s *c, struct hs_holder_s *p/*, struct chooseoption_s *opt*/);
void mechanics_attack(struct conn_client_s *c, struct chooseoption_s *opt, struct card_s *attacker, struct card_list_s *defenders);
void mechanics_next_turn(struct conn_client_s *c, struct hs_holder_s *p);
void mechanics_put_card_onboard(struct conn_client_s *c, u64 position, struct card_s *attacker, int summoned, struct card_list_s **defenders, struct chooseoption_s *opt, struct deck_s *deck);
void mechanics_attack_aoe(struct conn_client_s *c, u64 position, struct card_s *attacker, struct card_list_s **defenders, struct card_s **fake_levelup);
void mechanics_buff(struct conn_client_s *c, struct chooseoption_s *opt, struct card_s *attacker, struct card_list_s **defenders);
void mechanics_summon(struct conn_client_s *c, struct chooseoption_s *opt, struct card_s *attacker, struct card_list_s **defenders);
void mechanics_draw_card(struct conn_client_s *c, struct chooseoption_s *opt, struct card_s *attacker, int cards, struct card_list_s **defenders);
void mechanics_expire(int game_turn, struct card_s *card, enum expire_e key, int value, const char *name);
void force_expire(struct hs_holder_s *p, struct deck_s *deck, struct card_s *card);
void mechanics_attack_swipe(struct conn_client_s *c, u64 position, struct card_s *attacker, struct card_list_s **defenders);
int entity_attack(struct hs_holder_s *p, struct card_s *attacker, struct card_s *defender);
void put_card_onboard(struct conn_client_s *c, struct hs_holder_s *p, struct chooseoption_s *opt);
void power_attack(struct conn_client_s *c, struct hs_holder_s *p, struct chooseoption_s *opt);

extern struct hm_log_s *lg;

#endif
