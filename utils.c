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

#include "events.h"
#include "utime.h"

#define SEMI_FATAL_RETRIES 5

#ifndef HAVE_STRCASESTR
#include "gnulib_strcasestr.h" // gnulib implementation
#endif

#ifdef __MINGW32__
#include <direct.h> // _mkdir()
#endif

static void set_inline_cookies(CURL *handle, char *cookie_str) {
  char *copy_cookie_str = strdup(cookie_str);
  char *curr = copy_cookie_str, *next = NULL, *sep = NULL, *cookie = NULL;
  do {
    sep = strstr(curr, ";");
    if (sep) {
      next = sep + 1;
      // space
      sep[0] = '\0';
    }
    asprintf(&cookie, "Set-Cookie: %s; ", curr);
    curl_easy_setopt(handle, CURLOPT_COOKIELIST, cookie);
    free(cookie);
    curr = next;
  } while (sep);

  free(copy_cookie_str);
}

int saldl_mkdir(const char *path, mode_t mode) {
#if defined(HAVE_MKDIR)
  return mkdir(path, mode);
#elif defined(HAVE__MKDIR)
  (void) mode;
  return _mkdir(path);
#else
#error neither mkdir() nor _mkdir() available.
#endif
}

char* saldl_user_agent() {
  char *agent = saldl_calloc(1024, sizeof(char));
  snprintf(agent, 1024, "%s/%s", "libcurl", curl_version_info(CURLVERSION_NOW)->version);
  return agent;
}

void chunks_init(info_s *info_ptr) {
  size_t chunk_count = info_ptr->chunk_count;

  info_ptr->chunks = saldl_calloc(chunk_count, sizeof(chunk_s));

  for (size_t idx = 0; idx < chunk_count; idx++) {

    info_ptr->chunks[idx].idx = idx;

    /* size & ranges */
    info_ptr->chunks[idx].size = info_ptr->chunk_size;
    info_ptr->chunks[idx].range_start = (off_t)idx * info_ptr->chunks[idx].size;
    info_ptr->chunks[idx].range_end = (off_t)(idx+1) * info_ptr->chunks[idx].size - 1;
  }

  if (info_ptr->rem_size) {
    /* fix size and range_end of the last chunk */
    size_t idx = info_ptr->chunk_count - 1;
    info_ptr->chunks[idx].size = info_ptr->rem_size;
    info_ptr->chunks[idx].range_end = info_ptr->file_size - 1;
  }

  for (size_t idx = 0; idx < chunk_count; idx++) {
    /* events */
    info_ptr->chunks[idx].ev_trigger = &info_ptr->ev_trigger;
    info_ptr->chunks[idx].ev_merge = &info_ptr->ev_merge;
    info_ptr->chunks[idx].ev_queue = &info_ptr->ev_queue;
    info_ptr->chunks[idx].ev_ctrl = &info_ptr->ev_ctrl;
    info_ptr->chunks[idx].ev_status = &info_ptr->ev_status;
  }

}

void curl_set_ranges(CURL *handle, chunk_s *chunk) {
  char range_str[2 * s_num_digits(OFF_T_MAX) + 1];
  assert(chunk->range_end);
  assert( (uintmax_t)(chunk->range_end - chunk->range_start) <= (uintmax_t)SIZE_MAX );
  chunk->curr_range_start = chunk->range_start + (off_t)chunk->size_complete;
  snprintf(range_str, 2 * s_num_digits(OFF_T_MAX) + 1, "%jd-%jd", (intmax_t)chunk->curr_range_start, (intmax_t)chunk->range_end);
  curl_easy_setopt(handle, CURLOPT_RANGE, range_str);
}

/* 0 nmemb is banned, 0 size is banned */
void* saldl_calloc(size_t nmemb, size_t size) {
  void *p = NULL;
  assert(size);
  assert(nmemb);
  p = calloc(nmemb, size);
  assert(p);
  return p;
}

/* NULL ptr is banned, 0 size is banned */
void* saldl_realloc(void *ptr, size_t size) {
  void *p = NULL;
  assert(size);
  assert(ptr);
  p = realloc(ptr, size);
  assert(p);
  return p;
}

long fsize(FILE *f) {
  long curr = ftell(f);
  long size;
  fseek(f, 0l, SEEK_END);
  size = ftell(f);
  fseek(f, curr, SEEK_SET);
  return size;
}

off_t fsizeo(FILE *f) {
  off_t curr = ftello(f);
  off_t size;
  fseeko(f, 0, SEEK_END);
  size = ftello(f);
  fseeko(f, curr, SEEK_SET);
  return size;
}

off_t fsize2(char *fname) {
  struct stat st = {0};
  int ret = stat(fname, &st);
  return ret ? ret : st.st_size;
}

static size_t  header_function(  void  *ptr,  size_t  size, size_t nmemb, void *userdata) {
  char *header = strdup(ptr);
  char *tmp;

  /* Strip \r\n */
  if ( (tmp = strstr(header, "\r\n")) ) {
    memset(tmp, '\0', 2);
  }

  /* Look for Content-Disposition header */
  if ( (tmp = strcasestr(header, "Content-Disposition:")) == header ) {
    debug_msg(FN, "%s\n", header);

    /* Assumptions:
     *   1- There is only one assignment in the header line.
     *   2- The assignment has the filename as an rvalue.
     *   3- The assignment is located at the end of the header line.
     * If one of the assumptions is not met. Current code will produce broken results.
     * Having said that, the relevant RFC and the W3C documentation only show examples
     * that will work with this code.
     * */
    if ( (tmp = strrchr(header, '=')) ) {
      tmp++;

      /* Strip ';' if present at the end */
      if (tmp[strlen(tmp) - 1] == ';' ) {
        tmp[strlen(tmp) - 1] = '\0';
      }

      /* Strip quotes if they are present */
      if (*tmp == '"') {
        char *end = strrchr(tmp, '"');
        if (end && (long unsigned)(end - tmp) + 1 == strlen(tmp) ) {
          debug_msg(FN, "Stripping quotes from %s\n", tmp);
          tmp++;
          *end = '\0';
        }
      }

      /* Strip UTF-8'' if tmp starts with it, this usually comes from filename*=UTF-8''string */
      const char *to_strip = "UTF-8''";
      if (strstr(tmp, to_strip) == tmp) {
        tmp += strlen(to_strip);
      }

      /* Pass the result to userdata */
      {
        char **filename_ptr = userdata;
        debug_msg(FN, "Before basename: %s\n", tmp);
        *filename_ptr = strdup( basename(tmp) ); /* Last use of tmp (and header), so no need to back it up or use a copy */
        debug_msg(FN, "After basename: %s\n", *filename_ptr);
      }

    }
  }
  if (header) free(header);
  return size * nmemb;
}

