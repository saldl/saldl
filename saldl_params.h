#ifndef _SALDL_SALDL_PARAMS_H
#define _SALDL_SALDL_PARAMS_H
#else
#error redefining _SALDL_SALDL_PARAMS_H
#endif

/* Defaults */
#define SALDL_DEF_CHUNK_SIZE (size_t)1*1024*1024 /* 1.00 MiB */
#define SALDL_DEF_NUM_CONNECTIONS (size_t)6

typedef struct {
  bool dry_run;
  size_t no_color;
  size_t verbosity;
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
