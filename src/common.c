/*
    This file is a part of saldl.

    Copyright (C) 2014-2015 Mohammad AlSaleh <CE.Mohammad.AlSaleh at gmail.com>
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

#if defined(HAVE_GETMODULEFILENAME) || defined(HAVE__MKDIR)
#include <winsock2.h>
#include <windows.h>
#define NAME_MAX MAX_PATH
#endif


#ifdef HAVE__MKDIR
#include <direct.h> // _mkdir()
#endif

#include <math.h> // for HUGE_VAL
#include "common.h"

/* .part.sal , .ctrl.sal len is 9 */
#define EXT_LEN 9


#ifdef HAVE_GETMODULEFILENAME
char* windows_exe_path() {
  char path[PATH_MAX];
  char *sep_pos;

  intmax_t ret = GetModuleFileName(NULL, path, PATH_MAX);

  if (!ret) {
    return NULL;
  }

  if ( (sep_pos = strrchr(path, '\\')) ) {
    *sep_pos = '\0';
  }

  return saldl_strdup(path);
}
#endif

char* saldl_lstrip(char *str) {
  char *tmp = str;
  SALDL_ASSERT(tmp);

  while (*tmp == ' ' || *tmp == '\t') {
    tmp++;
  }

  return tmp;
}

/* 0 nmemb is banned, 0 size is banned */
void* saldl_calloc(size_t nmemb, size_t size) {
  void *p = NULL;
  SALDL_ASSERT(size);
  SALDL_ASSERT(nmemb);
  p = calloc(nmemb, size);
  SALDL_ASSERT(p);
  return p;
}

/* 0 size is banned */
void* saldl_malloc(size_t size) {
  void *p = NULL;
  SALDL_ASSERT(size);
  p = malloc(size);
  SALDL_ASSERT(p);
  return p;
}

/* NULL ptr is banned, 0 size is banned */
void* saldl_realloc(void *ptr, size_t size) {
  void *p = NULL;
  SALDL_ASSERT(size);
  SALDL_ASSERT(ptr);
  p = realloc(ptr, size);
  SALDL_ASSERT(p);
  return p;
}

char* saldl_strdup(const char *str) {
  char *dup = NULL;
  SALDL_ASSERT(str);
  dup = strdup(str);
  SALDL_ASSERT(dup);
  return dup;
}

void saldl_fflush(FILE *f) {
  int ret;
  SALDL_ASSERT(f);
  ret = fflush(f);
  if (ret) {
    fatal(FN, "%s\n", strerror(errno));
  }
}

long fsize(FILE *f) {
  long curr;
  long size;

  SALDL_ASSERT(f);

  curr = ftell(f);
  fseek(f, 0l, SEEK_END);
  size = ftell(f);
  fseek(f, curr, SEEK_SET);
  return size;
}

off_t fsizeo(FILE *f) {
  off_t curr;
  off_t size;

  SALDL_ASSERT(f);

  curr = ftello(f);
  fseeko(f, 0, SEEK_END);
  size = ftello(f);
  fseeko(f, curr, SEEK_SET);
  return size;
}

off_t fsize2(char *fname) {
  int ret;
  struct stat st = {0};

  SALDL_ASSERT(fname);

  ret = stat(fname, &st);
  return ret ? ret : st.st_size;
}

int saldl_mkdir(const char *path, mode_t mode) {
  SALDL_ASSERT(path);
#ifdef HAVE__MKDIR
  (void) mode;
  return _mkdir(path);
#elif defined(HAVE_MKDIR)
  return mkdir(path, mode);
#else
#error neither mkdir() nor _mkdir() available.
#endif
}

void saldl_pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg) {
  int ret = pthread_create(thread, attr, start_routine, arg);

  if (ret) {
    pre_fatal(FN, "Failed with error: %s.\n", strerror(ret));
    fatal(FN, "Please check your privileges and system limits.\n");
  }
}

void saldl_block_sig_pth() {
#ifdef HAVE_SIGADDSET
  /* Don't interrupt thread, Let the signal handler work properly */
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTERM);
  SALDL_ASSERT(!pthread_sigmask(SIG_SETMASK, &set, NULL));