static size_t file_write_function(void  *ptr, size_t  size, size_t nmemb, void *data) {
  size_t realsize = size * nmemb;
  file_s *tmp_f = data;

  if (!tmp_f) {
    /* Remember: This why getting info with GET works, this causes error 26 */
    return 0;
  }

  fflush(tmp_f->file);
  if ( fwrite(ptr, size, nmemb, tmp_f->file) !=  nmemb ) {
    fatal(FN, "Writing %zu bytes to file %s failed: %s\n", realsize, tmp_f->name, strerror(errno));
  }
  fflush(tmp_f->file);

  return realsize;
}

static size_t  mem_write_function(  void  *ptr,  size_t  size, size_t nmemb, void *data) {
  size_t realsize = size * nmemb;
  mem_s *mem = data;

  if (!mem) {
    /* Remember: This why getting info with GET works, this causes error 26 */
    return 0;
  }

  assert(mem->memory); // Preallocation failed
  assert(mem->size <= mem->allocated_size);

  memmove(&(mem->memory[mem->size]), ptr, realsize);
  mem->size += realsize;

  return realsize;
}

static void check_redirects(CURL *handle, saldl_params *params_ptr) {
  long redirects;
  char *url;
  curl_easy_getinfo(handle, CURLINFO_REDIRECT_COUNT, &redirects);
  if ( redirects ) {
    curl_easy_getinfo(handle, CURLINFO_EFFECTIVE_URL, &url);
    params_ptr->url = strdup(url); /* Note: strdup() because the pointer will be killed after curl_easy_cleanup() */
    fprintf(stderr, "%s%sRedirected:%s %s\n", bold, info_color, end, params_ptr->url);
  }
}

static void check_server_response(thread_s *tmp) {
  long response;
  short semi_fatal = 0;
  CURLcode ret;
semi_fatal_check_response_retry:
  ret = curl_easy_perform(tmp->ehandle);
  debug_msg(FN, "ret=%u\n", ret);

  switch (ret) {
    case CURLE_OK:
    case CURLE_WRITE_ERROR: /* Caused by *_write_function() returning 0 */
      break;
    case CURLE_SSL_CONNECT_ERROR:
    case CURLE_SEND_ERROR: // 55: SSL_write() returned SYSCALL, errno = 32
      semi_fatal++;
      if (semi_fatal <= SEMI_FATAL_RETRIES) {
        info_msg(FN, "libcurl returned semi-fatal (%d: %s) while trying to get remote info, retrying...\n", ret, tmp->err_buf);
        goto semi_fatal_check_response_retry;
      } else {
        fatal(FN, "libcurl returned semi-fatal (%d: %s) while trying to get remote info, max semi-fatal retries %u exceeded.\n", ret, tmp->err_buf, SEMI_FATAL_RETRIES);
      }
      break;
    default:
      fatal(FN, "libcurl returned (%d: %s) while trying to get remote info.\n", ret, tmp->err_buf);
  }

  curl_easy_getinfo(tmp->ehandle, CURLINFO_RESPONSE_CODE, &response);
  debug_msg(FN, "response=%ld\n", response );
}

static off_t get_remote_file_size(CURL *handle) {
  double size;
  curl_easy_getinfo(handle,CURLINFO_CONTENT_LENGTH_DOWNLOAD,&size);
  debug_msg(FN, "filesize=%f\n", size);
  if (size > 0) {
    fprintf(stderr, "%s%sFile Size:%s %.2f%s\n", bold, info_color, end, human_size(size), human_size_suffix(size));
  }
  return (off_t)size;
}

static void check_range_support(CURL *handle, saldl_params *params_ptr) {
  CURLcode ret;
  long response;
  curl_easy_setopt(handle, CURLOPT_RANGE, "4096-8191");

  debug_msg(FN, "ret=%u\n", ret = curl_easy_perform(handle) );
  curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response);
  debug_msg(FN, "response=%ld\n", response);

  switch (ret) {
    case CURLE_OK:
    case CURLE_WRITE_ERROR: /* Caused by *_write_function() returning 0 */
      switch (response) {
        case 0: /* file:// */
        case 206: /* HTTP(s) */
          return;
          break;
        case 225: /* FTP */
        case 150: /* FTP */
          /* TODO: Limited testing shows that although partial downloads are supported, server returns
           * 421 response(too many connections) which causes immediate timeout errors in libcurl.
           * Use a single connection for now **without disabling resume** until we gather more information
           * about how to detect multi-connection support.
           */
          warn_msg(FN, "single_mode is forced for FTP downloads at the moment.\n");
          params_ptr->single_mode = true;
          return;
          break;
        default:
          warn_msg(FN, "Server returned %ld while trying to get check range support.\n", response);
          warn_msg(FN, "Expected return code is 206 for HTTP(s),  225/150 for FTP and 0 for file.\n");
          warn_msg(FN, "Partial downloading for other protocols is not supported at the moment.\n");
      }
      break;
    default:
      switch (response) {
        case 416: /* HTTP Transfer < 8KiB */
          warn_msg(FN, "File size < 8KiB%s.\n", params_ptr->single_mode ? "" : ", single mode forced");
          params_ptr->single_mode = true;
          return;
          break;
        default:
          warn_msg(NULL, "libcurl returned (%d: %s) while trying to check for ranges.\n", ret, curl_easy_strerror(ret));
      }
  }
  warn_msg(NULL, "Assuming no support for ranges.\n");
  warn_msg(NULL, "%s %s\n", params_ptr->single_mode ? ""  : "Enabling single mode.", params_ptr->resume ? "Disabling resume."  : "");
  params_ptr->single_mode = true;
  params_ptr->resume = false;

}

