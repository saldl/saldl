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

#ifndef _SALDL_SALDL_DEFAULTS_H
#define _SALDL_SALDL_DEFAULTS_H
#else
#error redefining _SALDL_SALDL_DEFAULTS_H
#endif

#define SALDL_NAME "saldl"
#define SALDL_WWW "https://saldl.github.io"

#ifndef SALDL_VERSION
#define SALDL_VERSION "(unknown version)"
#endif

/* Default Params */
#define SALDL_DEF_CHUNK_SIZE (size_t)1*1024*1024 /* 1.00 MiB */
#define SALDL_DEF_NUM_CONNECTIONS (size_t)6

/* vim: set filetype=c ts=2 sw=2 et spell foldmethod=syntax: */