#endif
}

void saldl_unblock_sig_pth() {
#ifdef HAVE_SIGADDSET
  /* Unblock any blocked signals, */
  sigset_t set;
  sigemptyset(&set);
  SALDL_ASSERT(!pthread_sigmask(SIG_SETMASK, &set, NULL));

#endif
}

#ifdef HAVE_SIGACTION
void ignore_sig(int sig, struct sigaction *sa_save) {
  struct sigaction sa_ign;
  sigaction(sig, NULL, sa_save);
  sa_ign = *sa_save;
  sa_ign.sa_handler = SIG_IGN;
  sigaction(sig, &sa_ign, NULL);
}

void restore_sig_handler(int sig, struct sigaction *sa_restore) {
  sigaction(sig, sa_restore, NULL);
}
#endif

void fputs_count(uintmax_t count, const char* str, FILE* stream) {
  for (uintmax_t loops = 1; loops <= count; loops++) {
    fputs(str, stream);
  }
}

double human_size(double size) {
  if (size > 1000*1000*1000) {
    return size/1024/1024/1024;
  }
  else if (size > 1000*1000) {
    return size/1024/1024;
  }
  else if (size > 1000) {
    return size/1024;
  }
  else {
    return size;
  }
}

const char* human_size_suffix(double size) {
  if (size > 1000*1000*1000) {
    return "GiB";
  } else if (size > 1000*1000) {
    return "MiB";
  } else if (size > 1000) {
    return "KiB";
  } else {
    return "B";
  }
}

size_t s_num_digits(intmax_t num) {
	return snprintf(NULL, 0, "%jd", num);
}

size_t u_num_digits(uintmax_t num) {
	return snprintf(NULL, 0, "%ju", num);
}

size_t saldl_min(size_t a, size_t b) {
  return a < b ? a : b;
}

size_t saldl_max(size_t a, size_t b) {
  return a > b ? a : b;
}

off_t saldl_max_o(off_t a, off_t b) {
  return a > b ? a : b;
}

size_t saldl_max_z_umax(uintmax_t a, uintmax_t b) {
  uintmax_t ret = a > b ? a : b;
  if ((intmax_t)ret < 0 || ret > (uintmax_t)SIZE_MAX) {
    fatal(FN, "Max value %jd is out of size_t range 0-%zu.\n", (intmax_t)ret, SIZE_MAX);
  }
  return (size_t)ret;
}

char* valid_filename(const char *pre_valid) {
  char *corrected_name;

  SALDL_ASSERT(pre_valid);

  corrected_name = saldl_strdup(pre_valid);
  while ( strchr(corrected_name,'/') ) {
    memset(strchr(corrected_name,'/') , '_', 1);
  }
  while ( strchr(corrected_name,':') ) {
    memset(strchr(corrected_name,':') , '_', 1);
  }
  return corrected_name;
}

char* trunc_filename(const char *pre_trunc, int keep_ext) {
  char *trunc_name;
  char *pre_trunc_copy, *tmp_dirname, *tmp_basename; /* use with basename(), dirname() */

  char cwd[PATH_MAX];
  char dir_name[PATH_MAX] = "";
  char *base_name;

  char *ext_str, *ext_str_empty;
  size_t ext_len = 0;

  SALDL_ASSERT(pre_trunc);
  trunc_name = saldl_strdup(pre_trunc); /* allocates enough bits */

  pre_trunc_copy = saldl_strdup(pre_trunc);
  tmp_dirname = dirname(pre_trunc_copy);
  if ( tmp_dirname[0] != '.' ) {
    snprintf(dir_name,PATH_MAX,"%s/", tmp_dirname);
  }
  free(pre_trunc_copy);

  ext_str = ext_str_empty = saldl_strdup("");
  if (keep_ext) {
    ext_str = strrchr(pre_trunc, '.');
    if (!ext_str) ext_str = ext_str_empty;
    ext_len = strlen(ext_str);
  }

  pre_trunc_copy = saldl_strdup(pre_trunc);
  tmp_basename = basename(pre_trunc_copy);
  tmp_basename[ strlen(tmp_basename) - ext_len ] = '\0';
  base_name = saldl_strdup(tmp_basename);
  snprintf(base_name,NAME_MAX-EXT_LEN-ext_len,"%s", tmp_basename);
  free(pre_trunc_copy);

  snprintf(trunc_name,
           PATH_MAX - EXT_LEN - u_num_digits(SIZE_MAX) - (dir_name[0]=='/'?0:strlen(getcwd(cwd,PATH_MAX))+1),
           "%s%s%s",
           dir_name,
           base_name,
           ext_str);
  free(base_name);
  free(ext_str_empty);
  return trunc_name;
}