void remote_info(info_s *info_p) {

  saldl_params *params_ptr = info_p->params;

  thread_s tmp = {0};
  tmp.ehandle = curl_easy_init();

  if (params_ptr->no_remote_info) {
    warn_msg(FN, "no_remote_info enforces both enabling single mode and disabling resume.\n");
    params_ptr->single_mode = true;
    params_ptr->resume = false;
    set_names(tmp.ehandle, info_p);
    return;
  }

  set_params(&tmp, params_ptr);

  if (!params_ptr->no_timeouts) {
    curl_easy_setopt(tmp.ehandle, CURLOPT_LOW_SPEED_TIME, 75l); /* Resolving the host for the 1st time takes a long time sometimes */
  }

  if (!params_ptr->no_attachment_detection) {
    curl_easy_setopt(tmp.ehandle, CURLOPT_HEADERFUNCTION, header_function);
    curl_easy_setopt(tmp.ehandle, CURLOPT_HEADERDATA, &params_ptr->attachment_filename);
  }

  if (params_ptr->head && !params_ptr->post && !params_ptr->raw_post) {
    info_msg(FN, "Using HEAD for remote info.\n");
    curl_easy_setopt(tmp.ehandle,CURLOPT_NOBODY,1l);
  }

  set_write_opts(tmp.ehandle, NULL, 0);

  check_server_response(&tmp);
  check_redirects(tmp.ehandle, params_ptr);

  set_names(tmp.ehandle, info_p);

  info_p->file_size = get_remote_file_size(tmp.ehandle);
  if ( info_p->file_size == -1 ) {
    debug_msg(FN, "Zeroing filesize, was -1.\n");
    info_p->file_size = 0;
  }

  if (info_p->file_size && !params_ptr->skip_partial_check && (!params_ptr->single_mode || params_ptr->resume) ) {
    check_range_support(tmp.ehandle, params_ptr);
  } else {
    debug_msg(FN, "range check skipped.\n");
  }

  curl_easy_cleanup(tmp.ehandle);
}

void check_remote_file_size(info_s *info_p) {

  saldl_params *params_ptr = info_p->params;

  if ( 0 == info_p->file_size) {
    info_msg(FN, "Server either reported 0 filesize or no filesize at all.\n");
    info_msg(FN, "This enforces both using single mode and disabling resume.\n");
    params_ptr->resume = false;
    params_ptr->single_mode = true;
  }

  if (info_p->chunk_count <= 1 || params_ptr->single_mode) {
    set_single_mode(info_p);
  }

  if (info_p->chunk_count > 1 && info_p->chunk_count < info_p->num_connections) {
    info_msg(NULL, "File relatively small, use %zu connection(s)\n", info_p->chunk_count);
    info_p->num_connections = info_p->chunk_count;
  }
}

static void whole_file(info_s *info_p) {
  if (0 < info_p->file_size) {
    info_p->chunk_size = saldl_max_z_umax((uintmax_t)info_p->chunk_size , (uintmax_t)info_p->file_size  / info_p->num_connections);
    info_p->chunk_size = info_p->chunk_size >> 12 << 12 ; /* Round down to 4k boundary */
    info_msg(FN, "Chunk size set to %.2f%s based on file size %.2f%s and number of connections %zu.\n",
        human_size(info_p->chunk_size), human_size_suffix(info_p->chunk_size),
        human_size(info_p->file_size), human_size_suffix(info_p->file_size),
        info_p->num_connections);
  }
}

static void auto_size_func(info_s *info_p, int auto_size) {
  int cols = tty_width();
  if (cols <= 0) {
    info_msg(NULL, "Couldn't retrieve tty width. Chunk size will not be modified.\n");
    return;
  }

  if (cols <= 2) {
    info_msg(NULL, "Retrieved tty width (%d) is too small.\n", cols);
    return;
  }

  if (0 < info_p->file_size) {
    size_t orig_chunk_size = info_p->chunk_size;

    if ( info_p->num_connections > (size_t)cols ) {
      info_p->num_connections = (size_t)cols; /* Limit active connections to 1 line */
      info_msg(NULL, "no. of connections reduced to %zu based on tty width %d.\n", info_p->num_connections, cols);
    }

    if ( ( info_p->chunk_size = saldl_max_z_umax((uintmax_t)orig_chunk_size, (uintmax_t)info_p->file_size / (uintmax_t)(cols * auto_size) ) ) != orig_chunk_size) {
      info_p->chunk_size = (info_p->chunk_size  + (1<<12) - 1) >> 12 << 12; /* Round up to 4k boundary */
      info_msg(FN, "Chunk size set to %.2f%s, no. of connections set to %zu, based on tty width %d and no. of lines requested %d.\n",
          human_size(info_p->chunk_size), human_size_suffix(info_p->chunk_size),
          info_p->num_connections, cols, auto_size);
    }

  }

}

void check_url(char *url) {
  /* TODO: Add more checks */
  if (! strcmp(url, "") ) {
    fatal(NULL, "Invalid empty url \"%s\".\n", url);
  }
  fprintf(stderr,"%s%sURL:%s %s\n", bold, info_color, end, url);
}

