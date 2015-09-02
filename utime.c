/*
    This file is a part of saldl.

    Copyright (C) 2014-2015 Mohammad AlSaleh <CE.Mohammad.AlSaleh at gmail.com>
    https://github.com/saldl/saldl

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

#if defined(HAVE_CLOCK_MONOTONIC_RAW)

#include <time.h>
double saldl_utime() {
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
  return tp.tv_sec + tp.tv_nsec/1000.0/1000.0/1000.0;
}

#elif defined(HAVE_GETTIMEOFDAY)

#include <stddef.h> // NULL
#include <sys/time.h>

double saldl_utime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec/1000.0/1000.0;
}

#else
#error Both timer implementations are not supported.

#endif

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
