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

#if !defined(__CYGWIN__) && !defined(__MSYS__) && (defined(HAVE_GETMODULEFILENAME) || defined(HAVE__MKDIR))
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
#define SUFFIX_LEN 9

void curl_set_ranges(CURL *handle, chunk_s *chunk) {
  char range_str[2 * s_num_digits(OFF_T_MAX) + 1];
  SALDL_ASSERT(chunk->range_end);
  SALDL_ASSERT( (uintmax_t)(chunk->range_end - chunk->range_start) <= (uintmax_t)SIZE_MAX );
  chunk->curr_range_start = chunk->range_start + (off_t)chunk->size_complete;
  saldl_snprintf(false, range_str, 2 * s_num_digits(OFF_T_MAX) + 1, "%"SAL_JD"-%"SAL_JD"", (intmax_t)chunk->curr_range_start, (intmax_t)chunk->range_end);
  curl_easy_setopt(handle, CURLOPT_RANGE, range_str);
}

#if !defined(__CYGWIN__) && !defined(__MSYS__) && defined(HAVE_GETMODULEFILENAME)
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

int saldl_strcmp(const char *s1, const char *s2) {
  if (s1 && s2) {
    return strcmp(s1, s2);
  }
  else if (s1 || s2) {
    fatal(FN, "One of the arguments is NULL");
  }
  else {
    /* Return 0 if both are NULL */
    return 0;
  }
}

int saldl_strcasecmp(const char *s1, const char *s2) {
  if (s1 && s2) {
    return strcasecmp(s1, s2);
  }
  else if (s1 || s2) {
    fatal(FN, "One of the arguments is NULL");
  }
  else {
    /* Return 0 if both are NULL */
    return 0;
  }
}

static size_t saldl_custom_headers_count(char **headers) {
  size_t count = 0;
  SALDL_ASSERT(headers);

  while (headers[count]) {
    count++;
  }
  return count;
}

void saldl_custom_headers_free_all(char **headers) {
  size_t idx = 0;

  if (!headers) {
    return;
  }

  while (headers[idx]) {
    SALDL_FREE(headers[idx]);
    idx++;
  }

  /* Free the array itself */
  SALDL_FREE(headers);
}

char** saldl_custom_headers_append(char **headers, char *headers_to_append) {
  if (!headers) {
    headers = saldl_calloc(1, sizeof(char*));
  }

  SALDL_ASSERT(headers_to_append);
  char *curr, *next, *tmp;
  next = headers_to_append;

  if ( (tmp = strrchr(headers_to_append, '\r')) ) {
    if (tmp[1] == '\n' && tmp[2] == '\0') {
      fatal(FN, "Only or last header must not end with \\r\\n");
    }
  }

  while (*next != '\0') {
    curr = next;
    if ( (tmp = strstr(curr, "\r\n")) ) {
      next = tmp + strlen("\r\n");
      *tmp = '\0';
      debug_msg(FN, "Stripped \\r\\n for you at the end of %s", curr);
    }
    else {
      /* make *next == '\0' */
      next = curr + strlen(curr);
      SALDL_ASSERT(*next == '\0');
    }

    /* Check curr format before appending */
    if (!strstr(curr, ":") && !strstr(curr, ";")) {
      pre_fatal(FN, "%s is not a valid header format.", curr);
      pre_fatal(FN, "There are 3 valid formats for custom headers:");
      pre_fatal(FN, " 1- Header with info:");
      pre_fatal(FN, "  header_name: info");
      pre_fatal(FN, " 2- Header without info:");
      pre_fatal(FN, "  header_name;");
      pre_fatal(FN, " 3- Suppress non-custom header:");
      pre_fatal(FN, "  header_name:");
      pre_fatal(FN, "Multiple headers should be separated by \\r\\n");
      fatal    (FN, "The only or the last header must not end with \\r\\n");
    }

    /* if curr is valid, append it to headers */
    size_t curr_headers_size = saldl_custom_headers_count(headers) + 1; // +1 for NULL termination
    SALDL_ASSERT(headers);
    SALDL_ASSERT(curr_headers_size >= 1);
    headers = saldl_realloc(headers, (curr_headers_size+1) * sizeof(char*));

    debug_msg(FN, "Appending custom header: '%s'", curr);
    headers[curr_headers_size - 1] = saldl_strdup(curr);
    headers[curr_headers_size] = NULL;
  }


  return headers;
}

void saldl_fflush(const char *label, FILE *f) {
  int ret;
  SALDL_ASSERT(label);
  SALDL_ASSERT(f);

  ret = fflush(f);

  if (ret) {
    fatal(FN, "Flushing '%s' failed: %s", label, strerror(errno));
  }
}

