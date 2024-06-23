/*
    This file is a part of saldl.

    Copyright (C) 2014-2024 Mohammad AlSaleh <CE.Mohammad.AlSaleh at gmail.com>
    https://saldl.github.io

    saldl is free software: you can redistribute it and/or modify
    it under the terms of the Affero GNU General Public License as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    Affero GNU General Public License for more details.

    You should have received a copy of the Affero GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SALDL_LOG_H
#define SALDL_LOG_H
#else
#error redefining SALDL_LOG_H
#endif

#include "defaults.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h> /* va stuff */
#include <signal.h>
#include <string.h>
#include <errno.h>

#define FN __func__
#define FL __FILE__
#define LN __LINE__

#define MAX_VERBOSITY 7
#define MAX_NO_COLOR 2

#define SALDL_PRINTF_FORMAT printf

#ifdef __MINGW32__
#define SAL_JD "I64d"
#define SAL_JU "I64u"

#ifdef _WIN64
#define SAL_ZU "I64u"
#else
#define SAL_ZU "I32u"
#endif

#else /* #ifdef __MINGW32__ */
#define SAL_JD "jd"
#define SAL_JU "ju"
#define SAL_ZU "zu"
#endif

#define STR(s) #s
#define SALDL_ASSERT(cond) \
  do {\
    if (!(cond)) {\
      pre_fatal(FL, "Assert[%s/%d]: %s", FN, LN, STR((cond)));\
      /* We don't use saldl_fputs() to avoid recursive assertions */ \
      pre_fatal(NULL, "Your system is in bad shape, or this could be a bug in saldl.");\
      fatal_abort(NULL, "Please file a bug report: %s", SALDL_BUG);\
    }\
  } while(0)

/* predefined msg prefixes */
#define DEBUG_EVENT_MSG_TXT "[debug-event]"
#define DEBUG_MSG_TXT "[debug]"
#define INFO_MSG_TXT "[info]"
#define WARN_MSG_TXT "[warning]"
#define ERROR_MSG_TXT "[error]"
#define FATAL_MSG_TXT "[fatal]"

const char* ret_char;

const char *bold;
const char *invert;
const char *end;
const char *up;
const char *erase_before;
const char *erase_after;
const char *erase_screen_before;
const char *erase_screen_after;

const char *ok_color;
const char *debug_event_color;
const char *debug_color;
const char *info_color;
const char *warn_color;
const char *error_color;
const char *fatal_color;
const char *finish_color;

char debug_event_msg_prefix[256];
char debug_msg_prefix[256];
char info_msg_prefix[256];
char warn_msg_prefix[256];
char error_msg_prefix[256];
char fatal_msg_prefix[256];

typedef void(log_func)(const char*, const char*, ...) __attribute__(( format(SALDL_PRINTF_FORMAT,2,3) ));

log_func *debug_event_msg;
log_func *debug_msg;
log_func *info_msg;
log_func *warn_msg;
log_func *err_msg;

void finish_msg_and_exit(const char *msg);

log_func def_debug_event_msg;
log_func def_debug_msg;
log_func def_info_msg;
log_func def_warn_msg;
log_func def_err_msg;
log_func main_msg;
log_func status_msg;
log_func pre_fatal;
log_func  __attribute__((__noreturn__)) fatal_abort;
log_func  __attribute__((__noreturn__)) fatal;
void null_msg(void);

int tty_width(void);
void set_color(size_t *no_color);
void set_verbosity(size_t *verbosity, bool *libcurl_verbosity);

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
