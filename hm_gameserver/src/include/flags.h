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
#ifndef FLAGS_H_
#define FLAGS_H_

#define MAX_FLAGS_ARRAY     16
#define LL_BITS             64

enum flag_type_e {
    F_ADDED = 1,
    F_REMOVED = 2,
};

struct flag_pr_s {
    enum flags_e flag;
    int priority;
    enum flag_type_e type;
    struct card_s *parent_card;
};

// 64 * MAX_FLAGS_ARRAY flags
struct flags_s {
    unsigned long long nibble[MAX_FLAGS_ARRAY];
};

enum flags_action_e {
    FLAG_SET = 0,
    FLAG_UNSET = 1,
    FLAG_ISSET = 2,
    FLAG_DUMP = 3,
};

int flag(struct flags_s *flags, enum flags_e src, enum flags_action_e action);
int flags_copy(struct flags_s *dst, const struct flags_s *src);
void flags_diff(struct flags_s *old, struct flags_s *new,
        struct flag_pr_s ***src_added, int *nsrc_added,
        struct flag_pr_s ***src_removed, int *nsrc_removed,
        struct card_s *parent_card);
int flag_get(struct flags_s *flags, int **f);

#endif