size_t parse_num_d(const char *num_char) {
  double num;
  char *p[1]; /* **p must be initialized and not NULL */

  SALDL_ASSERT(num_char);
  num = strtod(num_char, p);

  if (strlen(*p)) {
    fatal(FN, "Invalid value passed: '%s', expected a floating number.\n", num_char);
  }

  if (num <= 0) {
    fatal(FN, "Expected a positive value, %s(%lf) was passed.\n", num_char, num);
  }

  if (num == HUGE_VAL) {
    fatal(FN, "Value: %s parsed as %lf is considered huge.\n", num_char, num);
  }

  return num;
}

size_t parse_num_z(const char *num_char, size_t suff_len) {
  uintmax_t num;
  char *p[1]; /* **p must be initialized and not NULL */

  SALDL_ASSERT(num_char);
  num = strtoumax(num_char, p, 10);

  if ((intmax_t)num < 0) {
    fatal(FN, "Expected value >= 0, %s(%jd) was passed.\n", num_char, (intmax_t)num);
  }

  if ( *p == num_char || strlen(*p) > suff_len ) {
    const char *suffix_msg = suff_len ? " possibly followed by a size suffix" : "";
    fatal(FN, "Invalid value passed: '%s', expected integer value%s.\n", num_char, suffix_msg);
  }

  switch (**p) {
    case '\0':
      break;
    case 'b':
    case 'B':
      break;
    case 'k':
    case 'K':
      num *= 1024;
      break;
    case 'm':
    case 'M':
      num *= 1024*1024;
      break;
    case 'g':
    case 'G':
      num *= 1024*1024*1024;
      break;
    default:
      fatal(FN, "Invalid value passed '%s', are you sure you used a valid size suffix?\n", num_char);
      break;
  }

  if (num > SIZE_MAX) {
    fatal(FN, "Value: %s(%ju) out of size_t range 0-%zu.\n", num_char, num, SIZE_MAX);
  }

  return (size_t)num;
}

off_t parse_num_o(const char *num_char, size_t suff_len) {
  intmax_t num;
  char *p[1]; /* **p must be initialized and not NULL */

  SALDL_ASSERT(num_char);
  num = strtoimax(num_char, p, 10);

  if (num < 0) {
    fatal(FN, "Expected value >= 0, %s(%jd) was passed.\n", num_char, num);
  }

  if ( *p == num_char || strlen(*p) > suff_len ) {
    const char *suffix_msg = suff_len ? " possibly followed by a size suffix" : "";
    fatal(FN, "Invalid value passed: '%s', expected integer value%s.\n", num_char, suffix_msg);
  }

  switch (**p) {
    case '\0':
      break;
    case 'b':
    case 'B':
      break;
    case 'k':
    case 'K':
      num *= 1024;
      break;
    case 'm':
    case 'M':
      num *= 1024*1024;
      break;
    case 'g':
    case 'G':
      num *= 1024*1024*1024;
      break;
    default:
      fatal(FN, "Invalid value passed '%s', are you sure you used a valid size suffix?\n", num_char);
      break;
  }

  if (num > OFF_T_MAX) {
    fatal(FN, "Value: %s(%jd) out of positive off_t range 0-%jd.\n", num_char, num, (intmax_t)OFF_T_MAX);
  }

  return (off_t)num;
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