void set_names(CURL *handle, info_s* info_p) {

  saldl_params *params_ptr = info_p->params;

  if (!params_ptr->filename) {
    char *prev_unescaped, *unescaped;

    /* Get initial filename (=url if no attachment name) */
    if (params_ptr->attachment_filename) {
      prev_unescaped = strdup(params_ptr->attachment_filename);
    } else {
      prev_unescaped = strdup(params_ptr->url);
    }

    /* unescape name/url */
    void(*free_prev)(void*) = free;
    while ( strcmp(unescaped = curl_easy_unescape(handle, prev_unescaped, 0, NULL), prev_unescaped) ) {
      curl_free(prev_unescaped);
      prev_unescaped = unescaped;
      free_prev = curl_free;
    }
    free_prev(prev_unescaped);

    /* keep attachment name ,if present, as is. basename() unescaped url */
    if (params_ptr->attachment_filename) {
      params_ptr->filename = strdup(unescaped);
    } else {
      params_ptr->filename = strdup(basename(unescaped));
    }
    curl_free(unescaped);

    /* Finally, remove GET atrrs if present */
    if (!params_ptr->keep_GET_attrs) {
      debug_msg(FN, "Filename before stripping GET attrs: %s\n", params_ptr->filename);

      char *q = strrchr(params_ptr->filename, '?');
      if (q) {
        if (strchr(q, '=')) {
          q[0] = '\0';
        }
      }

      debug_msg(FN, "Filename after stripping GET attrs: %s\n", params_ptr->filename);
    }

  }

  if ( !strcmp(params_ptr->filename,"") || !strcmp(params_ptr->filename,".") || !strcmp(params_ptr->filename,"..") || !strcmp(params_ptr->filename,"...") || ( strrchr(params_ptr->filename,'/') && ( !strcmp(strrchr(params_ptr->filename,'/'), "/") || !strcmp(strrchr(params_ptr->filename,'/'), "/.") || !strcmp(strrchr(params_ptr->filename,'/'), "/..") ) ) ) {
    fatal(NULL, "Invalid filename \"%s\".\n", params_ptr->filename);
  }

  if ( params_ptr->no_path ) {
    if ( strchr(params_ptr->filename, '/') ) {
      info_msg(FN, "Replacing %s/%s with %s_%s in %s\n", error_color, end, ok_color, end, params_ptr->filename);
    }
    params_ptr->filename = valid_filename(params_ptr->filename);
  }

  /* Prepend dir if set */
  if (params_ptr->root_dir) {

    /* strip '/' if present at the end of root_dir */
    if (params_ptr->root_dir[strlen(params_ptr->root_dir)-1] == '/') {
      params_ptr->root_dir[strlen(params_ptr->root_dir)-1] = '\0';
    }

    char *curr_filename = params_ptr->filename;
    size_t full_buf_size = strlen(params_ptr->root_dir) + strlen(curr_filename) + 2; // +1 for '/', +1 for '\0'

    info_msg(FN, "Prepending root_dir(%s) to filename(%s).\n", params_ptr->root_dir, params_ptr->filename);
    params_ptr->filename = saldl_calloc(full_buf_size, sizeof(char)); // +1 for '\0'
    snprintf(params_ptr->filename, full_buf_size, "%s/%s", params_ptr->root_dir, curr_filename);
    free(curr_filename);
  }

  if (params_ptr->auto_trunc || params_ptr->smart_trunc) {
    char *before_trunc = params_ptr->filename;

    /* See if smart_trunc is enabled before auto_trunc */
    if ( params_ptr->smart_trunc ) {
      params_ptr->filename = trunc_filename( params_ptr->filename, 1);
    }

    if ( params_ptr->auto_trunc ) {
      params_ptr->filename = trunc_filename( params_ptr->filename, 0);
    }

    if ( strlen(before_trunc) != strlen(params_ptr->filename) ) {
      warn_msg(NULL,"Filename truncated:\n");
      warn_msg(NULL,"  Original: %s\n", before_trunc);
      warn_msg(NULL,"  Truncated: %s\n", params_ptr->filename);
    }
  }

  fprintf(stderr, "%s%sSaving To:%s %s\n", bold, info_color, end, params_ptr->filename);

  /* Check if filename exists  */
  if (!access(params_ptr->filename,F_OK)) {
    fatal(NULL, "%s exists, quiting...\n", params_ptr->filename);
  }

  /* Set part/ctrl filenames, tmp dir */
  {
    char cwd[PATH_MAX];
    snprintf(info_p->part_filename,PATH_MAX-(params_ptr->filename[0]=='/'?0:strlen(getcwd(cwd,PATH_MAX))+1),"%s.part.sal",params_ptr->filename);
    snprintf(info_p->ctrl_filename,PATH_MAX,"%s.ctrl.sal",params_ptr->filename);
    snprintf(info_p->tmp_dirname,PATH_MAX,"%s.tmp.sal",params_ptr->filename);
  }
}

void set_info(info_s *info_p) {

  saldl_params *params_ptr = info_p->params;

  info_p->num_connections = params_ptr->user_num_connections ? params_ptr->user_num_connections : SALDL_DEF_NUM_CONNECTIONS;
  info_p->chunk_size = params_ptr->user_chunk_size ? params_ptr->user_chunk_size : SALDL_DEF_CHUNK_SIZE;

  if (! params_ptr->single_mode) {
    if ( params_ptr->auto_size  ) {
      auto_size_func(info_p, params_ptr->auto_size);
    }

    if ( params_ptr->whole_file  ) {
      whole_file(info_p);
    }
  }

  /* Chunk size should be at least 4k */
  if (info_p->chunk_size < 4096) {
    warn_msg(FN, "Rounding up chunk_size from %zu to 4096(4k).\n", info_p->chunk_size);
    info_p->chunk_size = 4096;
  }

  info_p->rem_size = (size_t)(info_p->file_size % (off_t)info_p->chunk_size);
  info_p->chunk_count = (size_t)(info_p->file_size / (off_t)info_p->chunk_size) + !!info_p->rem_size;

}

