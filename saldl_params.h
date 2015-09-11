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

#ifndef _SALDL_SALDL_PARAMS_H
#define _SALDL_SALDL_PARAMS_H
#else
#error redefining _SALDL_SALDL_PARAMS_H
#endif

#include <stdbool.h>

/* Defaults */
#define SALDL_DEF_CHUNK_SIZE (size_t)1*1024*1024 /* 1.00 MiB */
#define SALDL_DEF_NUM_CONNECTIONS (size_t)6

typedef struct {
  bool print_version;
  bool dry_run;
  size_t no_color;
  size_t verbosity;
  bool libcurl_verbosity;
  bool no_status;
  char* url;
  char* root_dir;
  char* filename;
  char* attachment_filename;
  size_t user_chunk_size;
  size_t last_chunks_first;
  size_t user_num_connections;
  size_t connection_max_rate;
  bool auto_referer;
  char *referer;
  char *cookie_file;
  char *inline_cookies;
  char *post;
  char *raw_post;
  bool no_user_agent;
  char *user_agent;
  char *proxy;
  char *tunnel_proxy;
  bool no_proxy;
  bool tls_no_verify;
  bool no_timeouts;
  bool head;
  bool no_remote_info;
  bool no_attachment_detection;
  bool skip_partial_check;
  bool single_mode;
  bool no_path;
  bool keep_GET_attrs;
  bool auto_trunc;
  bool smart_trunc;
  bool resume;
  bool force;
  int auto_size;
  bool whole_file;
  bool mem_bufs;
} saldl_params;

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
