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

chunk_s* first_prg_with_range(info_s *info_ptr, enum CHUNK_PROGRESS prg, bool match, size_t start, size_t end) {
  chunk_s *chunks = info_ptr->chunks;

  assert(start < info_ptr->chunk_count);
  assert(end < info_ptr->chunk_count);
  assert(end >= start);

  for (size_t counter = start; counter <= end; counter++) {

    if (match && chunks[counter].progress == prg) {
      return &chunks[counter];
    }

    if (!match &&  chunks[counter].progress != prg) {
      return &chunks[counter];
    }
  }

  return NULL;
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
