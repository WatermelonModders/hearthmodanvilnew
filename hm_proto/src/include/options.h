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
#ifndef OPTIONS_H_
#define OPTIONS_H_

struct suboption_target_s {
    u64 value;
    struct suboption_target_s *next;
};

struct suboption_s {
    u64 id;
    struct suboption_target_s *target;

    struct suboption_s *next;
};

struct option_s {
    u64 type;
    struct suboption_s *mainoption;
    struct suboption_s *suboptions;

    struct option_s *next;
};

struct alloptions_s {
    u64 id;
    struct option_s *options;
};

void alloptions_free(struct alloptions_s *a);
int alloptions_serialize(void *s, char **dst, const char *maxdst);
int alloptions_size(struct alloptions_s *p);

void option_free(struct option_s *p);
int option_serialize(struct option_s *s, char **dst, const char *maxdst);
int option_size(struct option_s *p);

void suboption_free(struct suboption_s *p);
int suboption_serialize(struct suboption_s *s, char **dst, const char *maxdst);
int suboption_size(struct suboption_s *p);

void alloptions_dump(struct alloptions_s *a);

#endif
