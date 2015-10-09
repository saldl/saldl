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
#include "ctrl.h"

void ctrl_cleanup_info(ctrl_info_s *ctrl) {
  SALDL_FREE(ctrl->chunks_progress_str);
}

int ctrl_get_info(char *ctrl_filename, ctrl_info_s *ctrl) {

  if (access(ctrl_filename,F_OK)) {
    warn_msg(FN, "%s does not exist.", ctrl_filename);
    return 1;
  }

  FILE *f_ctrl = fopen(ctrl_filename, "rb");

  off_t ctrl_fsize = saldl_fsizeo(ctrl_filename, f_ctrl);
  SALDL_ASSERT((uintmax_t)ctrl_fsize <= SIZE_MAX);
  SALDL_ASSERT(ctrl_fsize <= INT_MAX);

  if (!ctrl_fsize) {
    fatal(FN, "ctrl file is empty.");
  }
  else {
    char *p;
    char ctrl_file_size_str[s_num_digits(OFF_T_MAX)];
    char ctrl_chunk_size_str[u_num_digits(SIZE_MAX)];
    char ctrl_rem_size_str[u_num_digits(SIZE_MAX)];

    /* ctrl_fsize guarantees allocating enough bytes */
    ctrl->chunks_progress_str = saldl_calloc((size_t)ctrl_fsize, sizeof(char) );

    char *ret_fgets1 = fgets(ctrl_file_size_str, s_num_digits(OFF_T_MAX), f_ctrl);
    char *ret_fgets2 = fgets(ctrl_chunk_size_str, u_num_digits(SIZE_MAX), f_ctrl);
    char *ret_fgets3 = fgets(ctrl_rem_size_str, u_num_digits(SIZE_MAX), f_ctrl);
    char *ret_fgets4 = fgets(ctrl->chunks_progress_str, ctrl_fsize, f_ctrl);

    if (!ret_fgets1 || !ret_fgets2 || !ret_fgets3 || !ret_fgets4) {
      fatal(FN, "Reading the ctrl file failed. Are you sure it's not corrupt!");
    }

    if ( fgetc(f_ctrl) != (char)EOF ) {
      fatal(FN, "ctrl file should have ended here.");
    }

    if (! ( strchr(ctrl_file_size_str, '\n') && strchr(ctrl_chunk_size_str, '\n') && strchr(ctrl_rem_size_str, '\n') && strchr(ctrl->chunks_progress_str, '\n') ) ) {
      fatal(FN, "Parsing ctrl file failed.");
    }

    /* Needed for strtoimax()/strtoumax() in parse_num_o()/parse_num_z()  */
    p = strchr(ctrl_file_size_str, '\n');
    *p = '\0';
    p = strchr(ctrl_chunk_size_str, '\n');
    *p = '\0';
    p = strchr(ctrl_rem_size_str, '\n');
    *p = '\0';
    p = strchr(ctrl->chunks_progress_str, '\n');
    *p = '\0';

    ctrl->file_size = parse_num_o(ctrl_file_size_str, 0);
    ctrl->chunk_size = parse_num_z(ctrl_chunk_size_str, 0);
    ctrl->rem_size = parse_num_z(ctrl_rem_size_str, 0);
    ctrl->chunk_count = strlen(ctrl->chunks_progress_str);

    info_msg(FN, "ctrl file parsed:");
    info_msg(FN, " file_size:  %"SAL_JD"", (intmax_t)ctrl->file_size);
    info_msg(FN, " chunk_size: %"SAL_ZU"", ctrl->chunk_size);
    info_msg(FN, " rem_size: %"SAL_ZU"", ctrl->rem_size);
    info_msg(FN, " chunk_count: %"SAL_ZU"", ctrl->chunk_count);
    info_msg(FN, " chunks_progress_str: %s", ctrl->chunks_progress_str);
  }
  saldl_fclose(ctrl_filename, f_ctrl);
  return 0;
}

