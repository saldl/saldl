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

#include "vt100.h"
#include "log.h"
#include "exit.h"

#ifdef HAVE__ISATTY
#include <io.h>
#define SAL_ISATTY _isatty
#elif HAVE_ISATTY
#define SAL_ISATTY isatty
#else
#error Neither _isatty() nor isatty() available.
#endif

#ifdef HAVE__FILENO
#define SAL_FILENO _fileno
#elif HAVE_FILENO
#define SAL_FILENO fileno
#else
#error Neither _fileno() nor fileno() available.
#endif

#if defined(HAVE_IOCTL)
#include <sys/ioctl.h>
#elif !defined(__CYGWIN__) && !defined(__MSYS__) && defined(HAVE_GETCONSOLESCREENBUFFERINFO)
#include <windows.h>
#else
#error Neither ioctl() nor GetConsoleScreenBufferInfo() is available!
#endif

int tty_width() {
  if (! SAL_ISATTY(SAL_FILENO(stderr)) ) {
    return -3;
  }

#if defined(HAVE_IOCTL)
  struct winsize w_size;

  if ( ioctl(SAL_FILENO(stderr), TIOCGWINSZ, &w_size) ) {
    return -1;
  }

  return w_size.ws_col;
#elif !defined(__CYGWIN__) && !defined(__MSYS__) && defined(HAVE_GETCONSOLESCREENBUFFERINFO)
  HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO INFO;

  if (h == INVALID_HANDLE_VALUE) {
    return -2;
  }

  if ( !GetConsoleScreenBufferInfo(h, &INFO) ) {
    return -1;
  }

  return (int)INFO.dwSize.X - 1;  // -1 give us correct behavior in ansicon
#else
#error Neither ioctl() nor GetConsoleScreenBufferInfo() is available!
#endif
}

void set_color(size_t *no_color) {

  SALDL_ASSERT(no_color);

  ret_char = RET_CHAR;
  bold = BOLD;
  invert = INVERT;
  end = END;
  up = UP;
  erase_before = ERASE_BEFORE;
  erase_after = ERASE_AFTER;
  erase_screen_before = ERASE_SCREEN_BEFORE;
  erase_screen_after = ERASE_SCREEN_AFTER;
  if ( *no_color > MAX_NO_COLOR || tty_width() <= 0  ) {
    *no_color = MAX_NO_COLOR;
  }
  switch (*no_color) {
    case MAX_NO_COLOR:
      bold = invert = end = up = erase_before = erase_after = erase_screen_before = erase_screen_after = ret_char = "";
    case 1:
      ok_color = debug_event_color = debug_color = info_color = warn_color = error_color = fatal_color = finish_color = "";
      break;
    default:
      ok_color = OK_COLOR;
      debug_event_color = DEBUG_EVENT_COLOR;
      debug_color = DEBUG_COLOR;
      info_color = INFO_COLOR;
      warn_color = WARN_COLOR;
      error_color = ERROR_COLOR;
      fatal_color = FATAL_COLOR;
      finish_color = FINISH_COLOR;
      break;
  }
  sprintf(debug_event_msg_prefix, "%s%s%s%s ", bold, debug_event_color, DEBUG_EVENT_MSG_TXT, end);
  sprintf(debug_msg_prefix, "%s%s%s%s ", bold, debug_color, DEBUG_MSG_TXT, end);
  sprintf(info_msg_prefix, "%s%s%s%s ", bold, info_color, INFO_MSG_TXT, end);
  sprintf(warn_msg_prefix, "%s%s%s%s ", bold, warn_color, WARN_MSG_TXT, end);
  sprintf(error_msg_prefix, "%s%s%s%s ", bold, error_color, ERROR_MSG_TXT, end);
  sprintf(fatal_msg_prefix, "%s%s%s%s ", bold, fatal_color, FATAL_MSG_TXT, end);
}

void set_verbosity(size_t *verbosity, bool *libcurl_verbosity) {

  SALDL_ASSERT(verbosity);
  SALDL_ASSERT(libcurl_verbosity);

  debug_event_msg = debug_msg = info_msg = warn_msg = err_msg = (log_func *)null_msg;

  if (*verbosity > MAX_VERBOSITY) {
    *verbosity = MAX_VERBOSITY;
  }

  switch (*verbosity) {
    case MAX_VERBOSITY:
    case 6:
      debug_event_msg = &def_debug_event_msg;
    case 5:
      *libcurl_verbosity = true;
    case 4:
      debug_msg = &def_debug_msg;
    case 3:
      info_msg = &def_info_msg;
    case 2:
      warn_msg = &def_warn_msg;
    case 1:
      err_msg = &def_err_msg;
    default:
      break;
  }
}

__attribute__(( format(SALDL_PRINTF_FORMAT,3,0) ))
  static void log_stderr(const char *type, const char *name, const char *format, va_list args) {
    fprintf(stderr, "%s%s%s", ret_char, erase_before, erase_screen_after);
    /* saldl_fputs() is defined in common.c, so we can't use it here */
    if (type) SALDL_ASSERT(EOF != fputs(type, stderr));
    if (name) fprintf(stderr,"%s%s:%s ", bold, name, end);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
  }

void null_msg() {
  return;
}

void def_debug_event_msg(const char *name, const char *format, ...) {
  va_list args;
  va_start(args,format);
  log_stderr(debug_event_msg_prefix, name, format, args);
  va_end(args);
}

void def_debug_msg(const char *name, const char *format, ...) {
  va_list args;
  va_start(args,format);
  log_stderr(debug_msg_prefix, name, format, args);
  va_end(args);
}

void def_info_msg(const char *name, const char *format, ...) {
  va_list args;
  va_start(args,format);
  log_stderr(info_msg_prefix, name, format, args);
  va_end(args);
}

void def_warn_msg(const char *name, const char *format, ...) {
  va_list args;
  va_start(args,format);
  log_stderr(warn_msg_prefix, name, format, args);
  va_end(args);
}

void def_err_msg(const char *name, const char *format, ...) {
  va_list args;
  va_start(args,format);
  log_stderr(error_msg_prefix, name, format, args);
  va_end(args);
}

void main_msg(const char *name, const char *format, ...) {
  va_list args;
  if (format) {
    va_start(args, format);
    log_stderr(info_color, name, format, args);
    va_end(args);
  }
}

void status_msg(const char *name, const char *format, ...) {
  va_list args;
  if (format) {
    va_start(args, format);
    log_stderr("  ", name, format, args);
    va_end(args);
  }
}

void pre_fatal(const char *name, const char *format, ...) {
  /* Use this if you want multi-line fatal messages.
   * The last call should be fatal().
   */
  va_list args;
  if (format) {
    va_start(args,format);
    log_stderr(fatal_msg_prefix, name, format, args);
    va_end(args);
  }
}

void fatal_abort(const char *name, const char *format, ...) {
  va_list args;
  exit_routine();
  if (format) {
    va_start(args,format);
    log_stderr(fatal_msg_prefix, name, format, args);
    va_end(args);
  }
  raise(SIGABRT);
  /* Convince the compiler that this function will not return */
  exit(EXIT_FAILURE);
}

void fatal(const char *name, const char *format, ...) {
  va_list args;
  exit_routine();
  if (format) {
    va_start(args,format);
    log_stderr(fatal_msg_prefix, name, format, args);
    va_end(args);
  }
  exit(EXIT_FAILURE);
}

void finish_msg_and_exit(const char *msg) {
  fprintf(stderr, "%s%s%s%s%s%s\n", erase_before, erase_screen_after, bold, finish_color, msg, end);
  exit(EXIT_SUCCESS);
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
