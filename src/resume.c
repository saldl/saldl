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

#include "transfer.h"
#include "ctrl.h"

static void extra_resume(info_s *info_ptr, char* chunks_progress_str) {
  size_t idx;
  char c;

  if ( info_ptr->chunk_count != strlen(chunks_progress_str) ) {
    fatal(FN, "invalid chunks_progress_str length.\n");
  }

  for (idx = info_ptr->initial_merged_count; idx <  info_ptr->chunk_count; idx++) {
    c = chunks_progress_str[idx];
    switch (c) {
      case CH_PRG_MERGED:
        {
          set_chunk_merged(&info_ptr->chunks[idx]);
          info_ptr->initial_merged_count++;
          debug_msg(FN, "chunk %zu was merged in a previous run.\n", idx);
          break;
        }
      case CH_PRG_FINISHED:
      case CH_PRG_STARTED:
        {
          char idx_filename[PATH_MAX];
          size_t tmpf_size;

          if (info_ptr->params->mem_bufs) {
            debug_msg(FN, "Can't use incomplete tmp file for chunk %zu with memory buffers.\n", idx);
            break;
          }

          snprintf(idx_filename, PATH_MAX, "%s/%zu", info_ptr->tmp_dirname, idx);
          tmpf_size = fsize2(idx_filename);

          if (tmpf_size == (size_t)-1) {
            debug_msg(FN, "Can't stat %s, the file does not exist! This could be caused by using mem bufs or unsynced ctrl file.\n", idx_filename);
            debug_msg(FN, "chunk %zu will be downloaded from scratch.\n", idx);
            break;
          }
          if (tmpf_size > info_ptr->chunks[idx].size) {
            fatal(FN, "%s size exceeds chunk_size!! (size=%zu, chunk_size=%zu)\n", idx_filename, tmpf_size, info_ptr->chunks[idx].size);
          }

          info_ptr->chunks[idx].size_complete = saldl_max(4096, tmpf_size) - 4096 ; /* -4096 in case last bits are corrupted */
          debug_msg(FN, "chunk %zu was incomplete or unmerged in a previous run (Progress: %zu/%zu).\n", idx, info_ptr->chunks[idx].size_complete, info_ptr->chunks[idx].size);
          break;
        }
      case CH_PRG_QUEUED:
      case CH_PRG_NOT_STARTED:
        {
          break;
        }
      default:
        {
          fatal(FN, "Invalid chunk status '%c'\n", c);
          break;
        }
    }
  }

  info_ptr->extra_resume_set = true;
}

static off_t resume_was_single(info_s *info_ptr) {
  off_t done_size = 0;

  FILE *f_part;

  if ( ( f_part = fopen(info_ptr->part_filename, "rb") ) ) {
    done_size  = saldl_max_o(4096, fsizeo(f_part)) - 4096; /* -4096 in case last bits are corrupted */
    info_ptr->initial_merged_count = (size_t)(done_size / info_ptr->params->chunk_size);
    info_msg(FN, " done_size:  %jd (based on the size of %s)\n", (intmax_t)done_size, info_ptr->part_filename);
    fclose(f_part);
  }

  return done_size;
}

static off_t resume_was_default(info_s *info_ptr, ctrl_info_s *ctrl) {
  off_t done_size = 0;

  char STR_PRG_MERGED[] = {CH_PRG_MERGED , '\0'};
  size_t merged_cont = strspn(ctrl->chunks_progress_str, STR_PRG_MERGED);

  assert(merged_cont <= ctrl->chunk_count);

  if (merged_cont == ctrl->chunk_count) {
    done_size = ctrl->file_size;
    info_ptr->initial_merged_count = info_ptr->chunk_count;
  } else {
    done_size = ctrl->chunk_size * (off_t)merged_cont;
    info_ptr->initial_merged_count = (size_t)(done_size / info_ptr->params->chunk_size);
  }

  assert(done_size <= info_ptr->file_size);
  info_msg(FN, " done_size:  %jd\n", (intmax_t)done_size);

  for (size_t idx=0; idx<info_ptr->initial_merged_count; idx++) {
    set_chunk_merged(&info_ptr->chunks[idx]);
  }

  return done_size;
}

int check_resume(info_s *info_ptr) {
  ctrl_info_s ctrl;
  int ret = ctrl_get_info(info_ptr->ctrl_filename, &ctrl);
  if (ret) {
    info_ptr->params->resume = false;
    return 1;
  }

  if (access(info_ptr->part_filename,F_OK)) {
    info_msg(FN, "Nothing to resume: %s does not exist.\n", info_ptr->part_filename);
    info_ptr->params->resume = false;
    return 2;
  }

  if (info_ptr->file_size != ctrl.file_size) {
    if (ctrl.file_size) {
      fatal(FN, "Server filesize(%jd) does not match control filesize(%jd).", (intmax_t)info_ptr->file_size, (intmax_t)ctrl.file_size);
    } else {
      warn_msg(FN, "Control filesize is zero, assuming it's the same file.\n");
    }
  }

  off_t done_size = 0;
  if ((uintmax_t)ctrl.chunk_size == (uintmax_t)ctrl.file_size) {
    done_size = resume_was_single(info_ptr);
  } else {
    done_size = resume_was_default(info_ptr, &ctrl);
  }

  if (info_ptr->params->single_mode) {
    info_ptr->chunks[0].size_complete = done_size;
    info_msg(FN, "Resuming using single mode from offset: %.2f%s\n", human_size(done_size), human_size_suffix(done_size));
  } else {
    info_msg(FN, "Resuming from offset: %zu*%.2f%s (%.2f%s)\n",
        info_ptr->initial_merged_count, human_size(info_ptr->params->chunk_size), human_size_suffix(info_ptr->params->chunk_size),
        human_size((off_t)info_ptr->params->chunk_size*info_ptr->initial_merged_count), human_size_suffix((off_t)info_ptr->params->chunk_size*info_ptr->initial_merged_count));
  }

  /* More can be done if chunk_size is not altered between runs if not single mode */
  if ( (ctrl.chunk_size == info_ptr->params->chunk_size) && (ctrl.rem_size == info_ptr->rem_size) && ((uintmax_t)ctrl.chunk_size != (uintmax_t)ctrl.file_size) ) {
    extra_resume(info_ptr, ctrl.chunks_progress_str);
  }

  /* Correct num_connections if remaining chunks are not as many */
  size_t orig_num_connections = info_ptr->params->num_connections;
  size_t rem_chunks = info_ptr->chunk_count - info_ptr->initial_merged_count;
  if ( (info_ptr->params->num_connections = saldl_min(orig_num_connections, rem_chunks)) != orig_num_connections ) {
    info_msg(FN, "Remaining data after resume relatively small, use %zu connection(s)...\n", info_ptr->params->num_connections);
  }

  ctrl_cleanup_info(&ctrl);

  /* Set initial values in global_progress */
  global_progress_update(info_ptr, true);

  return 0;
}

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
