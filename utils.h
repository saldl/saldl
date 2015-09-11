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

#ifndef _SALDL_UTILS_H
#define _SALDL_UTILS_H
#else
#error redefining _SALDL_UTILS_H
#endif

#include <string.h>
#include <errno.h>

/*#include "vt100.h"*/
#include "common.h"
#include "merge.h"

char* saldl_user_agent();
void chunks_init(info_s*);
void curl_set_ranges(CURL*, chunk_s*);
void* saldl_calloc(size_t nmemb, size_t size);
void* saldl_realloc(void *ptr, size_t size);
long fsize(FILE*);
off_t fsizeo(FILE*);
off_t fsize2(char*);
void check_remote_file_size(info_s*);
void remote_info(info_s*);
void set_info(info_s*);
void print_chunk_info(info_s*);
void check_url(char*);
void set_names(CURL*, info_s*);
void global_progress_init(info_s*);
void global_progress_update(info_s *info_ptr, bool init);
void set_params(thread_s*, saldl_params*);
void set_progress_params(thread_s*, info_s*);
void set_write_opts(CURL*, void*, int);
void set_single_mode(info_s*);
void set_modes(info_s*);
void set_reset_storage(info_s*);
void prepare_storage_single(chunk_s*, file_s*);
void prepare_storage_tmpf(chunk_s*, file_s*);
void prepare_storage_mem(chunk_s*);
void reset_storage_single(thread_s*);
void reset_storage_tmpf(thread_s*);
void reset_storage_mem(thread_s*);
void saldl_perform(thread_s*);
int merge_finished_single();
int merge_finished_tmpf(info_s*, chunk_s*);
int merge_finished_mem(info_s*, chunk_s*);
void* thread_func(void*);
void curl_cleanup(info_s*);

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
