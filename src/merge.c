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

static void merge_finished_cb(evutil_socket_t fd, short what, void *arg) {
  info_s *info_ptr = arg;
  event_s *ev_merge = &info_ptr->ev_merge;
  chunk_s *chunks = info_ptr->chunks;

  debug_event_msg(FN, "callback no. %"SAL_JU" for triggered event %s, with what %d", ++ev_merge->num_of_calls, str_EVENT_FD(fd) , what);

  if (!exist_prg(info_ptr, PRG_MERGED, false) || info_ptr->session_status == SESSION_INTERRUPTED) {
    events_deactivate(ev_merge);
  }

  for (size_t counter = 0 ; counter < info_ptr->chunk_count ; counter++) {
    if (chunks[counter].progress == PRG_FINISHED) {
      info_ptr->merge_finished(&chunks[counter], info_ptr);
    }
  }

}

void* merger_thread(void *void_info_ptr) {
  info_s *info_ptr = (info_s*)void_info_ptr;

  /* Thread entered */
  SALDL_ASSERT(info_ptr->ev_merge.event_status == EVENT_NULL);
  info_ptr->ev_merge.event_status = EVENT_THREAD_STARTED;

  /* event loop */
  /* Use 3s max time-out, the default 0.5s is an overkill */
  info_ptr->ev_merge.tv =  (struct timeval) { .tv_sec = 3, .tv_usec = 0 };
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

int merge_finished_single() {
  return 0;
}

int merge_finished_tmpf(chunk_s *chunk, info_s *info_ptr) {
  size_t size = chunk->size;
  off_t offset = (off_t)chunk->idx * info_ptr->params->chunk_size;

  file_s *tmp_f = chunk->storage;
  char *tmp_buf = NULL;
  size_t f_ret = 0;

  saldl_fseeko(tmp_f->name, tmp_f->file, 0, SEEK_SET);
  saldl_fseeko(info_ptr->part_filename, info_ptr->file, offset, SEEK_SET);

  saldl_fflush(tmp_f->name, tmp_f->file);
  tmp_buf = saldl_calloc(size, sizeof(char));

  if ( ( f_ret = fread(tmp_buf, 1, size, tmp_f->file) ) != size ) {
    fatal(FN, "Reading from tmp file %s at offset %"SAL_JD" failed, chunk_size=%"SAL_ZU", fread() returned %"SAL_ZU".", tmp_f->name, (intmax_t)offset, size, f_ret);
  }

  saldl_fwrite_fflush(tmp_buf, 1, size, info_ptr->file, info_ptr->part_filename, offset);

  set_chunk_merged(chunk);

  saldl_fclose(tmp_f->name, tmp_f->file);

  if ( remove(tmp_f->name) ) {
    fatal(FN, "Removing file %s failed: %s", tmp_f->name, strerror(errno));
  }

  SALDL_FREE(tmp_buf);
  SALDL_FREE(tmp_f->name);
  SALDL_FREE(tmp_f);

  return 0;
}

int merge_finished_mem(chunk_s *chunk, info_s *info_ptr) {
  size_t size = chunk->size;
  off_t offset = (off_t)chunk->idx * info_ptr->params->chunk_size;

  mem_s *buf = chunk->storage;

  saldl_fseeko(info_ptr->part_filename, info_ptr->file, offset, SEEK_SET);
  saldl_fwrite_fflush(buf->memory, 1, size, info_ptr->file, info_ptr->part_filename, offset);

  SALDL_FREE(buf->memory);
  SALDL_FREE(buf);

  set_chunk_merged(chunk);

  return 0;
}

int merge_finished_null(chunk_s *chunk) {
  set_chunk_merged(chunk);
  return 0;
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
