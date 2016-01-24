/*
    This file is a part of saldl.

    Copyright (C) 2014-2016 Mohammad AlSaleh <CE.Mohammad.AlSaleh at gmail.com>
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

#ifndef SALDL_PARAMS_H
#define SALDL_PARAMS_H
#else
#error redefining SALDL_PARAMS_H
#endif

#include <stdbool.h>

typedef struct {
  bool print_version;
  bool dry_run;
  size_t no_color;
  size_t verbosity;
  bool libcurl_verbosity;
  double status_refresh_interval;
  bool no_status;
  char* start_url;
  char* mirror_start_url;
  bool fatal_if_invalid_mirror;
  char* root_dir;
  char* filename;
  size_t chunk_size;
  size_t last_chunks_first;
  off_t last_size_first;
  size_t num_connections;
  size_t connection_max_rate;
  bool auto_referer;
  char *referer;
  char *date_expr;
  char *since_file_mtime;
  char *cookie_file;
  char *inline_cookies;
  char *post;
  char *raw_post;
  char **custom_headers; /* NULL-terminated */
  char **proxy_custom_headers; /* NULL-terminated */
  uint8_t forced_ip_protocol; /* 4 or 6 */
  bool no_http2;
  bool http2_upgrade;
  bool compress;
  bool no_decompress;
  bool no_user_agent;
  char *user_agent;
  char *proxy;
  char *tunnel_proxy;
  bool no_proxy;
  bool tls_no_verify;
  bool no_timeouts;
  bool no_tcp_keep_alive;
  bool head;
  bool no_remote_info;
  bool no_attachment_detection;
  bool assume_range_support;
  bool single_mode;
  bool no_path;
  bool keep_GET_attrs;
  bool filename_from_redirect;
  bool auto_trunc;
  bool smart_trunc;
  bool resume;
  bool force;
  int auto_size;
  bool whole_file;
  bool no_mmap;
  bool mem_bufs;
  bool read_only;
  bool to_stdout;
  bool merge_in_order;
} saldl_params;

/* Static initializer.
 * This is unnecessary, but avoids some compilers' warnings.
 * Clang and old GCC versions found in the BSDs just don't get
 * that {0} initialization is perfectly valid (and complete in a sense).
 */
static const saldl_params DEF_SALDL_PARAMS;

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
