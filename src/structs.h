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

#ifndef SALDL_STRUCTS_H
#define SALDL_STRUCTS_H
#else
#error redefining SALDL_STRUCTS_H
#endif

#include <pthread.h>
#include <event2/thread.h>
#include <event2/event.h>

#include <curl/curl.h>

#include "saldl_params.h"

/* enum for event status */
enum EVENT_STATUS {
  EVENT_NULL = 0,
  EVENT_THREAD_STARTED = 1,
  EVENT_INIT = 2,
  EVENT_ACTIVE = 3
};

/* enum for session_status */
enum SESSION_STATUS {
  SESSION_INIT = 0,
  SESSION_IN_PROGRESS = 1,
  SESSION_QUEUE_INTERRUPTED = 2,
  SESSION_INTERRUPTED = 3
};

/* enum for (fake) negative fd values used in custom events */
enum EVENT_FD {
  EVENT_STATUS = -1,
  EVENT_CTRL = -2,
  EVENT_MERGE_FINISHED = -3,
  EVENT_QUEUE = -4,
  EVENT_TRIGGER = -62,
  EVENT_NONE = -63
};

/* event_s */
typedef struct {
  struct timeval tv;
  struct event_base *ev_b;
  struct event *ev;
  enum EVENT_STATUS event_status;
  enum EVENT_FD vFD;
  pthread_mutex_t ev_mutex;
  pthread_mutexattr_t ev_mutex_attr;
  uintmax_t num_of_calls;
  int64_t queued;
} event_s;

/* mem_s: for memory buffers */
typedef struct {
  char *memory;
  size_t size;
  size_t allocated_size;
} mem_s;

/* file_s: groups file and filename under one ptr, used when tmp files are used as buffers */
typedef struct {
  char *name;
  FILE *file;
} file_s;

/* chunk_s: fields needed for each chunk */
typedef struct {
  pthread_t thr_id;
  size_t idx;
  size_t size;
  size_t size_complete;
  off_t range_start;
  off_t curr_range_start; /* used for strict checking when resuming or resetting */
  off_t range_end;
  void *storage;
  enum CHUNK_PROGRESS progress;
  event_s *ev_trigger;
  event_s *ev_merge;
  event_s *ev_queue;
  event_s *ev_ctrl;
  event_s *ev_status;
} chunk_s;

/* thread_s: fields needed for each thread/connection */
typedef struct {
  CURL* ehandle;
  struct curl_slist *header_list;
  struct curl_slist *proxy_header_list;
  char err_buf[CURL_ERROR_SIZE];
  void (*reset_storage)();
  chunk_s *chunk;
  bool single;
} thread_s;

/* chunks_progress_s: progress of all chunks */
typedef struct {
  size_t merged;
  size_t finished;
  size_t started;
  size_t empty_started;
  size_t queued;
  size_t not_started;
} chunks_progress_s;

/* progress_s: progress of the whole session/download */
typedef struct {
  int initialized;
  double start;
  double curr;
  double prev;
  double dur;
  double curr_dur;
  double rem;
  double curr_rem;
  off_t dlprev;
  off_t complete_size;
  off_t initial_complete_size; /* Session start (only relevant with resume) */
  double rate;
  double curr_rate;
  chunks_progress_s chunks_progress;
} progress_s;

/* status_s: Variables used in status updates */
typedef struct {
  size_t c_char_size;
  char *chunks_status;
  size_t lines;
} status_s;

/* control_s: Variables used in .ctrl.sal file updates */
typedef struct {
  char *raw_status;
  long pos;
} control_s;

typedef struct {
  char *location;
  char *content_range;
  char *content_encoding;
  char *content_type;
  char *content_disposition;
} headers_s;

/* info_s: mother of all structs */
typedef struct {
  char *curr_url;
  saldl_params *params;
  curl_version_info_data *curl_info;
  file_s storage_info;
  void (*prepare_storage)();
  int (*merge_finished)();
  pthread_t trigger_events_pth;
  pthread_t queue_next_pth;
  pthread_t merger_pth;
  pthread_t sync_ctrl_pth;
  pthread_t status_display_pth;
  size_t rem_size;
  size_t chunk_count;
  size_t initial_merged_count;
  bool extra_resume_set;
  char *content_type;
  long redirects_count;
  FILE* file;
  FILE* ctrl_file;
  char tmp_dirname[PATH_MAX];
  char part_filename[PATH_MAX];
  char ctrl_filename[PATH_MAX];
  off_t file_size;
  bool file_size_from_dltotal;
  bool content_encoded;
  headers_s headers;
  thread_s *threads;
  chunk_s *chunks;
  progress_s global_progress;
  enum SESSION_STATUS session_status;
  status_s status;
  control_s ctrl;
  bool events_queue_done;
  event_s ev_trigger;
  event_s ev_merge;
  event_s ev_queue;
  event_s ev_status;
  event_s ev_ctrl;
  bool already_finished;
  bool called_exit;
} info_s;

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
