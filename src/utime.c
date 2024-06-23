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

#include "log.h"

#ifdef HAVE_CLOCK_MONOTONIC_RAW

#include <time.h>
double saldl_utime(void) {
  struct timespec tp;
  int ret = clock_gettime(CLOCK_MONOTONIC_RAW, &tp);

  if (ret) {
    fatal(FN, "clock_gettime() failed, %s.", strerror(errno));
  }

  return tp.tv_sec + tp.tv_nsec/1000.0/1000.0/1000.0;
}

#elif defined(HAVE_GETTIMEOFDAY)

#include <sys/time.h>

double saldl_utime(void) {
  struct timeval tv;
  int ret = gettimeofday(&tv, NULL);

  if (ret) {
    fatal(FN, "gettimeofday() failed, %s.", strerror(errno));
  }

  return tv.tv_sec + tv.tv_usec/1000.0/1000.0;
}

#else
#error Both timer implementations are not supported.

#endif

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