void print_chunk_info(info_s *info_p) {
  if (info_p->file_size) { /* Avoid printing useless info if remote file size is reported 0 */

    if (info_p->rem_size && !info_p->params->single_mode) {
      fprintf(stderr, "%s%sChunks:%s %zu*%.2f%s + 1*%.2f%s\n", bold, info_color, end,info_p->chunk_count-1,
          human_size(info_p->chunk_size), human_size_suffix(info_p->chunk_size),
          human_size(info_p->rem_size), human_size_suffix(info_p->rem_size));
    } else {
      fprintf(stderr, "%s%sChunks:%s %zu*%.2f%s\n", bold, info_color, end, info_p->chunk_count,
          human_size(info_p->chunk_size), human_size_suffix(info_p->chunk_size));
    }
  }

}

void global_progress_init(info_s *info_ptr) {
  info_ptr->global_progress.start = saldl_utime();
  info_ptr->global_progress.curr = info_ptr->global_progress.prev = info_ptr->global_progress.start;
  info_ptr->global_progress.enable = 1;
}

void global_progress_update(info_s *info_ptr, bool init) {
  progress_s *p = &info_ptr->global_progress;
  chunks_progress_s *chsp = &p->chunks_progress;
  if (info_ptr->chunks) {
    size_t idx;
    off_t total_complete_size = 0;
    size_t merged = 0;
    size_t finished = 0;
    size_t started = 0;
    size_t empty_started = 0;
    size_t queued = 0;
    size_t not_started = 0;
    for (idx = 0; idx < info_ptr->chunk_count; idx++) {
      chunk_s chunk = info_ptr->chunks[idx]; /* Important to get consistent info */
      total_complete_size += chunk.size_complete;

      /* Status */
      switch (chunk.progress) {
        case PRG_MERGED:
          merged++;
          break;
        case PRG_FINISHED:
          finished++;
        case PRG_STARTED:
          started++;
          if (!chunk.size_complete) {
            empty_started++;
          }
          break;
        case PRG_QUEUED:
          queued++;
        case PRG_NOT_STARTED:
          not_started++;
          break;
        default:
          fatal(FN, "Invalid progress value %d for chunk %zu\n", chunk.progress, idx);
          break;
      }
    }

    /* Apply update */
    p->complete_size = total_complete_size;
    chsp->merged = merged;
    chsp->finished = finished;
    chsp->started = started;
    chsp->empty_started = empty_started;
    chsp->queued = queued;
    chsp->not_started = not_started;
  }

  if (init) {
    p->initial_complete_size = p->complete_size;
    p->dlprev = p->complete_size;
  }
}