static void ctrl_update_cb(evutil_socket_t fd, short what, void *arg) {
  info_s *info_ptr = arg;
  control_s *ctrl = &info_ptr->ctrl;
  event_s *ev_ctrl = &info_ptr->ev_ctrl;

  debug_event_msg(FN, "callback no. %"SAL_JU" for triggered event %s, with what %d", ++ev_ctrl->num_of_calls, str_EVENT_FD(fd) , what);

  /* .part file size will be used to infer progress made in single mode */
  if (info_ptr->params->single_mode) {
    events_deactivate(ev_ctrl);
  }

  /* We check if the merge loop is already de-initialized to not lose status of any merged chunks */
  if ( (info_ptr->session_status == SESSION_INTERRUPTED || !exist_prg(info_ptr, PRG_MERGED, false) ) && info_ptr->ev_merge.event_status < EVENT_INIT) {
    events_deactivate(ev_ctrl);
  }

  for (size_t counter=0; counter < info_ptr->chunk_count; counter++) {
    memset(&ctrl->raw_status[counter], '0' + info_ptr->chunks[counter].progress, 1);
  }

  saldl_fseeko(info_ptr->ctrl_filename, info_ptr->ctrl_file, ctrl->pos, SEEK_SET);

  saldl_fputs(ctrl->raw_status, info_ptr->ctrl_file, info_ptr->ctrl_filename);
  saldl_fputc('\n', info_ptr->ctrl_file, info_ptr->ctrl_filename);

  saldl_fflush(info_ptr->ctrl_filename, info_ptr->ctrl_file);
}

void* sync_ctrl(void *void_info_ptr) {
  info_s *info_ptr = (info_s*)void_info_ptr;
  control_s *ctrl = &info_ptr->ctrl;

  /* Thread entered */
  SALDL_ASSERT(info_ptr->ev_ctrl.event_status == EVENT_NULL);
  info_ptr->ev_ctrl.event_status = EVENT_THREAD_STARTED;

  /* Initialize ctrl */
  /* +1 because saldl_fputs() needs \0 termination to know where to stop */
  ctrl->raw_status = saldl_calloc(info_ptr->chunk_count + 1, sizeof(char));
  memset(ctrl->raw_status,'0', info_ptr->chunk_count);
  ctrl->pos = 0; // redundant?

  /* Get file_size, chunk_size & rem_size as strings */
  char char_file_size[s_num_digits(OFF_T_MAX)];
  char char_chunk_size[u_num_digits(SIZE_MAX)];
  char char_rem_size[u_num_digits(SIZE_MAX)];
  snprintf(char_file_size, s_num_digits(OFF_T_MAX), "%"SAL_JD"", (intmax_t)info_ptr->file_size);
  snprintf(char_chunk_size, u_num_digits(SIZE_MAX), "%"SAL_ZU"", info_ptr->params->chunk_size);
  snprintf(char_rem_size, u_num_digits(SIZE_MAX), "%"SAL_ZU"", info_ptr->rem_size);

  /* Rewind ctrl_file */
  saldl_fseeko(info_ptr->ctrl_filename, info_ptr->ctrl_file, 0, SEEK_SET);

  /* Start writing in ctrl_file */
  saldl_fputs(char_file_size, info_ptr->ctrl_file, info_ptr->ctrl_filename);
  saldl_fputc('\n', info_ptr->ctrl_file, info_ptr->ctrl_filename);
  saldl_fputs(char_chunk_size, info_ptr->ctrl_file, info_ptr->ctrl_filename);
  saldl_fputc('\n', info_ptr->ctrl_file, info_ptr->ctrl_filename);
  saldl_fputs(char_rem_size, info_ptr->ctrl_file, info_ptr->ctrl_filename);
  saldl_fputc('\n', info_ptr->ctrl_file, info_ptr->ctrl_filename);
  ctrl->pos = saldl_ftello(info_ptr->ctrl_filename, info_ptr->ctrl_file);

  /* event loop */
  events_init(&info_ptr->ev_ctrl, ctrl_update_cb, info_ptr, EVENT_CTRL);

  if (info_ptr->session_status != SESSION_INTERRUPTED && exist_prg(info_ptr, PRG_MERGED, false)) {
    debug_msg(FN, "Start ev_ctrl loop.");
    events_activate(&info_ptr->ev_ctrl);
  }

  /* Event loop exited */
  events_deinit(&info_ptr->ev_ctrl);

  /* finalize and cleanup */
  SALDL_FREE(ctrl->raw_status);
  return info_ptr;
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
