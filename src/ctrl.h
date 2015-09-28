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

#ifndef SALDL_CTRL_H
#define SALDL_CTRL_H
#else
#error redefining SALDL_CTRL_H
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
