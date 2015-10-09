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

#include "events.h"

static size_t last_chunk_from_last_size(info_s *info_ptr) {
  size_t rem_last_sz;
  off_t last_sz_1st =  info_ptr->params->last_size_first;

  if (last_sz_1st >= info_ptr->file_size) {
    warn_msg(FN, "last_size_first > file_size, disabled.");
    last_sz_1st = 0;
  }

  /* Avoid overflowing size_t */
  if (OFF_T_MAX > SIZE_MAX && last_sz_1st >= (off_t)SIZE_MAX) {
    warn_msg(FN, "lowering last_size_first to %.2lf%s",
        human_size(SIZE_MAX),
        human_size_suffix(SIZE_MAX)
        );
    last_sz_1st = (off_t)SIZE_MAX;
  }

  SALDL_ASSERT(last_sz_1st >= 0);

  rem_last_sz = (size_t)last_sz_1st;

  if (rem_last_sz <= info_ptr->rem_size) {
    return rem_last_sz ? 1 : 0;
  }
  else {
    size_t chunk_sz = info_ptr->params->chunk_size;
    rem_last_sz -= info_ptr->rem_size;
    return (rem_last_sz / chunk_sz) + !!(rem_last_sz % chunk_sz) + !!(info_ptr->rem_size);
  }
}

static chunk_s* pick_next_last_first(info_s *info_ptr) {
  size_t last_first, start_idx;
  size_t end_idx = info_ptr->chunk_count - 1;

  if (!info_ptr->params->last_size_first && !info_ptr->params->last_chunks_first) {
    return NULL;
  }

  if (info_ptr->params->last_size_first) {
    last_first = last_chunk_from_last_size(info_ptr);
  }
  else if (info_ptr->rem_size) {
  /* last chunk is smaller, so we don't take it into account */
    last_first = saldl_min(info_ptr->params->last_chunks_first+1, end_idx);
  }
  else {
    last_first = saldl_min(info_ptr->params->last_chunks_first, end_idx);
  }

  /* -1 for indices */
  start_idx = last_first ? info_ptr->chunk_count - last_first : 0;
  debug_msg(FN, "start_idx=%"SAL_ZU", end_idx=%"SAL_ZU"", start_idx, end_idx);

  /* Pick not-started chunk */
  return first_prg_with_range(info_ptr, PRG_NOT_STARTED, true, start_idx, end_idx);

}

static chunk_s* pick_next(info_s *info_ptr) {
  chunk_s *chunk = NULL;

  if (! (chunk = pick_next_last_first(info_ptr)) ) {
    chunk = first_prg(info_ptr, PRG_NOT_STARTED, true);
  }

  SALDL_ASSERT(chunk);
  return chunk;
}

void prep_next(info_s *info_ptr, thread_s *thread, chunk_s *chunk, int init) {

  saldl_params *params_ptr = info_ptr->params;
  file_s *storage_info = &info_ptr->storage_info;

  thread->chunk = chunk;
  info_ptr->prepare_storage(thread->chunk, storage_info);

  if (params_ptr->single_mode) {
    thread->single = true;
  }

  if (init) {
    thread->ehandle = curl_easy_init() ;
    set_params(thread, info_ptr);
  }

  set_progress_params(thread, info_ptr);
  set_write_opts(thread->ehandle, thread->chunk->storage, params_ptr, false);

  /* Don't set ranges for single mode unless we are resuming.
   * To avoid setting range for naive servers reporting 0 size */
  if ( !params_ptr->single_mode || params_ptr->resume ) {
    curl_set_ranges(thread->ehandle, thread->chunk);
  }

  set_chunk_progress(thread->chunk, PRG_QUEUED);
}

void queue_next_chunk(info_s *info_ptr, size_t thr_idx, int init) {

  thread_s *thr = &info_ptr->threads[thr_idx];
  chunk_s *chunk = pick_next(info_ptr);

  prep_next(info_ptr, thr, chunk, init);

  /* Fetch */
  saldl_pthread_create(&thr->chunk->thr_id, NULL, thread_func, thr);
}

static void queue_next_cb(evutil_socket_t fd, short what, void *arg) {
  info_s *info_ptr = arg;
  event_s *ev_queue = &info_ptr->ev_queue;

  debug_event_msg(FN, "callback no. %"SAL_JU" for triggered event %s, with what %d", ++ev_queue->num_of_calls, str_EVENT_FD(fd) , what);

  if (info_ptr->session_status >= SESSION_QUEUE_INTERRUPTED || !exist_prg(info_ptr, PRG_NOT_STARTED, true) ) {
    events_deactivate(ev_queue);
  }

  for (size_t counter = 0; counter < info_ptr->params->num_connections && exist_prg(info_ptr, PRG_NOT_STARTED, true); counter++) {
    if (info_ptr->threads[counter].chunk->progress >= PRG_FINISHED) {
      queue_next_chunk(info_ptr, counter, 0);
    }
  }
}

void* queue_next_thread(void *void_info_ptr) {
  info_s *info_ptr = (info_s*)void_info_ptr;

  /* Thread entered */
  SALDL_ASSERT(info_ptr->ev_queue.event_status == EVENT_NULL);
  info_ptr->ev_queue.event_status = EVENT_THREAD_STARTED;

  /* event loop */
  events_init(&info_ptr->ev_queue, queue_next_cb, info_ptr, EVENT_QUEUE);

  if (info_ptr->session_status < SESSION_QUEUE_INTERRUPTED && exist_prg(info_ptr, PRG_NOT_STARTED, true)) {
    debug_msg(FN, "Start ev_queue loop.");
    events_activate(&info_ptr->ev_queue);
  }

  /* Event loop exited */
  events_deinit(&info_ptr->ev_queue);

  return info_ptr;
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
