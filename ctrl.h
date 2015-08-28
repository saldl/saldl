#ifndef _SALDL_CTRL_H
#define _SALDL_CTRL_H
#else
#error redefining _SALDL_CTRL_H
#endif

typedef struct {
 off_t file_size;
 size_t chunk_size;
 size_t rem_size;
 size_t chunk_count;
 char* chunks_progress_str;
}  ctrl_info_s;


void ctrl_cleanup_info(ctrl_info_s *ctrl);
int ctrl_get_info(char *ctrl_filename, ctrl_info_s *ctrl);
void* sync_ctrl(void *void_info_ptr);

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
