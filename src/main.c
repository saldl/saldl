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

#include <getopt.h>

#include "saldl.h"

#if !CURL_AT_LEAST_VERSION(7, 42, 0)
#error "libcurl >= 7.42.0 required."
#endif

static void print_libcurl_protocols(const char * const *protocols) {
  intptr_t index = 0;
  SALDL_ASSERT(protocols);

  fprintf(stderr, "Protocols:");

  while (protocols[index]) {
    fprintf(stderr, " %s", protocols[index]);
    index++;
  }

  fprintf(stderr, ".\n");
}

static int saldl_version() {
  curl_version_info_data *curl_info = curl_version_info(CURLVERSION_NOW);
  fprintf(stderr, "%s %s (%s)\n", SALDL_NAME, SALDL_VERSION, SALDL_WWW);
#ifdef _WIN32
  fprintf(stderr, "WARNING: Windows builds of %s are experimental.\n", SALDL_NAME);
#endif
  fprintf(stderr, "\n");
  fprintf(stderr, "Copyright (C) 2014-2016 Mohammad AlSaleh.\n");
  fprintf(stderr, "Free use of this software is granted under the terms of\n");
  fprintf(stderr, "the GNU Affero General Public License (AGPL).\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Built against: libcurl %s\n", LIBCURL_VERSION);
  fprintf(stderr, "Loaded: libcurl %s (%s)\n", curl_info->version, curl_info->ssl_version);
  print_libcurl_protocols(curl_info->protocols);
  return 0;
}

static int usage(char *caller) {
  saldl_version();
  fprintf(stderr, "\n");
  fprintf(stderr, "Usage: %s [OPTIONS] URL\n", caller);
  fprintf(stderr, "Detailed documentation is available in the manual.\n");
  fprintf(stderr, "An online version of the manual is available at:\n");
  fprintf(stderr, "https://saldl.github.io/saldl.1.html\n");
  return EXIT_FAILURE;
}

int set_get_info(saldl_params *params, char *get_info) {
  if (!strcmp(get_info, "file-name")) {
    params->get_file_name = true;
  }
  else if (!strcmp(get_info, "file-size")) {
    params->get_file_size = true;
  }
  else if (!strcmp(get_info, "effective-url")) {
    params->get_effective_url = true;
  }
  else {
    fprintf(stderr, "Only one of the following arguments can be passed to --get-info: \"file-name\", \"file-size\", or \"effective-url\".\n\n");
    return  1;
  }
  return 0;
}

