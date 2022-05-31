/*
   hm_base -  hearthmod base library
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
#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <malloc.h>

#include <hmbase.h>

/** Send functions */
char *rb_send_next(struct rb_s *rb, int *size)
{
    assert(rb);

    if(rb->send == NULL) {
        *size = 0;
        return NULL;
    }

    *size = rb->send->len - rb->send->sent;
    // test
    /*
       if(*size >= 10) {
     *size = 10;
     }
     */

    return (char *)(rb->send->buf + rb->send->sent);
}

static void rb_next(struct rb_s *rb)
{
    struct rb_slot_s *next;

    if(rb && rb->send && rb->send->sent == rb->send->len) {
        next = rb->send->next;
        free(rb->send->buf);
        free(rb->send);
        rb->send = next;
        if(rb->send == NULL) {
            rb->tail = NULL;
        }
    }

}

void rb_send_skip(struct rb_s *rb, int offset)
{
    assert(rb);
    assert(rb->send);
    rb->send->sent += offset;

    rb_next(rb);
}

int rb_send_is_empty(struct rb_s *rb)
{
    assert(rb);
    return (rb->send == NULL);
}

void rb_send_pop(struct rb_s *rb, struct hm_pool_s *pool)
{
    rb_next(rb);
}

int rb_send_init(struct rb_s *rb, char *buf, const int len, struct hm_pool_s *pool)
{
    assert(rb);
    struct rb_slot_s *slot;

    slot = malloc(sizeof(*slot));
    if(slot == NULL) {
        return -1;
    }
    slot->buf = malloc(len);
    if(slot->buf == NULL) {
        free(slot);
        return -1;
    }

    memcpy(slot->buf, buf, len);
    slot->len = len;
    slot->sent = 0;
    slot->next = NULL;

    if(rb->send == NULL && rb->tail == NULL) {
        //printf("added to head send init\n");
        rb->send = rb->tail = slot;
    } else {

        //printf("added to tail send init\n");
        assert(rb->tail);
        rb->tail->next = slot;
        rb->tail = slot;
    }

    return 0;
}

/** Receive functions */
char *rb_recv_ptr(struct rb_s *rb, int *used)
{
    assert(rb && used);
    *used = rb->recv.len;
    return (rb->recv.slot + rb->recv.len);
}

void rb_recv_append(struct rb_s *rb, const int len)
{
    assert(rb);
    rb->recv.len += len;
}

char *rb_recv_read(struct rb_s *rb, int *size)
{
    assert(rb && size);
    *size = rb->recv.len;
    return rb->recv.slot;
}

void rb_recv_pop(struct rb_s *rb)
{
    assert(rb);
    rb->recv.len = 0;
}

int rb_recv_is_full(struct rb_s *rb)
{
    assert(rb);
    return (rb->recv.len >= RB_SLOT_SIZE);
}