static int status_single_display(void *void_info_ptr, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {

  uintmax_t lines = 5;
  info_s *info_ptr = (info_s *)void_info_ptr;

  assert(!ulnow || info_ptr->params->post || info_ptr->params->raw_post);
  assert(!ultotal || info_ptr->params->post || info_ptr->params->raw_post);

  if (info_ptr) {
    progress_s *p = &info_ptr->global_progress;
    if (p->enable) {

      if (!info_ptr->file_size && dltotal) {
        info_ptr->file_size = dltotal;
      }

      curl_off_t offset = dltotal && info_ptr->file_size > dltotal ? (curl_off_t)info_ptr->file_size - dltotal : 0;
      info_ptr->chunks[0].size_complete = (size_t)(offset + dlnow);

      if (p->enable++ == 1) {
        fputs_count(lines+1, "\n", stderr); // +1 in case offset becomes non-zero
      }

      p->curr = saldl_utime();
      p->dur = p->curr - p->start;
      p->curr_dur = p->curr - p->prev;


      if (p->curr_dur >= 0.3 || dlnow == dltotal) {
        if (p->curr_dur >= 0.3) {
          off_t curr_done = saldl_max_o(dlnow + offset - p->dlprev, 0); // Don't go -ve on reconnects
          p->curr_rate =  curr_done / p->curr_dur;
          p->curr_rem = p->curr_rate && dltotal ? (dltotal - dlnow) / p->curr_rate : INT64_MAX;

          p->prev = p->curr;
          p->dlprev = dlnow + offset;
        }

        if (p->dur >= 0.3) {
          p->rate = dlnow / p->dur;
          p->rem = p->rate && dltotal ? (dltotal - dlnow) / p->rate : INT64_MAX;
        }

        fputs_count(lines+!!offset, up, stderr);
        fprintf(stderr, "%s%s%sSingle mode progress:%s\n", erase_after, info_color, bold, end);
        fprintf(stderr, " %s%sProgress:%s\t%.2f%s / %.2f%s\n",
            erase_after, bold, end,
            human_size(dlnow + offset), human_size_suffix(dlnow + offset),
            human_size(info_ptr->file_size), human_size_suffix(info_ptr->file_size));
        if (offset) {
          fprintf(stderr, " %s%s        %s\t%.2f%s / %.2f%s (Offset: %.2f%s)\n",
              erase_after, bold, end,
              human_size(dlnow), human_size_suffix(dlnow),
              human_size(dltotal), human_size_suffix(dltotal),
              human_size(offset), human_size_suffix(offset));
        }
        fprintf(stderr, " %s%sRate:%s  \t%.2f%s/s : %.2f%s/s\n",
            erase_after, bold, end,
            human_size(p->rate), human_size_suffix(p->rate),
            human_size(p->curr_rate), human_size_suffix(p->curr_rate));
        fprintf(stderr, " %s%sRemaining:%s\t%.1fs : %.1fs\n", erase_after, bold, end, p->rem, p->curr_rem);
        fprintf(stderr, " %s%sDuration:%s\t%.1fs\n", erase_after, bold, end, p->dur);
      }
    }
  }
  return 0;
}

static int chunk_progress(void *void_chunk_ptr, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {

  assert(!ulnow);
  assert(!ultotal);

  chunk_s *chunk =  (chunk_s *)void_chunk_ptr;
  size_t rem;

  /* Check bad server behavior, e.g. if dltotal becomes file_size mid-transfer. */
  if (dltotal && dltotal != (chunk->range_end - chunk->curr_range_start + 1) ) {
    fatal(FN, "Transfer size(%jd) does not match requested range(%jd-%jd) in chunk %zu, this is a sign of a bad server, retry with a single connection.\n", (intmax_t)dltotal, (intmax_t)chunk->curr_range_start, (intmax_t)chunk->range_end, chunk->idx);
  }

  if (dlnow) { /* dltotal & dlnow can both be 0 initially */
    curl_off_t curr_chunk_size = chunk->range_end - chunk->curr_range_start + 1;
    if (dltotal != curr_chunk_size) {
      fatal(FN, "Transfer size does not equal requested range: %jd!=%jd for chunk %zu, this is a sign of a bad server, retry with a single connection.\n", (intmax_t)dltotal, (intmax_t)curr_chunk_size, chunk->idx);
    }
    rem = (size_t)(dltotal-dlnow);
  } else if (chunk->size_complete) { /* dltotal & dlnow can also both be 0 initially if a chunk download restarted */
    rem = chunk->size - chunk->size_complete;
  } else {
    rem = chunk->size;
  }
  chunk->size_complete = chunk->size - rem;

  return 0;
}

void set_progress_params(thread_s *thread, info_s *info_ptr) {

  saldl_params *params_ptr = info_ptr->params;

  if (params_ptr->single_mode) {
    curl_easy_setopt(thread->ehandle,CURLOPT_NOPROGRESS,0l);
    curl_easy_setopt(thread->ehandle,CURLOPT_XFERINFOFUNCTION,&status_single_display);
    curl_easy_setopt(thread->ehandle,CURLOPT_PROGRESSDATA,info_ptr);
  } else if (thread->chunk && thread->chunk->size) {
    curl_easy_setopt(thread->ehandle,CURLOPT_NOPROGRESS,0l);
    curl_easy_setopt(thread->ehandle,CURLOPT_XFERINFOFUNCTION,&chunk_progress);
    curl_easy_setopt(thread->ehandle,CURLOPT_PROGRESSDATA, thread->chunk);
  }
}

void set_params(thread_s *thread, saldl_params *params_ptr) {
  curl_easy_setopt(thread->ehandle, CURLOPT_ERRORBUFFER, thread->err_buf);


  /* Try HTTP/2, but don't care about the return value.
   * Most libcurl binaries would not include support for HTTP/2 in the short term */
  curl_easy_setopt(thread->ehandle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);

  /* TODO: no_remote_info is not assumed. So, the code will double-POST by default.
   * Remember to document the default behavior. */

  /* Send post fields if provided */

  /* If both raw_post & post are set, post is ignored */
  if (params_ptr->raw_post) {
    struct curl_slist *custom_headers = NULL;
    curl_easy_setopt(thread->ehandle, CURLOPT_POST, 1L);

    /* Disable headers set by CURLOPT_POST */
    custom_headers = curl_slist_append(custom_headers, "Content-Type:");
    custom_headers = curl_slist_append(custom_headers, "Content-Length:");

    /* Append raw_post as-is to custom_headers */
    custom_headers = curl_slist_append(custom_headers, params_ptr->raw_post);

    /* Add custom_headers to the request */
    curl_easy_setopt(thread->ehandle, CURLOPT_HTTPHEADER, custom_headers);
  } else if (params_ptr->post) {
    debug_msg(FN, "POST fields: %s\n", params_ptr->post);
    curl_easy_setopt(thread->ehandle,CURLOPT_POSTFIELDS, params_ptr->post);
  }

  if (params_ptr->cookie_file) {
    curl_easy_setopt(thread->ehandle, CURLOPT_COOKIEFILE, params_ptr->cookie_file);
  } else {
    /* Just enable the cookie engine */
    curl_easy_setopt(thread->ehandle, CURLOPT_COOKIEFILE, "");
  }

  if (params_ptr->inline_cookies) {
    set_inline_cookies(thread->ehandle, params_ptr->inline_cookies);
  }

  curl_easy_setopt(thread->ehandle, CURLOPT_URL, params_ptr->url);

  curl_easy_setopt(thread->ehandle,CURLOPT_NOSIGNAL,1l); /* Try to avoid threading related segfaults */
  curl_easy_setopt(thread->ehandle,CURLOPT_FAILONERROR,1l); /* Fail on 4xx errors */
  curl_easy_setopt(thread->ehandle,CURLOPT_FOLLOWLOCATION,1l); /* Handle redirects */

  if (params_ptr->libcurl_verbosity) {
    debug_msg(FN, "enabling libcurl verbose output.\n");
    curl_easy_setopt(thread->ehandle,CURLOPT_VERBOSE,1l);
  }

  if (params_ptr->connection_max_rate) {
    curl_easy_setopt(thread->ehandle, CURLOPT_MAX_RECV_SPEED_LARGE, (curl_off_t)params_ptr->connection_max_rate);
  }

  if (!params_ptr->no_timeouts) {
    curl_easy_setopt(thread->ehandle,CURLOPT_LOW_SPEED_LIMIT,512l); /* Abort if dl rate goes below .5K/s for > 15 seconds */
    curl_easy_setopt(thread->ehandle,CURLOPT_LOW_SPEED_TIME,15l);
  }

  if (params_ptr->tls_no_verify) {
    curl_easy_setopt(thread->ehandle,CURLOPT_SSL_VERIFYPEER,0l);
    curl_easy_setopt(thread->ehandle,CURLOPT_SSL_VERIFYHOST,0l);
  }

  if (!params_ptr->no_user_agent) {
    if (params_ptr->user_agent) {
      curl_easy_setopt(thread->ehandle, CURLOPT_USERAGENT, params_ptr->user_agent);
    }
    else {
      char *default_agent = saldl_user_agent();
      curl_easy_setopt(thread->ehandle, CURLOPT_USERAGENT, default_agent);
      if (default_agent) free(default_agent);
    }
  }

  /* TODO: Add option to not send Accept-Encoding */
  curl_easy_setopt(thread->ehandle, CURLOPT_ACCEPT_ENCODING, ""); /* "" sends all supported encodings */

  if (params_ptr->proxy) {
    curl_easy_setopt(thread->ehandle,CURLOPT_PROXY,params_ptr->proxy);
  }
  if (params_ptr->tunnel_proxy) {
    curl_easy_setopt(thread->ehandle,CURLOPT_PROXY,params_ptr->tunnel_proxy);
    curl_easy_setopt(thread->ehandle,CURLOPT_HTTPPROXYTUNNEL,1l);
  }
  if (params_ptr->no_proxy) {
    curl_easy_setopt(thread->ehandle,CURLOPT_PROXY,"");
  }

  if (params_ptr->auto_referer) {
    curl_easy_setopt(thread->ehandle, CURLOPT_AUTOREFERER, 1l);
  }

  if (params_ptr->referer) {
    curl_easy_setopt(thread->ehandle, CURLOPT_REFERER, params_ptr->referer);
  }
}

void set_write_opts(CURL* handle, void* storage, int file_storage) {
  if (file_storage) {
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, file_write_function);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, storage);
  } else {
    curl_easy_setopt(handle,CURLOPT_WRITEDATA,storage);
    curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,mem_write_function);
  }
}

