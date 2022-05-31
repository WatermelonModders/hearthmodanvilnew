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
#ifndef HMLOG_H_
#define HMLOG_H_

#include <stdio.h>

enum errors_e {
    LOG_EMERG = 0,      ///< system is unusable
    LOG_ALERT,          ///< action must be taken immediately
    LOG_CRIT,           ///< critical conditions
    LOG_ERR,            ///< error conditions
    LOG_WARNING,        ///< warning conditions
    LOG_NOTICE,         //< normal, but significant, condition
    LOG_INFO,           //< informational message
    LOG_DEBUG,          ///< debug-level message
    LOG_MEMORY,         ///< memory-level message
};

struct hm_log_s {
    const char *name;
    int fd;
    FILE *file;
    void *data;
    int priority;
};

#define hm_log(t, l, fmt...)\
    hm_log_impl(0, t, l, __FILE__, __LINE__, __FUNCTION__, fmt)

int hm_log_impl(unsigned long long seq_nb, int priority, struct hm_log_s *log, const char *file, int line, const char *func, const char *fmt, ...) __attribute__ ((format (printf, 7, 8)));
int hm_log_open(struct hm_log_s *l, const char *filename, const int priority);
int hm_log_close(struct hm_log_s *l);

#endif