void saldl_fwrite_fflush(const void *read_buf, size_t size, size_t nmemb, FILE *out_file, const char *out_name, off_t offset_info) {
  size_t ret;
  size_t write_size;

  SALDL_ASSERT(read_buf);
  SALDL_ASSERT(size);
  SALDL_ASSERT(nmemb);
  SALDL_ASSERT(size * nmemb <= SIZE_MAX);
  SALDL_ASSERT(out_file);

  saldl_fflush(out_name, out_file);

  ret = fwrite(read_buf, size, nmemb, out_file);
  write_size = size * nmemb;

  if (ret != nmemb) {
    if (offset_info) {
      fatal(FN, "Writing %"SAL_ZU" bytes to '%s' failed at offset %"SAL_JD": %s",
          write_size, out_name, offset_info, strerror(errno));
    }
    else {
      fatal(FN, "Writing %"SAL_ZU" bytes to '%s' failed: %s",
          write_size, out_name, strerror(errno));
    }
  }

  saldl_fflush(out_name, out_file);
}

void saldl_fclose(const char *label, FILE *f) {
  int ret;
  SALDL_ASSERT(label);
  SALDL_ASSERT(f);

  ret = fclose(f);

  if (ret) {
    fatal(FN, "Closing '%s' failed: %s", label, strerror(errno));
  }
}

void saldl_fseeko(const char *label, FILE *f, off_t offset, int whence) {
  int ret;
  SALDL_ASSERT(label);
  SALDL_ASSERT(f);

  ret = fseeko(f, offset, whence);

  if (ret) {
    fatal(FN, "Seeking in '%s' failed: %s", label, strerror(errno));
  }
}

off_t saldl_ftello(const char *label, FILE *f) {
  off_t ret;

  SALDL_ASSERT(label);
  SALDL_ASSERT(f);

  ret = ftello(f);

  if (ret == -1) {
    fatal(FN, "ftello() in '%s' failed: %s", label, strerror(errno));
  }

  return ret;
}

off_t saldl_fsizeo(const char *label, FILE *f) {
  off_t curr;
  off_t size;

  SALDL_ASSERT(label);
  SALDL_ASSERT(f);

  curr = saldl_ftello(label, f);
  saldl_fseeko(label, f, 0, SEEK_END);
  size = saldl_ftello(label, f);
  saldl_fseeko(label, f, curr, SEEK_SET);
  return size;
}

off_t saldl_fsize_sys(char *file_path) {
  int ret;
  struct stat st;
  SALDL_ASSERT(file_path);
  ret = stat(file_path, &st);
  return ret ? ret : st.st_size;
}

time_t saldl_file_mtime(char *file_path) {
  int ret;
  struct stat st;
  SALDL_ASSERT(file_path);
  ret = stat(file_path, &st);
  return ret ? (time_t)ret : st.st_mtime;
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
    pre_fatal(FN, "Failed with error: %s.", strerror(ret));
    fatal(FN, "Please check your privileges and system limits.");
  }
}

void saldl_pthread_join_accept_einval(pthread_t thread, void **retval) {
  int ret;

  while (1) {
    ret = pthread_join(thread, retval);

    switch (ret) {
      case 0:
        return;
      case EINVAL:
        /* If download completed, and saldl was interrupted, it's
         * possible that exit_routine() would be called while joining
         * a thread. Re-joining that thread would fail with EINVAL.
         * So we consider this non-fatal. And only issue a warning. */
        warn_msg(FN, "EINVAL returned, pthread already joined!");
        return;
      default:
        fatal(FN, "Failed: %s.", strerror(ret));
    }
  }

}

void saldl_pthread_mutex_lock_retry_deadlock(pthread_mutex_t *mutex) {
  int ret;
  SALDL_ASSERT(mutex);

  while(1) {

    /* Note: PTHREAD_MUTEX_ERRORCHECK attr is set */
    ret = pthread_mutex_lock(mutex);

    switch (ret) {
      case 0:
        return;
      case EDEADLK:
        continue;
      default:
        fatal(FN, "Acquiring mutex failed, %s.", strerror(ret));
    }

  }

}

void saldl_pthread_mutex_unlock(pthread_mutex_t *mutex) {
  int ret;
  SALDL_ASSERT(mutex);

  /* Note: PTHREAD_MUTEX_ERRORCHECK attr is set */
  ret = pthread_mutex_unlock(mutex);

  if (ret) {
    fatal(FN, "Failed: %s.", strerror(ret));
  }
}

