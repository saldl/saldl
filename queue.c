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

static chunk_s* pick_next(info_s *info_ptr) {

  /* -1 because we assume the last chunk is too small. */
  size_t last_first = saldl_min(info_ptr->params->last_chunks_first, info_ptr->chunk_count - 1);
  size_t start = last_first ? info_ptr->chunk_count - last_first - 1 : 0;

  size_t end = info_ptr->chunk_count - 1;

  /* Pick not-started chunk */
  chunk_s *chunk = first_prg_with_range(info_ptr, PRG_NOT_STARTED, true, start, end);

  if (!chunk) {
    chunk = first_prg(info_ptr, PRG_NOT_STARTED, true);
  }

  assert(chunk);
  return chunk;
}

void prep_next(info_s *info_ptr, thread_s *thread, chunk_s *chunk, int init) {

  saldl_params *params_ptr = info_ptr->params;
  file_s *storage_info = &info_ptr->storage_info;

  thread->chunk = chunk;
  info_ptr->prepare_storage(thread->chunk, storage_info);

  if (init) {
    thread->ehandle = curl_easy_init() ;
    set_params(thread, params_ptr);
  }

  set_progress_params(thread, info_ptr);
  set_write_opts(thread->ehandle, thread->chunk->storage, !params_ptr->mem_bufs || params_ptr->single_mode);

  /* Don't set ranges for single mode unless we are resuming.
   * To avoid setting range for naive servers reporting 0 size */
  if ( !params_ptr->single_mode || params_ptr->resume ) {
    curl_set_ranges(thread->ehandle, thread->chunk);
  }

  set_chunk_progress(thread->chunk, PRG_QUEUED);
}

void queue_next_chunk(info_s *info_ptr, size_t thr_idx, int init) {

  chunk_s *chunk = pick_next(info_ptr);
  prep_next(info_ptr, &info_ptr->threads[thr_idx], chunk, init);

  /* Fetch */
  pthread_create(&info_ptr->threads[thr_idx].chunk->thr_id,NULL,thread_func,&info_ptr->threads[thr_idx]);

}

static void queue_next_cb(evutil_socket_t fd, short what, void *arg) {
  info_s *info_ptr = arg;
  event_s *ev_queue = &info_ptr->ev_queue;

  debug_event_msg(FN, "callback no. %ju for triggered event %s, with what %d\n", ++ev_queue->num_of_calls, str_EVENT_FD(fd) , what);

  if (info_ptr->session_status >= SESSION_QUEUE_INTERRUPTED || !exist_prg(info_ptr, PRG_NOT_STARTED, true) ) {
    events_deactivate(ev_queue);
  }

  for (size_t counter = 0; counter < info_ptr->num_connections && exist_prg(info_ptr, PRG_NOT_STARTED, true); counter++) {
    if (info_ptr->threads[counter].chunk->progress >= PRG_FINISHED) {
      queue_next_chunk(info_ptr, counter, 0);
    }
  }
}

void* queue_next_thread(void *void_info_ptr) {
  info_s *info_ptr = (info_s*)void_info_ptr;

  /* Thread entered */
  assert(info_ptr->ev_queue.event_status == EVENT_NULL);
  info_ptr->ev_queue.event_status = EVENT_THREAD_STARTED;

  /* event loop */
  events_init(&info_ptr->ev_queue, queue_next_cb, info_ptr, EVENT_QUEUE);

  if (info_ptr->session_status < SESSION_QUEUE_INTERRUPTED && exist_prg(info_ptr, PRG_NOT_STARTED, true)) {
    debug_msg(FN, "Start ev_queue loop.\n");
    events_activate(&info_ptr->ev_queue);
  }

  /* Event loop exited */
  events_deinit(&info_ptr->ev_queue);

  return info_ptr;
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
