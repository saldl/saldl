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

#include "events.h"

static void merge_finished_cb(evutil_socket_t fd, short what, void *arg) {
  info_s *info_ptr = arg;
  event_s *ev_merge = &info_ptr->ev_merge;

  debug_event_msg(FN, "callback no. %"SAL_JU" for triggered event %s, with what %d", ++ev_merge->num_of_calls, str_EVENT_FD(fd) , what);

  if (!exist_prg(info_ptr, PRG_MERGED, false) || info_ptr->session_status == SESSION_INTERRUPTED) {
    events_deactivate(ev_merge);
  }

  chunk_s *first_finished = NULL;
  while ( (first_finished = first_prg(info_ptr, PRG_FINISHED, true)) ) {
    /* Don't delay merging if:
     * 1- We are not piping.
     * 2- It's the first chunk
     * 3- All previous chunks have been merged/sent to pipe.
     */
    if (!info_ptr->params->to_stdout ||
        !first_finished->idx ||
        !first_prg_with_range(info_ptr, PRG_MERGED, false, 0, first_finished->idx - 1)
        ) {
      info_ptr->merge_finished(first_finished, info_ptr);
    }
    else {
      break;
    }
  }

}

void* merger_thread(void *void_info_ptr) {
  info_s *info_ptr = (info_s*)void_info_ptr;

  /* Thread entered */
  SALDL_ASSERT(info_ptr->ev_merge.event_status == EVENT_NULL);
  info_ptr->ev_merge.event_status = EVENT_THREAD_STARTED;

  /* event loop */
  /* Use 2s max time-out, the default 0.5s is an overkill */
  info_ptr->ev_merge.tv =  (struct timeval) { .tv_sec = 2, .tv_usec = 0 };
  events_init(&info_ptr->ev_merge, merge_finished_cb, info_ptr, EVENT_MERGE_FINISHED);

  if (exist_prg(info_ptr, PRG_MERGED, false) && info_ptr->session_status != SESSION_INTERRUPTED) {
    debug_msg(FN, "Start ev_merge loop.");
    events_activate(&info_ptr->ev_merge);
  }

  /* Event loop exited */
  events_deinit(&info_ptr->ev_merge);

  return info_ptr;
}

void set_chunk_merged(chunk_s *chunk) {
  chunk->size_complete = chunk->size;
  set_chunk_progress(chunk, PRG_MERGED);
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
