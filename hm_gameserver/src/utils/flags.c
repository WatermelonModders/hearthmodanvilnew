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

void flags_diff(struct flags_s *old, struct flags_s *new,
        struct flag_pr_s ***src_added, int *nsrc_added,
        struct flag_pr_s ***src_removed, int *nsrc_removed,
        struct card_s *card)
{
    int i, j;
    unsigned long long t;

    for(i = 0; i < MAX_FLAGS_ARRAY; i++) {
        for(j = 0; j < LL_BITS; j++) {
            t = 1;
            t <<= j;
#define FFILL(f_s, f_n, f_t)\
            *f_s = realloc(*f_s, ++(*f_n) * sizeof(void *));\
            (*f_s)[*f_n - 1] = malloc(sizeof(***f_s));\
            (*f_s)[*f_n - 1]->flag = (i * LL_BITS + j);\
            if(card->priority_override > 0) {\
                (*f_s)[*f_n - 1]->priority = card->priority_override;\
            } else {\
                (*f_s)[*f_n - 1]->priority = flag_defs[(i * LL_BITS + j)];\
            }\
            (*f_s)[*f_n - 1]->type = f_t;\
            (*f_s)[*f_n - 1]->parent_card = card;

            if((old->nibble[i] & t) == t && (new->nibble[i] & t) == 0) {
                FFILL(src_removed, nsrc_removed, F_REMOVED)
            } else if((old->nibble[i] & t) == 0 && (new->nibble[i] & t) == t) {
                FFILL(src_added, nsrc_added, F_ADDED)
            }
        }
    }
}

int flags_copy(struct flags_s *dst, const struct flags_s *src)
{
    int i, j;
    unsigned long long t;

    for(i = 0; i < MAX_FLAGS_ARRAY; i++) {
        for(j = 0; j < LL_BITS; j++) {
            t = 1;
            t <<= j;
            if((src->nibble[i] & t) == t) {
                dst->nibble[i] |= t;
            }
        }
    }
    return 0;

}

int flag_get(struct flags_s *flags, int **f)
{
    assert(flags);

    if(flags == NULL) {
        return -1;
    }

    int i, j, c = 0;
    unsigned long long t;
    for(i = 0; i < MAX_FLAGS_ARRAY; i++) {
        for(j = 0; j < LL_BITS; j++) {
            t = 1;
            t <<= j;
            if((flags->nibble[i] & t) == t) {
                *f = realloc(*f, ++c * sizeof(int));
                (*f)[c - 1] = i * LL_BITS + j;
            }
        }
    }

    return c;
}

int flag(struct flags_s *flags, enum flags_e src, enum flags_action_e action)
{
    int index;
    unsigned long long mod = 1;

    assert(flags);

    if(flags == NULL) {
        return -1;
    }

    if(action == FLAG_DUMP && src == -1) {
        int i, j;
        unsigned long long t;
        for(i = 0; i < MAX_FLAGS_ARRAY; i++) {
            for(j = 0; j < LL_BITS; j++) {
                t = 1;
                t <<= j;
                if((flags->nibble[i] & t) == t) {
                    hm_log(LOG_DEBUG, lg, "flag %p set: %d", flags, i * LL_BITS + j);
                }
            }
        }
        return 0;
    }

    if(src > (MAX_FLAGS_ARRAY * LL_BITS) || src < 0) return -1;

    index = src / LL_BITS;
    mod <<= (src % LL_BITS);

    if(action == FLAG_SET) flags->nibble[index] |= mod;
    else if(action == FLAG_UNSET) flags->nibble[index] &= ~(mod);
    else if(action == FLAG_ISSET) {
        if((flags->nibble[index] & mod) == mod) return 1;
        else return 0;
    } else return -1;

    return 0;
}