void set_single_mode(info_s *info_p) {

  saldl_params *params_ptr = info_p->params;

  if (!params_ptr->single_mode) {
    info_msg(FN, "File small, enabling single mode.\n");
    params_ptr->single_mode = true;
  }

  /* TODO-LFS: Try to support large files with single mode in 32b systems */
  if ((uintmax_t)info_p->file_size > (uintmax_t)SIZE_MAX) {
    fatal(FN, "Trying to set single chunk size to file_size %jd, but chunk size can't exceed %zu \n", (intmax_t)info_p->file_size, SIZE_MAX);
  }

  info_p->chunk_size = (size_t)info_p->file_size;
  info_p->chunk_count = info_p->num_connections = 1;
  info_p->rem_size = 0;
}

void set_modes(info_s *info_p) {

  saldl_params *params_ptr = info_p->params;
  file_s *storage_info_p = &info_p->storage_info;

  if (! access(info_p->tmp_dirname, F_OK) ) {
    if (params_ptr->mem_bufs || params_ptr->single_mode) {
      warn_msg(FN, "%s seems to be left over. You have to delete the dir manually.\n", info_p->tmp_dirname);
    } else if (!info_p->extra_resume_set) {
      fatal(FN, "%s is left over from a previous run with different chunk size. You have to use the same chunk size or delete the dir manually.\n", info_p->tmp_dirname);
    }
  }

  if ( params_ptr->single_mode ) { /* Write to .part file directly, no mem or file buffers */
    info_msg(NULL, "single mode, writing to %s directly.\n", info_p->part_filename);
    storage_info_p->name = info_p->part_filename;
    storage_info_p->file = info_p->file;
    info_p->prepare_storage = &prepare_storage_single;
    info_p->merge_finished = &merge_finished_single;
    return;
  }

  if (params_ptr->mem_bufs) {
    info_p->prepare_storage = &prepare_storage_mem;
    info_p->merge_finished = &merge_finished_mem;
    return;
  }

  if ( saldl_mkdir(info_p->tmp_dirname, S_IRWXU) ) { /* mkdir with 700 perms */
    if (errno != EEXIST) {
      fatal(FN, "Failed to create %s: %s\n", info_p->tmp_dirname, strerror(errno) );
    }
  } else if ( info_p->extra_resume_set ) {
    warn_msg(FN, "%s did not exist. Maybe previous run used memory buffers or the dir was deleted manually.\n", info_p->tmp_dirname);
  }

  storage_info_p->name = info_p->tmp_dirname;
  info_p->prepare_storage = &prepare_storage_tmpf;
  info_p->merge_finished = &merge_finished_tmpf;
}

void set_reset_storage(info_s *info_ptr) {
  saldl_params *params_ptr = info_ptr->params;
  thread_s *threads = info_ptr->threads;

  size_t num = info_ptr->num_connections;
  void(*reset_storage)();

  if (params_ptr->single_mode) {
    reset_storage = &reset_storage_single;
  } else if (params_ptr->mem_bufs) {
    reset_storage = &reset_storage_mem;
  } else {
    reset_storage = &reset_storage_tmpf ;
  }

  for (size_t counter = 0; counter < num; counter++) {
    threads[counter].reset_storage = reset_storage;
  }

}

void reset_storage_single(thread_s *thread) {
  file_s *storage = thread->chunk->storage;
  off_t offset = saldl_max_o(fsizeo(storage->file), 4096) - 4096;
  curl_easy_setopt(thread->ehandle, CURLOPT_RESUME_FROM_LARGE, (curl_off_t)offset);
  fseeko(storage->file, offset, SEEK_SET);
  info_msg(FN, "restarting from offset %jd\n", (intmax_t)offset);
}

void prepare_storage_single(chunk_s *chunk, file_s *part_file) {
  assert(part_file->file);
  if (chunk->size_complete) {
    fseeko(part_file->file, chunk->size_complete, SEEK_SET);
  }
  chunk->storage = part_file;
}

void reset_storage_tmpf(thread_s *thread) {
  file_s *storage = thread->chunk->storage;
  fflush(storage->file);
  thread->chunk->size_complete = saldl_max(fsize(storage->file), 4096) - 4096;
  curl_set_ranges(thread->ehandle, thread->chunk);
  info_msg(FN, "restarting chunk %s from offset %zu\n", storage->name, thread->chunk->size_complete);
  fseeko(storage->file, thread->chunk->size_complete, SEEK_SET);
  thread->chunk->size_complete = 0;
}