static int parse_opts(saldl_params *params_ptr, int full_argc, char **full_argv) {
  int opt_idx = 0, c = 0;
  static struct option long_opts[] = {
    {"version" , no_argument, 0, 'v'},
    {"verbosity" , no_argument, 0, 'V'},
    {"no-color" , no_argument, 0, 'C'},
    {"status-refresh-interval", required_argument, 0, 'i'},
    {"chunk-size" , required_argument, 0, 's'},
    {"last-chunks-first" , required_argument, 0, 'l'},
    {"last-size-first" , required_argument, 0, 'L'},
    {"connections", required_argument, 0, 'c'},
    {"connection-max-rate", required_argument, 0, 'R'},
    {"resolve-ipv6", no_argument, 0, '6'},
    {"resolve-ipv4", no_argument, 0, '4'},
    {"user-agent", required_argument, 0, 'u'},
    {"no-user-agent", no_argument, 0, 'U'},
    {"compress", no_argument, 0, 'z'},
    {"no-decompress", no_argument, 0, 'Z'},
    {"since-file-mtime", required_argument, 0, 'M'},
    {"date-cond", required_argument, 0, 'Y'},
    {"post", required_argument, 0, 'p'},
    {"raw-post", required_argument, 0, 'P'},
    {"inline-cookies", required_argument, 0, 'k'},
    {"cookie-file", required_argument, 0, 'K'},
    {"referer", required_argument, 0, 'e'},
    {"auto-referer", no_argument, 0, 'E'},
    {"proxy", required_argument, 0, 'x'},
    {"tunnel-proxy", required_argument, 0, 'X'},
    {"no-proxy", no_argument, 0, 'N'},
    {"no-timeouts", no_argument, 0, 'O'},
    {"no-remote-info", no_argument, 0, 'I'},
    {"no-attachment-detection", no_argument, 0, 'A'},
    {"custom-headers", required_argument, 0, 'H'},
    {"single", no_argument, 0, 'S'},
    {"no-path", no_argument, 0, 'n'},
    {"keep-GET-attrs", no_argument, 0, 'G'},
    {"filename-from-redirect", no_argument, 0, 'g'},
    {"dry-run", no_argument, 0, 'd'},
    {"root-dir", required_argument, 0, 'D'},
    {"output-filename", required_argument, 0, 'o'},
    {"auto-trunc", no_argument, 0, 't'},
    {"smart-trunc", no_argument, 0, 'T'},
    {"resume", no_argument, 0, 'r'},
    {"force", no_argument, 0, 'f'},
    {"auto-size", required_argument, 0, 'a'},
    {"whole-file", no_argument, 0, 'w'},
    {"memory-buffers", no_argument, 0, 'm'},
    {"allow-ftp-segments", no_argument, 0, 'F'},
    /* long only */
#define SAL_OPT_NO_STATUS               CHAR_MAX+1
#define SAL_OPT_NO_HTTP2                CHAR_MAX+2
#define SAL_OPT_ASSUME_RANGE_SUPPORT    CHAR_MAX+3
#define SAL_OPT_SKIP_TLS_VERIFICATION   CHAR_MAX+4
#define SAL_OPT_VERBOSE_LIBCURL         CHAR_MAX+5
#define SAL_OPT_READ_ONLY               CHAR_MAX+6
#define SAL_OPT_USE_HEAD                CHAR_MAX+7
#define SAL_OPT_PROXY_CUSTOM_HEADERS    CHAR_MAX+8
#define SAL_OPT_NO_MMAP                 CHAR_MAX+9
#define SAL_OPT_NO_TCP_KEEP_ALIVE       CHAR_MAX+10
#define SAL_OPT_STDOUT                  CHAR_MAX+11
#define SAL_OPT_HTTP2_UPGRADE           CHAR_MAX+12
#define SAL_OPT_MIRROR_URL              CHAR_MAX+13
#define SAL_OPT_FATAL_IF_INVALID_MIRROR CHAR_MAX+14
#define SAL_OPT_MERGE_IN_ORDER          CHAR_MAX+15
#define SAL_OPT_RANDOM_ORDER            CHAR_MAX+16
#define SAL_OPT_GET_INFO                CHAR_MAX+17
    {"mirror-url", required_argument, 0, SAL_OPT_MIRROR_URL},
    {"fatal-if-invalid-mirror", no_argument, 0, SAL_OPT_FATAL_IF_INVALID_MIRROR},
    {"no-http2", no_argument, 0, SAL_OPT_NO_HTTP2},
    {"http2-upgrade", no_argument, 0, SAL_OPT_HTTP2_UPGRADE},
    {"no-tcp-keep-alive", no_argument, 0, SAL_OPT_NO_TCP_KEEP_ALIVE},
    {"no-status", no_argument, 0, SAL_OPT_NO_STATUS},
    {"verbose-libcurl", no_argument, 0, SAL_OPT_VERBOSE_LIBCURL},
    {"skip-TLS-verification", no_argument, 0, SAL_OPT_SKIP_TLS_VERIFICATION},
    {"assume-range-support", no_argument, 0, SAL_OPT_ASSUME_RANGE_SUPPORT},
    {"no-mmap", no_argument, 0, SAL_OPT_NO_MMAP},
    {"stdout", no_argument, 0, SAL_OPT_STDOUT},
    {"merge-in-order", no_argument, 0, SAL_OPT_MERGE_IN_ORDER},
    {"random-order", no_argument, 0, SAL_OPT_RANDOM_ORDER},
    {"read-only", no_argument, 0, SAL_OPT_READ_ONLY},
    {"use-HEAD", no_argument, 0, SAL_OPT_USE_HEAD},
    {"proxy-custom-headers", required_argument, 0, SAL_OPT_PROXY_CUSTOM_HEADERS},
    {"get-info", required_argument, 0, SAL_OPT_GET_INFO},
    {0, 0, 0, 0}
  };

  const char *opts = "s:l:L:c:R:x:X:N3OSH:IAnGgdD:o:tTrfa:wmVvCi:K:k:M:Y:p:P:e:Eu:U64ZzF";
  opt_idx = 0 , optind = 0;
  while (1) {
    c = getopt_long(full_argc, full_argv, opts, long_opts, &opt_idx);
    if ( c == -1  ) break;
    switch (c) {
      case 'C':
        params_ptr->no_color++ ;
        break;
      default:
        break;
    }
  }
  set_color(&params_ptr->no_color);

  opt_idx = 0 , optind = 0;
  while (1) {
    c = getopt_long(full_argc, full_argv, opts, long_opts, &opt_idx);
    if ( c == -1  ) break;
    switch (c) {
      case 'V':
        params_ptr->verbosity++;
        break;
      default:
        break;
    }
  }
  set_verbosity(&params_ptr->verbosity, &params_ptr->libcurl_verbosity);

  opt_idx = 0 , optind = 0;
  while (1) {
    c = getopt_long(full_argc, full_argv, opts, long_opts, &opt_idx);
    if ( c == -1  ) break;
    switch (c) {
      case 'C':
      case 'V':
        break;
      case 'v':
        params_ptr->print_version = true;
        break;
      case 'i':
        params_ptr->status_refresh_interval = parse_num_d(optarg);
        break;
      case 's':
        params_ptr->chunk_size = parse_num_z(optarg, 1);
        break;
      case 'L':
        params_ptr->last_size_first = parse_num_o(optarg, 1);
        break;
      case 'l':
        params_ptr->last_chunks_first = parse_num_z(optarg, 0);
        break;
      case 'c':
        params_ptr->num_connections = parse_num_z(optarg, 0);
        break;
      case 'R':
        params_ptr->connection_max_rate = parse_num_z(optarg, 1);
        break;
      case 'u':
        params_ptr->user_agent = saldl_strdup(optarg);
        break;
      case 'U':
        params_ptr->no_user_agent = true;
        break;
      case 'z':
        params_ptr->compress= true;
        break;
      case 'Z':
        params_ptr->no_decompress= true;
        break;
      case 'M':
        params_ptr->since_file_mtime = saldl_strdup(optarg);
        break;
      case 'Y':
        params_ptr->date_expr = saldl_strdup(optarg);
        break;
      case 'p':
        params_ptr->post = saldl_strdup(optarg);
        break;
      case 'P':
        params_ptr->raw_post = saldl_strdup(optarg);
        break;
      case 'k':
        params_ptr->inline_cookies = saldl_strdup(optarg);
        break;
      case 'K':
        params_ptr->cookie_file = saldl_strdup(optarg);
        break;
      case '6':
        params_ptr->forced_ip_protocol = 6;
        break;
      case '4':
        params_ptr->forced_ip_protocol = 4;
        break;
      case 'e':
        params_ptr->referer = saldl_strdup(optarg);
        break;
      case 'E':
        params_ptr->auto_referer = true;
        break;
      case 'x':
        params_ptr->proxy = saldl_strdup(optarg);
        break;
      case 'X':
        params_ptr->tunnel_proxy = saldl_strdup(optarg);
        break;
      case 'N':
        params_ptr->no_proxy = true;
        break;
      case 'O':
        params_ptr->no_timeouts = true;
        break;
      case 'H':
        params_ptr->custom_headers = saldl_custom_headers_append(params_ptr->custom_headers, optarg);
        break;
      case 'I':
        params_ptr->no_remote_info = true;
        break;
      case 'A':
        params_ptr->no_attachment_detection = true;
        break;
      case 'S':
        params_ptr->single_mode = true;
        break;
      case 'n':
        params_ptr->no_path = true;
        break;
      case 'g':
        params_ptr->filename_from_redirect = true;
        break;
      case 'G':
        params_ptr->keep_GET_attrs = true;
        break;
      case 'd':
        params_ptr->dry_run = true;
        break;
      case 'D':
        params_ptr->root_dir = saldl_strdup(optarg);
        break;
      case 'o':
        params_ptr->filename= saldl_strdup(optarg);
        break;
      case 't':
        params_ptr->auto_trunc= true;
        break;
      case 'T':
        params_ptr->smart_trunc= true;
        break;
      case 'r':
        params_ptr->resume = true;
        break;
      case 'f':
        params_ptr->force = true;
        break;
      case 'a':
        params_ptr->auto_size= parse_num_z(optarg, 0);
        break;
      case 'w':
        params_ptr->whole_file = true;
        break;
      case 'm':
        params_ptr->mem_bufs = true;
        break;
      case 'F':
        params_ptr->allow_ftp_segments = true;
        break;

      /* long only */
      case SAL_OPT_GET_INFO:
        if (set_get_info(params_ptr, optarg)) {
          return EXIT_FAILURE;
        }
        break;

      case SAL_OPT_MIRROR_URL:
        params_ptr->mirror_start_url = saldl_strdup(optarg);
        break;

      case SAL_OPT_FATAL_IF_INVALID_MIRROR:
        params_ptr->fatal_if_invalid_mirror = true;
        break;

      case SAL_OPT_NO_HTTP2:
        params_ptr->no_http2 = true;
        break;

      case SAL_OPT_HTTP2_UPGRADE:
        params_ptr->http2_upgrade = true;
        break;

      case SAL_OPT_NO_TCP_KEEP_ALIVE:
        params_ptr->no_tcp_keep_alive = true;
        break;

      case SAL_OPT_SKIP_TLS_VERIFICATION:
        params_ptr->tls_no_verify = true;
        break;

      case SAL_OPT_ASSUME_RANGE_SUPPORT:
        params_ptr->assume_range_support = true;
        break;

      case SAL_OPT_NO_MMAP:
        params_ptr->no_mmap = true;
        break;

      case SAL_OPT_RANDOM_ORDER:
        params_ptr->random_order= true;
        break;

      case SAL_OPT_MERGE_IN_ORDER:
        params_ptr->merge_in_order= true;
        break;

      case SAL_OPT_STDOUT:
        params_ptr->to_stdout= true;
        break;

      case SAL_OPT_READ_ONLY:
        params_ptr->read_only = true;
        break;

      case SAL_OPT_NO_STATUS:
        params_ptr->no_status = true;
        break;

      case SAL_OPT_VERBOSE_LIBCURL:
        params_ptr->libcurl_verbosity = true;
        break;

      case SAL_OPT_USE_HEAD:
        params_ptr->head = true;
        break;

      case SAL_OPT_PROXY_CUSTOM_HEADERS:
        params_ptr->proxy_custom_headers = saldl_custom_headers_append(params_ptr->proxy_custom_headers, optarg);
        break;

      default:
        return 1;
        break; /* keep it here in case we change this code in the future */
    }
  }
  if (full_argc - optind != 1) {
    return 1;
  }

    params_ptr->start_url = saldl_strdup(full_argv[optind]);

  return 0;
}

