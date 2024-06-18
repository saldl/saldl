/*
    This file is a part of saldl.

    Copyright (C) 2014-2024 Mohammad AlSaleh <CE.Mohammad.AlSaleh at gmail.com>
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

#ifndef SALDL_QUEUE_H
#define SALDL_QUEUE_H
#else
#error redefining SALDL_QUEUE_H
#endif

void* queue_next_thread(void *void_info_ptr);
void prep_next(info_s *info_ptr, thread_s *thread, chunk_s *chunk, int init);
void queue_next_chunk(info_s *info_ptr, size_t thr_idx, int init);

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
