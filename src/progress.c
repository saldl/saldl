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

bool exist_prg(info_s *info_ptr, enum CHUNK_PROGRESS prg, bool match) {
  chunk_s *chunks = info_ptr->chunks;

  /* In our use-cases, we return early if we loop backwards */
  /* counter+1 to avoid using the underflowed value in the comparison */
  for (size_t counter = info_ptr->chunk_count - 1; counter+1 > 0 ; counter--) {

    if (match && chunks[counter].progress == prg) {
      return true;
    }

    if (!match &&  chunks[counter].progress != prg) {
      return true;
    }

  }

  return false;
}

chunk_s* prg_with_range(info_s *info_ptr, enum CHUNK_PROGRESS prg, bool match, size_t start, size_t end) {
  chunk_s *chunks = info_ptr->chunks;

  SALDL_ASSERT(start < info_ptr->chunk_count);
  SALDL_ASSERT(end < info_ptr->chunk_count);

  bool reverse;

  if (end >= start) {
    reverse = false;
  }
  else {
    reverse = true;
  }

  size_t index = start;
  while(true) {
    if (!reverse && index > end) break;
    /* Checking against SIZE_MAX in case the decrement caused wrapping */
    if (reverse && (index < end || index == SIZE_MAX)) break;

    if (match && chunks[index].progress == prg) {
      return &chunks[index];
    }

    if (!match &&  chunks[index].progress != prg) {
      return &chunks[index];
    }

    index += reverse ? -1 : 1;
  }

  return NULL;
}

chunk_s* first_prg_with_range(info_s *info_ptr, enum CHUNK_PROGRESS prg, bool match, size_t start, size_t end) {
  SALDL_ASSERT(end >= start);
  return prg_with_range(info_ptr, prg, match, start, end);
}

chunk_s* last_prg_with_range(info_s *info_ptr, enum CHUNK_PROGRESS prg, bool match, size_t start, size_t end) {
  SALDL_ASSERT(end <= start);
  return prg_with_range(info_ptr, prg, match, start, end);
}

chunk_s* first_prg(info_s *info_ptr, enum CHUNK_PROGRESS prg, bool match) {
  return first_prg_with_range(info_ptr, prg, match, 0, info_ptr->chunk_count-1);
}

void set_chunk_progress(chunk_s *chunk, enum CHUNK_PROGRESS progress){
  chunk->progress = progress;
  event_queue(chunk->ev_trigger, chunk->ev_queue);
  event_queue(chunk->ev_trigger, chunk->ev_merge);
  event_queue(chunk->ev_trigger, chunk->ev_ctrl);
  event_queue(chunk->ev_trigger, chunk->ev_status);
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
