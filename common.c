#ifdef __MINGW32__
#include <winsock2.h>
#include <windows.h>
#define NAME_MAX MAX_PATH
#endif

#include "common.h"

/* .part.sal , .ctrl.sal len is 9 */
#define EXT_LEN 9

void saldl_block_sig_pth() {
#ifdef HAVE_SIGADDSET
  /* Don't interrupt thread, Let the signal handler work properly */
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTERM);
  int ret_sigmask = pthread_sigmask(SIG_SETMASK, &set, NULL);
  assert(!ret_sigmask);
#endif
}

void saldl_unblock_sig_pth() {
#ifdef HAVE_SIGADDSET
  /* Unblock any blocked signals, */
  sigset_t set;
  sigemptyset(&set);
  int ret_sigmask = pthread_sigmask(SIG_SETMASK, &set, NULL);
  assert(!ret_sigmask);
#endif
}

void ignore_sig(int sig, struct sigaction *sa_save) {
#ifdef HAVE_SIGACTION
  struct sigaction sa_ign;
  sigaction(sig, NULL, sa_save);
  sa_ign = *sa_save;
  sa_ign.sa_handler = SIG_IGN;
  sigaction(sig, &sa_ign, NULL);
#endif
}

void restore_sig_handler(int sig, struct sigaction *sa_restore) {
#ifdef HAVE_SIGACTION
  sigaction(sig, sa_restore, NULL);
#endif
}

void fputs_count(uintmax_t count, const char* str, FILE* stream) {
  for (uintmax_t loops = 1; loops <= count; loops++) {
    fputs(str, stream);
  }
}

double human_size(double size) {
  if (size > 1000*1000*1000) {
    return GiB(size);
  } else if (size > 1000*1000) {
    return MiB(size);
  } else if (size > 1000) {
    return KiB(size);
  } else {
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
  char *corrected_name = strdup(pre_valid);
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

  if (!pre_trunc) return NULL;

  trunc_name = strdup(pre_trunc); /* allocates enough bits */

  pre_trunc_copy = strdup(pre_trunc);
  tmp_dirname = dirname(pre_trunc_copy);
  if ( tmp_dirname[0] != '.' ) {
    snprintf(dir_name,PATH_MAX,"%s/", tmp_dirname);
  }
  free(pre_trunc_copy);

  ext_str = ext_str_empty = strdup("");
  if (keep_ext) {
    ext_str = strrchr(pre_trunc, '.');
    if (!ext_str) ext_str = ext_str_empty;
    ext_len = strlen(ext_str);
  }

  pre_trunc_copy = strdup(pre_trunc);
  tmp_basename = basename(pre_trunc_copy);
  tmp_basename[ strlen(tmp_basename) - ext_len ] = '\0';
  base_name = strdup(tmp_basename);
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

size_t parse_num_z(const char *num_char, size_t suff_len) {
  uintmax_t num;
  char *p[1]; /* **p must be initialized and not NULL */
  num = strtoumax(num_char, p, 10);

  if ((intmax_t)num < 0) {
    fatal(FN, "Expected value >= 0, %s(%jd) was passed.\n", num_char, (intmax_t)num);
  }

  if ( *p == num_char || strlen(*p) > suff_len ) {
    err_msg(FN, "Invalid value was found while parsing opts/control:\n");
    err_msg(FN, " Valid format is <num>");
    if (suff_len) {
      fprintf(stderr, " optionally followed by one of the chars from the group [bBkKmMgG]");
    }
    fprintf(stderr, "\n");
    fatal(FN, "  Value passed: %s.\n", num_char);
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
      err_msg(FN, "Invalid value was found while parsing opts/control:\n");
      err_msg(FN, " Valid format is <num>");
      if (suff_len) {
        fprintf(stderr, " optionally followed by one of the chars from the group [bBkKmMgG]");
      }
      fprintf(stderr, "\n");
      fatal(FN, " Value passed: %s.\n", num_char);
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
  num = strtoimax(num_char, p, 10);

  if (num < 0) {
    fatal(FN, "Expected value >= 0, %s(%jd) was passed.\n", num_char, num);
  }

  if ( *p == num_char || strlen(*p) > suff_len ) {
    err_msg(FN, "Invalid value was found while parsing opts/control:\n");
    err_msg(FN, " Valid format is <num>");
    if (suff_len) {
      fprintf(stderr, " optionally followed by one of the chars from the group [bBkKmMgG]");
    }
    fprintf(stderr, "\n");
    fatal(FN, "  Value passed: %s.\n", num_char);
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
      err_msg(FN, "Invalid value was found while parsing opts/control:\n");
      err_msg(FN, " Valid format is <num>");
      if (suff_len) {
        fprintf(stderr, " optionally followed by one of the chars from the group [bBkKmMgG]");
      }
      fprintf(stderr, "\n");
      fatal(FN, " Value passed: %s.\n", num_char);
      break;
  }

  if (num > OFF_T_MAX) {
    fatal(FN, "Value: %s(%jd) out of positive off_t range 0-%jd.\n", num_char, num, (intmax_t)OFF_T_MAX);
  }

  return (off_t)num;
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
