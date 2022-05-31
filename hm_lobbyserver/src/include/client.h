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
#ifndef CLIENT_H_
#define CLIENT_H_

enum packet_e {
	CLIENT_CREATEACCOUNT = 1,
	CLIENT_LOGIN = 2,
	CLIENT_DECK = 3,
};

int proto_add(char **dst, const char *maxdst, const void *src, const int nsrc);
int proto_packet(char *dst, enum packet_e packet, char *src, const int nsrc);
char *read_str(char **buf, char *end, int *len);

void start_game(struct conn_client_s *p1, struct conn_client_s *p2, const int gamehandle);

void client_login(struct conn_client_s *cs, const char *buf, int len);

int matchmaking(struct conn_client_s *player1);

#endif
