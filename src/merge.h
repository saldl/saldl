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

#ifndef SALDL_MERGE_H
#define SALDL_MERGE_H
#else
#error redefining SALDL_MERGE_H
#endif

void* merger_thread(void *void_info_ptr);
void set_chunk_merged(chunk_s *chunk);
int merge_finished_single();
int merge_finished_tmpf(info_s *info_ptr, chunk_s *chunk);
int merge_finished_mem(info_s *info_ptr, chunk_s *chunk);

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