int main(int argc,char **argv) {
  /* Initialize params */
  saldl_params params = DEF_SALDL_PARAMS;

  /* Set to defaults before parsing opts. Without this, any early call
   * to a log function [e.g. fatal()] would sigfault.
   */
  set_color(&params.no_color);
  set_verbosity(&params.verbosity, &params.libcurl_verbosity);

  int counter;
  int full_argc = argc;
  char **full_argv = saldl_calloc(argc, sizeof(char *));

  char *extra_argv_str = getenv("SALDL_EXTRA_ARGS");

  /* As argv is not re-allocatable, copy argv elements' pointers into full_argv */
  for (counter = 0; counter < argc; counter++) {
    full_argv[counter] = argv[counter];
  }

  /* Append extra args from env */
  if (extra_argv_str) {
    char *token  = strtok(extra_argv_str, " ");
    do {
      full_argc++;
      full_argv = saldl_realloc(full_argv, sizeof(char *) * full_argc);
      full_argv[full_argc-1] = token;
    } while ( (token = strtok(NULL, " ")) );
  }

  /* Parse opts */
  int ret_parse = parse_opts(&params, full_argc, full_argv);
  SALDL_FREE(full_argv);

  if (ret_parse) {
    /* We want --version to work, no matter what */
    if (params.print_version) {
      return saldl_version();
    } else {
      return usage(argv[0]);
    }
  }

  if (params.print_version) {
    return saldl_version();
  }

  saldl(&params);
  return 0;
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
