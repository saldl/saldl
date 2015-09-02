/*
    This file is a part of saldl.

    Copyright (C) 2014-2015 Mohammad AlSaleh <CE.Mohammad.AlSaleh at gmail.com>
    https://github.com/saldl/saldl

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

void usage(char *caller) {
  fprintf(stderr, "VERSION\n");
  fprintf(stderr, " %s %s\n", SALDL_NAME, SALDL_VERSION);
  fprintf(stderr, " built against libcurl %s\n", LIBCURL_VERSION);
  fprintf(stderr, " loaded %s\n", curl_version() );
  fprintf(stderr, "USAGE\n");
  fprintf(stderr, " %s [ -r ] [ -f ] [ -n ] [ -d ] [ -t ] [ -T ] [ -w ] [ -F ] [ -S ] [ -x <proxy> | -X <proxy> ] [ -e <referer> ] [ -E ] [ -k <cookies> ] [ -K <cookie_file> ] [-u <user-agent> ] [ -U ] [-p <post>] [-P <raw_post>] [ -N ] [ -A ] [ -V <num> ] [ -c <num> ] [ -R <rate>  ]  [ -s <size> ] [ -l <size> ] [ -a <num> ] [ -D <dir> ] [ -o <filename> ] url\n", caller);
  fprintf(stderr, "  -D <dir> --root-dir=<dir>\n    Prepend dir to filename. \n");
  fprintf(stderr, "  -o <filename> --output-filename=<filename>\n    save output in this file. \n");
  fprintf(stderr, "  -c <num>  --connections=<num>\n    no. of connections. (default: %zu) \n", SALDL_DEF_NUM_CONNECTIONS);
  fprintf(stderr, "  -R <num>  --connection-max-rate=<num>\n    maximum rate per connection (default: 0 [unlimited]) \n");
  fprintf(stderr, "  -s <size> --chunk-size=size\n    chunk size (one char from [bBkKmMgG] can be appended). (default: %.2lf%s) \n", human_size(SALDL_DEF_CHUNK_SIZE), human_size_suffix(SALDL_DEF_CHUNK_SIZE));
  fprintf(stderr, "  -l <num> --last-chunks_first=<num>\n    the number of last chunks that should be downloaded first. (default: 0) \n");
  fprintf(stderr, "  -A --no-attachment-detection\n    Do not use Content-Disposition attachment filename if present. \n");
  fprintf(stderr, "  -S --single\n    a single chunk single connection mode (wget-like). \n");
  fprintf(stderr, "  -r --resume\n    resume download / only if started by %s. \n", caller);
  fprintf(stderr, "  -f --force\n    overwrite .part file if not resuming from a non-zero offset. \n");
  fprintf(stderr, "  -d --dry-run\n    do not download, only display information. \n");
  fprintf(stderr, "  -n --no-path\n    assume path is relative, replace all / or : with _. \n");
  fprintf(stderr, "  -G --keep-GET-attrs\n    keep GET attributes at the end of a filename(not needed with -o/--output-filename).\n");
  fprintf(stderr, "  -t --auto-trunc\n    truncate filename if name or path is too long. \n");
  fprintf(stderr, "  -T --smart-trunc\n    same as --auto-trunc but tries to keep the file extension. (overrides -t/--auto-trunc) \n");
  fprintf(stderr, "  -a <num> --auto-size=<num>\n    modify (chunk size/no. of connections) so that chunk progress can fit in <num> lines. \n");
  fprintf(stderr, "  -w --whole-file\n    make %s work like other accelerators (multiple connections, no chunking). (-s becomes a minimum) \n", caller);
  fprintf(stderr, "  -m --memory-buffers\n    Use memory buffers instead of temp files for downloading segments. \n"); /* TODO: Update with disadvantages */
  fprintf(stderr, "  -N --no-proxy\n    disable all proxies even if set in the environment. \n");
  fprintf(stderr, "  -O --no-timeouts\n    disable all timeouts. \n");
  fprintf(stderr, "  -E --auto-referer\n    auto set referer in case of a redirect. \n");
  fprintf(stderr, "  -e <referer> --referer=<referer>\n    set referer. \n");
  fprintf(stderr, "  -K <cookie_file> --cookie-file=<cookie_file>\n    File to read cookies from. \n");
  fprintf(stderr, "  -k <cookies> --inline-cookies=<cookies>\n    Add cookies. \n");
  fprintf(stderr, "  -p <post> --post=<post>\n    Send those fields in a simple POST request (no multipart). \n");
  fprintf(stderr, "  -P <raw_post> --raw-post=<raw_post>\n    Send this POST request as-is including headers (supports multipart) \n");
  fprintf(stderr, "  -U --no-user-agent\n    Don't set user agent (disables default agent). \n");
  fprintf(stderr, "  -u <agent> --user-agent=<agent>\n    set user agent. \n");
  fprintf(stderr, "  -x <proxy> --proxy=<proxy>\n    set proxy. \n");
  fprintf(stderr, "  -X <proxy> --tunnel-proxy=<proxy>\n    similar to --proxy but tunneling all traffic through it. \n");
  fprintf(stderr, "  -V  --verbosity\n    Increase verbosity level. \n");
  fprintf(stderr, "  -C --no-color\n    Disable colors in output. Passing it twice will disable all other formattings. \n");
  fprintf(stderr, " The following options only exist in long form because they should be used with care:\n");
  fprintf(stderr, "  --TLS-no-verify\n      skip TLS/SSL verification.\n");
  fprintf(stderr, " The following options exist for debugging purposes or to help in rare setups:\n");
  fprintf(stderr, "  -H --use-HEAD\n");
  fprintf(stderr, "  -I --no-remote-info\n");
  fprintf(stderr, "  -F --assume-partial-support\n");
  fprintf(stderr, "ENVIRONMENT\n");
  fprintf(stderr, " In addition to all environment variables that affect libcurl, the following variables are supported:\n");
  fprintf(stderr, "  SALDL_EXTRA_ARGS\n    Append extra arguments to argv\n");
  exit(EXIT_FAILURE);
}

int parse_opts(saldl_params *params_ptr, int full_argc, char **full_argv) {
  int opt_idx = 0, c = 0;
  static struct option long_opts[] = {
    {"verbosity" , no_argument, 0, 'V'},
    {"no-color" , no_argument, 0, 'C'},
    {"chunk-size" , required_argument, 0, 's'},
    {"last-chunks-first" , required_argument, 0, 'l'},
    {"connections", required_argument, 0, 'c'},
    {"connection-max-rate", required_argument, 0, 'R'},
    {"user-agent", required_argument, 0, 'u'},
    {"no-user-agent", no_argument, 0, 'U'},
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
    {"TLS-no-verify", no_argument, 0, CHAR_MAX+1},
    {0, 0, 0, 0}
  };

  const char *opts = "s:l:c:R:x:X:N3OSHFIAnGdD:o:tTrfa:wmVCK:k:p:P:e:Eu:U";
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
  set_verbosity(&params_ptr->verbosity);

  opt_idx = 0 , optind = 0;
  while (1) {
    c = getopt_long(full_argc, full_argv, opts, long_opts, &opt_idx);
    if ( c == -1  ) break;
    switch (c) {
      case 'C':
      case 'V':
        break;
      case 's':
        params_ptr->user_chunk_size = parse_num_z(optarg, 1);
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
  set_verbosity(&params.verbosity);

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
    usage(argv[0]);
  }

  sal_dl(&params);
  free(full_argv);
  return 0;
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
