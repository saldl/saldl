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

#ifndef _SALDL_PROGRESS_H
#define _SALDL_PROGRESS_H
#else
#error redefining _SALDL_PROGRESS_H
#endif

/* enum for chunk progress */
enum CHUNK_PROGRESS {
  PRG_NOT_STARTED = 0,
  PRG_QUEUED = 1,
  PRG_STARTED = 2,
  PRG_FINISHED = 3, // but not merged
  PRG_MERGED = 4,
  PRG_UNDEF = 9
};

/* enum for chunk progress as char */
enum CH_CHUNK_PROGRESS {
  CH_PRG_NOT_STARTED = '0',
  CH_PRG_QUEUED = '1',
  CH_PRG_STARTED = '2',
  CH_PRG_FINISHED = '3', // but not merged
  CH_PRG_MERGED = '4',
  CH_PRG_UNDEF = '9'
};

#include "structs.h"

bool exist_prg(info_s *info_ptr, enum CHUNK_PROGRESS prg, bool match);
chunk_s* first_prg_with_range(info_s *info_ptr, enum CHUNK_PROGRESS prg, bool match, size_t start, size_t end);
chunk_s* first_prg(info_s *info_ptr, enum CHUNK_PROGRESS prg, bool match);
size_t first_prg_idx(info_s *info_ptr, enum CHUNK_PROGRESS prg, bool match);
void set_chunk_progress(chunk_s *chunk, enum CHUNK_PROGRESS progress);

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
