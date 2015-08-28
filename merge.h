#ifndef _SALDL_MERGE_H
#define _SALDL_MERGE_H
#else
#error redefining _SALDL_MERGE_H
#endif

void* merger_thread(void *void_info_ptr);
void set_chunk_merged(chunk_s *chunk);
int merge_finished_single();
int merge_finished_tmpf(info_s *info_ptr, chunk_s *chunk);
int merge_finished_mem(info_s *info_ptr, chunk_s *chunk);

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
