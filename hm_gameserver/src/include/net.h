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
#ifndef NET_H_
#define NET_H_

enum nettype_e {
    NET_BUFF = 1,
    NET_BUFFAOE,
    NET_PUTONBOARD,
    NET_ATTACK,
    NET_SPELL,
    NET_SPELLAOE,
    NET_NEXTTURN,
    NET_SPELLHP_DAMAGE,
};

void net_card_flags(struct powerhistory_data_s **data, struct deck_s *deck_old, struct deck_s *deck_new,
        struct deck_s *opp_old, struct deck_s *opp_new, struct card_list_s *src, struct hs_holder_s *p, int target_player, int nettype);
struct packet_s *net_attack(struct conn_client_s *c, struct chooseoption_s *opt, struct card_s *attacker, struct card_s *defender, int target_player);
int net_send(struct conn_client_s *c, struct packet_s *packet);
int net_send_options(struct conn_client_s *c, struct deck_s *deck, struct deck_s *opponent, int turn);
int net_send_cl_options(struct conn_client_s *c, struct card_list_s *cl, int turn);
struct packet_s *net_next_turn(struct conn_client_s *c, struct card_s **drawn_card, int *zoneposition);
struct packet_s *net_card_onboard(struct hs_holder_s *p, struct chooseoption_s *opt, struct card_s *attacker, int player);
struct packet_s *net_spell_aoe(struct conn_client_s *c, struct chooseoption_s *opt, int player, struct card_s *attacker);
struct packet_s *net_buff(struct conn_client_s *c, struct chooseoption_s *opt, int player, struct card_s *attacker, struct card_list_s *defenders);
struct packet_s *net_spell(struct conn_client_s *c, struct chooseoption_s *opt, int player, struct card_s *attacker, struct card_list_s *defenders, struct card_s *fake_levelup);
struct packet_s *net_spell_hp_damage(struct conn_client_s *c, struct card_list_s *defenders, struct card_s *attacker);
void net_send_turntimer(struct conn_client_s *c, int seconds, int turn, int show);
struct packet_s *net_game_over(struct hs_holder_s *p, int loser);

#endif
