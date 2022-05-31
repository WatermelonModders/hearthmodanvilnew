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
#ifndef PACKET_H_
#define PACKET_H_

enum packet_e {
    P_GETGAMESTATE 		= 0x01,
    P_CHOOSEOPTION		= 0x02,
    P_CHOOSEENTITIES	= 0x03,
    P_TURNTIMER	        = 0x09,
    P_ENTITIESCHOSEN	= 0x0D,
    P_ALLOPTIONS		= 0x0E,
    P_USERUI 			= 0x0F,
    P_GAMESETUP			= 0x10,
    P_ENTITYCHOICES		= 0x11,
    P_POWERHISTORY 		= 0x13,
    P_SPECTATORNOTIFY   = 0x18,
    P_PING				= 0x73,
    P_PONG				= 0x74,
    P_HANDSHAKE			= 0xA8,
};

struct packet_s {
    int id;
    int len;
    void *data;
};

struct packet_meta_s {
    int id;
    void *(*deserialize)(char **dst, const char *end);
    int (*serialize)(void *p, char **dst, const char *end);
    int (*free)(void *p);
};

#define parse_packet(m_p, m_buf, m_nbuf)\
	char *ptrd##m_buf = m_buf;\
	m_p = deserialize(&ptrd##m_buf, ptrd##m_buf + m_nbuf);

#define build_packet(m_p, m_src, m_type, m_buffer)\
	char m_buffer[8192];\
	char *ptr##m_buffer;\
	int n##m_buffer = 0;\
	ptr##m_buffer = m_buffer;\
	add_packet(&m_p, m_src, m_type);\
	n##m_buffer = serialize(m_p, &ptr##m_buffer, ptr##m_buffer + sizeof(m_buffer));

void packet_free(struct packet_s *p);
struct packet_s *deserialize(char **dst, const char *maxdst);
int serialize(struct packet_s *p, char **dst, const char *maxdst);
struct packet_s *packet_init(enum packet_e id, void *data);
void add_packet(struct packet_s **dst, void *ph, enum packet_e n);

#endif
