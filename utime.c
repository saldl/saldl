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
