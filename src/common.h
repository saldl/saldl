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

#ifndef _SALDL_COMMON_H
#define _SALDL_COMMON_H
#else
#error redefining _SALDL_COMMON_H
#endif

#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h> /* basename() */
#include <inttypes.h> /* strtoimax(), strtoumax() */
#include <signal.h>
#include <sys/stat.h> /* mkdir(), stat() */

#include "log.h"
#include "progress.h"

#ifndef OFF_T_MAX
#define OFF_T_MAX (off_t)( sizeof(off_t) < sizeof(int64_t) ? INT32_MAX : INT64_MAX )
#endif
#define PCT(x1, x2) ((x1))*100.0/((x2))

#ifdef HAVE_GETMODULEFILENAME
char* windows_exe_path();
#endif

char* saldl_lstrip(char *str);
void* saldl_calloc(size_t nmemb, size_t size);
void* saldl_malloc(size_t size);
void* saldl_realloc(void *ptr, size_t size);
long fsize(FILE *f);
off_t fsizeo(FILE *f);
off_t fsize2(char *fname);
int saldl_mkdir(const char *path, mode_t mode);

void saldl_block_sig_pth();
void saldl_unblock_sig_pth();

#ifdef HAVE_SIGACTION
void ignore_sig(int sig, struct sigaction *sa_save);
void restore_sig_handler(int sig, struct sigaction *sa_restore);
#endif

void fputs_count(uintmax_t count, const char* str, FILE* stream);
double human_size(double size);
const char* human_size_suffix(double size);
size_t s_num_digits(intmax_t num);
size_t u_num_digits(uintmax_t num);
size_t saldl_min(size_t a, size_t b);
size_t saldl_max(size_t a, size_t b);
off_t saldl_max_o(off_t a, off_t b);
size_t saldl_max_z_umax(uintmax_t a, uintmax_t b);
char* valid_filename(const char *pre_valid);
char* trunc_filename(const char *pre_trunc, int keep_ext);
off_t parse_num_o(const char *num_char, size_t suff_len);
size_t parse_num_z(const char *num_char, size_t suff_len);

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
