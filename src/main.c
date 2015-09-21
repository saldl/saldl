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

#include <getopt.h>

#include "saldl.h"

#ifndef SALDL_NAME
#define SALDL_NAME "saldl"
#endif

#ifndef SALDL_VERSION
#define SALDL_VERSION "(unknown version)"
#endif

#ifndef SALDL_WWW
#define SALDL_WWW "https://saldl.github.io"
#endif

static int saldl_version() {
  curl_version_info_data *curl_info = curl_version_info(CURLVERSION_NOW);
  fprintf(stderr, "%s %s (%s)\n", SALDL_NAME, SALDL_VERSION, SALDL_WWW);
  fprintf(stderr, "\n");
  fprintf(stderr, "Copyright (C) 2014-2015 Mohammad AlSaleh.\n");
  fprintf(stderr, "Free use of this software is granted under the terms of\n");
  fprintf(stderr, "the GNU Affero General Public License (AGPL).\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Built against: libcurl %s\n", LIBCURL_VERSION);
  fprintf(stderr, "Loaded: libcurl %s (%s)\n", curl_info->version, curl_info->ssl_version);
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

static int parse_opts(saldl_params *params_ptr, int full_argc, char **full_argv) {
  int opt_idx = 0, c = 0;
  static struct option long_opts[] = {
    {"verbosity" , no_argument, 0, 'V'},
    {"version" , no_argument, 0, 'v'},
    {"no-color" , no_argument, 0, 'C'},
    {"chunk-size" , required_argument, 0, 's'},
    {"last-chunks-first" , required_argument, 0, 'l'},
    {"last-size-first" , required_argument, 0, 'L'},
    {"connections", required_argument, 0, 'c'},
    {"connection-max-rate", required_argument, 0, 'R'},
    {"user-agent", required_argument, 0, 'u'},
    {"no-user-agent", no_argument, 0, 'U'},
    {"no-compress", no_argument, 0, 'Z'},
    {"no-decompress", no_argument, 0, 'z'},
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
    {"use-HEAD", no_argument, 0, 'H'},
    {"assume-partial-support", no_argument, 0, 'F'},
    {"single", no_argument, 0, 'S'},
    {"no-path", no_argument, 0, 'n'},
    {"keep-GET-attrs", no_argument, 0, 'G'},
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
    /* long only */
    {"skip-TLS-verification", no_argument, 0, CHAR_MAX+1},
    {"no-status", no_argument, 0, CHAR_MAX+2},
    {"verbose-libcurl", no_argument, 0, CHAR_MAX+3},
    {0, 0, 0, 0}
  };

  const char *opts = "s:l:L:c:R:x:X:N3OSHFIAnGdD:o:tTrfa:wmVvCK:k:p:P:e:Eu:UZz";
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
      case 's':
        params_ptr->user_chunk_size = parse_num_z(optarg, 1);
        break;
      case 'L':
        params_ptr->last_size_first = parse_num_o(optarg, 1);
        break;
      case 'l':
        params_ptr->last_chunks_first = parse_num_z(optarg, 0);
        break;
      case 'c':
        params_ptr->user_num_connections = parse_num_z(optarg, 0);
        break;
      case 'R':
        params_ptr->connection_max_rate = parse_num_z(optarg, 1);
        break;
      case 'u':
        params_ptr->user_agent = strdup(optarg);
        break;
      case 'U':
        params_ptr->no_user_agent = true;
        break;
      case 'Z':
        params_ptr->no_compress= true;
        break;
      case 'z':
        params_ptr->no_decompress= true;
        break;
      case 'p':
        params_ptr->post = strdup(optarg);
        break;
      case 'P':
        params_ptr->raw_post = strdup(optarg);
        break;
      case 'k':
        params_ptr->inline_cookies = strdup(optarg);
        break;
      case 'K':
        params_ptr->cookie_file = strdup(optarg);
        break;
      case 'e':
        params_ptr->referer = strdup(optarg);
        break;
      case 'E':
        params_ptr->auto_referer = true;
        break;
      case 'x':
        params_ptr->proxy = strdup(optarg);
        break;
      case 'X':
        params_ptr->tunnel_proxy = strdup(optarg);
        break;
      case 'N':
        params_ptr->no_proxy = true;
        break;
      case 'O':
        params_ptr->no_timeouts = true;
        break;
      case 'H':
        params_ptr->head = true;
        break;
      case 'F':
        params_ptr->skip_partial_check = true;
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
      case 'G':
        params_ptr->keep_GET_attrs = true;
        break;
      case 'd':
        params_ptr->dry_run = true;
        break;
      case 'D':
        params_ptr->root_dir = strdup(optarg);
        break;
      case 'o':
        params_ptr->filename= strdup(optarg);
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

      /* long only */
      case CHAR_MAX+1:
        params_ptr->tls_no_verify = true;
        break;

      case CHAR_MAX+2:
        params_ptr->no_status = true;
        break;

      case CHAR_MAX+3:
        params_ptr->libcurl_verbosity = true;
        break;

      default:
        return 1;
        break; /* keep it here in case we change this code in the future */
    }
  }
  if (full_argc - optind != 1) {
    return 1;
  }

    params_ptr->url = full_argv[optind];

  return 0;
}

int main(int argc,char **argv) {

  int counter;
  int full_argc = argc;
  char **full_argv = saldl_calloc(argc, sizeof(char *));

  char *extra_argv_str = getenv("SALDL_EXTRA_ARGS");

  saldl_params params = {0} ; /* initialize all members to zero */

  /* Set to defaults before parsing opts */
  set_color(&params.no_color);
  set_verbosity(&params.verbosity, &params.libcurl_verbosity);

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
  if ( parse_opts(&params, full_argc, full_argv) ) {
    /* We want --version to work, no matter what */
    if (params.print_version) {
      return saldl_version();
    } else
      return usage(argv[0]);
  }

  if (params.print_version) {
    return saldl_version();
  }

  sal_dl(&params);
  free(full_argv);
  return 0;
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
