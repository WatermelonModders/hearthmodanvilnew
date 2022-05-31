/*
   hm_base - hearthmod base library
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
#ifndef RB_H
#define RB_H

#define RB_SLOT_SIZE    (16 * 1024)


struct rb_slot_s {
    void *buf;
    int len;
    int sent;
    struct rb_slot_s *next;
};

struct rb_s {
    struct {
        char slot[RB_SLOT_SIZE];
        int len;
    } recv;

    struct rb_slot_s *send, *tail;
};

char *rb_send_next(struct rb_s *rb, int *size);
void rb_send_skip(struct rb_s *rb, int offset);
int rb_send_is_empty(struct rb_s *rb);
void rb_send_pop(struct rb_s *rb, struct hm_pool_s *pool);
int rb_send_init(struct rb_s *rb, char *buf, const int len, struct hm_pool_s *pool);
char *rb_recv_ptr(struct rb_s *rb, int *used);
void rb_recv_append(struct rb_s *rb, const int len);
char *rb_recv_read(struct rb_s *rb, int *size);
void rb_recv_pop(struct rb_s *rb);
int rb_recv_is_full(struct rb_s *rb);

#endif
