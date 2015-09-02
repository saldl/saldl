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

static void merge_finished_cb(evutil_socket_t fd, short what, void *arg) {
  info_s *info_ptr = arg;
  event_s *ev_merge = &info_ptr->ev_merge;
  chunk_s *chunks = info_ptr->chunks;

  debug_event_msg(FN, "callback no. %ju for triggered event %s, with what %d\n", ++ev_merge->num_of_calls, str_EVENT_FD(fd) , what);

  if (!exist_prg(info_ptr, PRG_MERGED, false) || info_ptr->session_status == SESSION_INTERRUPTED) {
    events_deactivate(ev_merge);
  }

  for (size_t counter = 0 ; counter < info_ptr->chunk_count ; counter++) {
    if (chunks[counter].progress == PRG_FINISHED) {
      info_ptr->merge_finished(info_ptr, &chunks[counter]);
    }
  }

}

void* merger_thread(void *void_info_ptr) {
  info_s *info_ptr = (info_s*)void_info_ptr;

  /* Thread entered */
  assert(info_ptr->ev_merge.event_status == EVENT_NULL);
  info_ptr->ev_merge.event_status = EVENT_THREAD_STARTED;

  /* event loop */
  /* Use 3s max time-out, the default 0.5s is an overkill */
  info_ptr->ev_merge.tv =  (struct timeval) { .tv_sec = 3, .tv_usec = 0 };
  events_init(&info_ptr->ev_merge, merge_finished_cb, info_ptr, EVENT_MERGE_FINISHED);

  if (exist_prg(info_ptr, PRG_MERGED, false) && info_ptr->session_status != SESSION_INTERRUPTED) {
    debug_msg(FN, "Start ev_merge loop.\n");
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

int merge_finished_tmpf(info_s *info_ptr, chunk_s *chunk) {

  FILE *f = info_ptr->file;
  size_t size = chunk->size;
  off_t offset = (off_t)chunk->idx * info_ptr->chunk_size;

  file_s *tmp_f = chunk->storage;
  char *tmp_buf = NULL;
  size_t f_ret = 0;

  if ( fseeko(tmp_f->file, 0, SEEK_SET) ) {
    fatal(FN, "%s failed to rewind.\n", tmp_f->name);
  }
  if ( fseeko(f, offset, SEEK_SET) ) {
    fatal(FN, ".part file failed to seek.\n");
  }

  fflush(tmp_f->file);
  tmp_buf = saldl_calloc(size, sizeof(char));

  if ( ( f_ret = fread(tmp_buf, 1, size, tmp_f->file) ) != size ) {
    fatal(FN, "Reading from tmp file %s at offset %jd failed, chunk_size=%zu, fread() returned %zu.\n", tmp_f->name, (intmax_t)offset, size, f_ret);
  }
  if ( fwrite(tmp_buf, 1, size, f) != size ) {
    fatal(FN, "Write to file %s failed at offset %jd: %s\n", tmp_f->name, (intmax_t)offset, strerror(errno));
  }

  fflush(f); /* Very important, especially if the process was killed/interrupted */

  set_chunk_merged(chunk);

  if ( fclose(tmp_f->file) ) {
    fatal(FN, "Closing file %s failed: %s\n", tmp_f->name, strerror(errno));
  }

  if ( remove(tmp_f->name) ) {
    fatal(FN, "Removing file %s failed: %s\n", tmp_f->name, strerror(errno));
  }

  free(tmp_buf);
  free(tmp_f->name);
  free(tmp_f);

  return 0;
}

int merge_finished_mem(info_s *info_ptr, chunk_s *chunk) {

  FILE *f = info_ptr->file;
  size_t size = chunk->size;
  off_t offset = (off_t)chunk->idx * info_ptr->chunk_size;

  mem_s *buf = chunk->storage;

  fseeko(f, offset, SEEK_SET);
  if (buf->memory) {
    if ( fwrite(buf->memory,1, size,f) != size ) {
      fatal(FN, "Write to file failed at offset %jd: %s\n", (intmax_t)offset, strerror(errno));
    }
    free(buf->memory);
    free(buf);
  }
  else {
    fatal(FN, "buffer does not exist, offset=%jd\n", (intmax_t)offset);
  }

  set_chunk_merged(chunk);

  return 0;
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