#ifdef HAVE_SIGACTION
void saldl_sigaction(int sig, const struct sigaction *restrict act, struct sigaction *restrict oact) {
  int ret;
  SALDL_ASSERT(act || oact);

  ret = sigaction(sig, act, oact);

  if (ret) {
    fatal(FN, "Failed: %s", strerror(errno));
  }
}

static void saldl_pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset) {
  int ret;
  SALDL_ASSERT(set || oldset);

  ret = pthread_sigmask(how, set, oldset);

  if (ret) {
    fatal(FN, "Failed to change the mask of blocked signals: %s", strerror(errno));
  }
}
#else
void saldl_win_signal(int signum, void (*handler)(int)) {
  void (*ret)(int);
  SALDL_ASSERT(handler);

  ret = signal(signum, handler);

  if (ret == SIG_ERR) {
    fatal(FN, "Failed to set signal handler: %s", strerror(errno));
  }
}
#endif

#ifdef HAVE_SIGADDSET
void saldl_sigaddset(sigset_t *set, int signum) {
  int ret;
  SALDL_ASSERT(set);

  ret = sigaddset(set, signum);

  if (ret) {
    fatal(FN, "Failed to add to signal set: %s", strerror(errno));
  }
}

void saldl_sigemptyset(sigset_t *set) {
  int ret;
  SALDL_ASSERT(set);

  ret = sigemptyset(set);

  if (ret) {
    fatal(FN, "Failed to empty signal set: %s", strerror(errno));
  }
}
#endif

void saldl_block_sig_pth() {
#ifdef HAVE_SIGADDSET
  /* Don't interrupt thread, Let the signal handler work properly */
  sigset_t set;
  saldl_sigemptyset(&set);
  saldl_sigaddset(&set, SIGINT);
  saldl_sigaddset(&set, SIGTERM);
  saldl_pthread_sigmask(SIG_SETMASK, &set, NULL);
#endif
}

void saldl_unblock_sig_pth() {
#ifdef HAVE_SIGADDSET
  /* Unblock any blocked signals, */
  sigset_t set;
  saldl_sigemptyset(&set);
  saldl_pthread_sigmask(SIG_SETMASK, &set, NULL);

#endif
}

#ifdef HAVE_SIGACTION
void ignore_sig(int sig, struct sigaction *sa_save) {
  struct sigaction sa_ign;
  saldl_sigaction(sig, NULL, sa_save);
  sa_ign = *sa_save;
  sa_ign.sa_handler = SIG_IGN;
  saldl_sigaction(sig, &sa_ign, NULL);
}

void restore_sig_handler(int sig, struct sigaction *sa_restore) {
  saldl_sigaction(sig, sa_restore, NULL);
}
#endif

void saldl_snprintf(bool allow_truncation, char *str, size_t size, const char *format, ...) {
  if (format) {
    va_list args;
    va_start(args, format);

    int ret = vsnprintf(str, size, format, args);

    if (size && ret <= 0) {
      fatal(FN, "snprintf() returned error.");
    }

    if (size && !allow_truncation &&  (intmax_t)ret >= (intmax_t)size) {
      fatal(FN, "\"%s\" is truncated.", str);
    }

    va_end(args);
  }
}

void saldl_fputc(int c, FILE *stream, const char *label) {
  int ret;

  SALDL_ASSERT(c >= 0);
  SALDL_ASSERT(c <= CHAR_MAX);
  SALDL_ASSERT(stream);
  SALDL_ASSERT(label);

  ret = fputc(c, stream);

  if (ret == EOF) {
    fatal(FN, "Writing to '%s' failed.", label);
  }
}

void saldl_fputs(const char *str, FILE *stream, const char *label) {
  int ret;

  SALDL_ASSERT(str);
  SALDL_ASSERT(stream);
  SALDL_ASSERT(label);

  ret = fputs(str, stream);
  if (ret == EOF) {
    fatal(FN, "Writing to '%s' failed.", label);
  }
}

void saldl_fputs_count(uintmax_t count, const char* str, FILE* stream, const char *label) {
  SALDL_ASSERT(str);
  SALDL_ASSERT(stream);
  SALDL_ASSERT(label);

  for (uintmax_t loops = 1; loops <= count; loops++) {
    saldl_fputs(str, stream, label);
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
  int len = snprintf(NULL, 0, "%"SAL_JD"", num);
  if (len < 0) fatal(FN, "Failed to get len of %"SAL_JD"", num);
  return len;
}

size_t u_num_digits(uintmax_t num) {
  int len = snprintf(NULL, 0, "%"SAL_JU"", num);
  if (len < 0) fatal(FN, "Failed to get len of %"SAL_JU"", num);
  return len;
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
    fatal(FN, "Max value %"SAL_JD" is out of size_t range 0-%"SAL_ZU".", (intmax_t)ret, SIZE_MAX);
  }
  return (size_t)ret;
}