void prepare_storage_tmpf(chunk_s *chunk, file_s* dir) {
  file_s *tmp_f = saldl_calloc (1, sizeof(file_s));
  tmp_f->name = saldl_calloc(PATH_MAX, sizeof(char));
  snprintf(tmp_f->name, PATH_MAX, "%s/%zu", dir->name, chunk->idx);
  if (chunk->size_complete) {
    if (! (tmp_f->file = fopen(tmp_f->name, "rb+"))) {
      fatal(FN, "Failed to open %s for read/write: %s\n", tmp_f->name, strerror(errno));
    }
    fseeko(tmp_f->file, chunk->size_complete, SEEK_SET);
  } else {
    if (! (tmp_f->file = fopen(tmp_f->name, "wb+"))) {
      fatal(FN, "Failed to open %s for read/write: %s\n", tmp_f->name, strerror(errno));
    }
  }
  chunk->storage = tmp_f;
}

void reset_storage_mem(thread_s *thread) {
  mem_s *buf = thread->chunk->storage;
  buf->size = 0;
}

void prepare_storage_mem(chunk_s *chunk) {
  mem_s *buf = saldl_calloc (1, sizeof(mem_s));
  buf->memory = saldl_calloc(chunk->size, sizeof(char));
  buf->allocated_size = chunk->size;
  chunk->storage = buf;
}

void saldl_perform(thread_s *thread) {
  CURLcode ret;
  long response;
  size_t retries = 0;
  size_t delay = 1;
  short semi_fatal = 0;


  while (1) {

#ifdef HAVE_SIGACTION
    /* shutdown code in OpenSSL sometimes raises SIGPIPE, and libcurl
     * should be already ignoring that signal, but for some reason, it's
     * still raised sometimes, so, we are ignoring it explicitly here */

    struct sigaction sa_orig;
    ignore_sig(SIGPIPE, &sa_orig);
#endif

    ret = curl_easy_perform(thread->ehandle);

#ifdef HAVE_SIGACTION
    /* Restore SIGPIPE handler */
    restore_sig_handler(SIGPIPE, &sa_orig);
#endif

    /* Break if everything went okay */
    if (ret == CURLE_OK && thread->chunk->size_complete == thread->chunk->size) {
      break;
    }

    switch (ret) {
      case CURLE_OK:
        if (thread->chunk->size) {
          if (!thread->chunk->size_complete) {
            /* This happens sometimes, especially when tunneling through proxies! Consider it non-fatal and retry */
            info_msg(NULL, "libcurl returned CURLE_OK for chunk %zu before getting any data , restarting (retry %zu, delay=%zu).\n", thread->chunk->idx, ++retries, delay);
          } else {
            fatal(FN, "libcurl returned CURLE_OK for chunk %zu, but completed size(%zu) != requested size(%zu). This should never happen.\n",
                thread->chunk->idx ,thread->chunk->size_complete, thread->chunk->size);
          }
        } else {
          return; // Avoid endless loop if server does not report file size
        }
        break;
      case CURLE_SSL_CONNECT_ERROR:
      case CURLE_SEND_ERROR: // 55: SSL_write() returned SYSCALL, errno = 32
        semi_fatal++;
        if (semi_fatal <= SEMI_FATAL_RETRIES) {
          info_msg(FN, "libcurl returned semi-fatal (%d: %s) while downloading chunk %zu, restarting (retry %zu, delay=%zu).\n", ret, thread->err_buf, thread->chunk->idx, ++retries, delay);
          goto semi_fatal_perform_retry;
        } else {
          fatal(NULL, "libcurl returned semi-fatal (%d: %s) while downloading chunk %zu, max semi-fatal retries %u exceeded.\n", ret, thread->err_buf, thread->chunk->idx, SEMI_FATAL_RETRIES);
        }
      case CURLE_OPERATION_TIMEDOUT:
      case CURLE_PARTIAL_FILE: /* single mode */
      case CURLE_COULDNT_RESOLVE_HOST:
      case CURLE_COULDNT_CONNECT:
      case CURLE_GOT_NOTHING:
      case CURLE_RECV_ERROR:
      case CURLE_HTTP_RETURNED_ERROR:
        if (ret == CURLE_HTTP_RETURNED_ERROR) {
          curl_easy_getinfo(thread->ehandle, CURLINFO_RESPONSE_CODE, &response);
          if (response < 500) {
            fatal(NULL, "libcurl returned fatal error (%d: %s) while downloading chunk %zu.\n", ret, thread->err_buf, thread->chunk->idx);
          } else {
            info_msg(NULL, "libcurl returned (%d: %s) while downloading chunk %zu, restarting (retry %zu, delay=%zu).\n", ret, thread->err_buf, thread->chunk->idx, ++retries, delay);
          }
        } else {
          info_msg(NULL, "libcurl returned (%d: %s) while downloading chunk %zu, restarting (retry %zu, delay=%zu).\n", ret, thread->err_buf, thread->chunk->idx, ++retries, delay);
        }
semi_fatal_perform_retry:
        sleep(delay);
        thread->reset_storage(thread);
        delay=retries*3/2;
        break;
      default:
        fatal(NULL, "libcurl returned fatal error (%d: %s) while downloading chunk %zu.\n", ret, thread->err_buf, thread->chunk->idx);
        break;
    }
  }
}

void* thread_func(void* threadS) {
  /* Block signals first */
  saldl_block_sig_pth();

  /* Detach so we don't have to waste time joining it */
  pthread_detach(pthread_self());

  thread_s* tmp = threadS;
  set_chunk_progress(tmp->chunk, PRG_STARTED);
  saldl_perform(tmp);
  set_chunk_progress(tmp->chunk, PRG_FINISHED);
  return threadS;
}

void curl_cleanup(info_s *info_ptr) {

  for (size_t counter = 0; counter < info_ptr->num_connections; counter++) {
    curl_easy_cleanup(info_ptr->threads[counter].ehandle);
  }

  curl_global_cleanup();
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
