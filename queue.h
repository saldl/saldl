#ifndef _SALDL_QUEUE_H
#define _SALDL_QUEUE_H
#else
#error redefining _SALDL_QUEUE_H
#endif

void* queue_next_thread(void *void_info_ptr);
void prep_next(info_s *info_ptr, thread_s *thread, chunk_s *chunk, int init);
void queue_next_chunk(info_s *info_ptr, size_t thr_idx, int init);

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