char* saldl_getcwd(char *buf, size_t size) {
  char *ret = NULL;
  SALDL_ASSERT(buf);
  SALDL_ASSERT(size);

  ret = getcwd(buf, size);
  if (!ret) {
    fatal(FN, "Failed getting current directory path: %s", strerror(errno));
  }

  return  ret;
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

    // Make sure dirname is not too long even if the shortest possible basename is used
    if (strlen(tmp_dirname) >= PATH_MAX - strlen("/0.part.sal"))  {
      fatal(FN, "dirname '%s' is too long.", tmp_dirname);
    }

    saldl_snprintf(false, dir_name,PATH_MAX,"%s/", tmp_dirname);
  }
  SALDL_FREE(pre_trunc_copy);

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

  if (ext_len >= NAME_MAX - SUFFIX_LEN) {
    fatal(FN, "Extension '%s' is too long.", ext_str);
  }
  saldl_snprintf(true, base_name,NAME_MAX-SUFFIX_LEN-ext_len,"%s", tmp_basename);
  SALDL_FREE(pre_trunc_copy);

  saldl_snprintf(false, trunc_name,
      PATH_MAX - SUFFIX_LEN - u_num_digits(SIZE_MAX) - (dir_name[0]=='/'?0:strlen(getcwd(cwd,PATH_MAX))+1),
      "%s%s%s",
      dir_name,
      base_name,
      ext_str);
  SALDL_FREE(base_name);
  SALDL_FREE(ext_str_empty);
  return trunc_name;
}

size_t parse_num_d(const char *num_char) {
  double num;
  char *p[1]; /* **p must be initialized and not NULL */

  SALDL_ASSERT(num_char);
  num = strtod(num_char, p);

  if (strlen(*p)) {
    fatal(FN, "Invalid value passed: '%s', expected a floating number.", num_char);
  }

  if (num <= 0) {
    fatal(FN, "Expected a positive value, %s(%lf) was passed.", num_char, num);
  }

  if (num == HUGE_VAL) {
    fatal(FN, "Value: %s parsed as %lf is considered huge.", num_char, num);
  }

  return num;
}

size_t parse_num_z(const char *num_char, size_t suff_len) {
  uintmax_t num;
  char *p[1]; /* **p must be initialized and not NULL */

  SALDL_ASSERT(num_char);
  num = strtoumax(num_char, p, 10);

  if ((intmax_t)num < 0) {
    fatal(FN, "Expected value >= 0, %s(%"SAL_JD") was passed.", num_char, (intmax_t)num);
  }

  if ( *p == num_char || strlen(*p) > suff_len ) {
    const char *suffix_msg = suff_len ? " possibly followed by a size suffix" : "";
    fatal(FN, "Invalid value passed: '%s', expected integer value%s.", num_char, suffix_msg);
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
      fatal(FN, "Invalid value passed '%s', are you sure you used a valid size suffix?", num_char);
      break;
  }

  if (num > SIZE_MAX) {
    fatal(FN, "Value: %s(%"SAL_JU") out of size_t range 0-%"SAL_ZU".", num_char, num, SIZE_MAX);
  }

  return (size_t)num;
}

off_t parse_num_o(const char *num_char, size_t suff_len) {
  intmax_t num;
  char *p[1]; /* **p must be initialized and not NULL */

  SALDL_ASSERT(num_char);
  num = strtoimax(num_char, p, 10);

  if (num < 0) {
    fatal(FN, "Expected value >= 0, %s(%"SAL_JD") was passed.", num_char, num);
  }

  if ( *p == num_char || strlen(*p) > suff_len ) {
    const char *suffix_msg = suff_len ? " possibly followed by a size suffix" : "";
    fatal(FN, "Invalid value passed: '%s', expected integer value%s.", num_char, suffix_msg);
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
      fatal(FN, "Invalid value passed '%s', are you sure you used a valid size suffix?", num_char);
      break;
  }

  if (num > OFF_T_MAX) {
    fatal(FN, "Value: %s(%"SAL_JD") out of positive off_t range 0-%"SAL_JD".", num_char, num, (intmax_t)OFF_T_MAX);
  }

  return (off_t)num;
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
