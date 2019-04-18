// Copyright (C) 2016, 2017, 2018, 2019  Stefan Vargyas
// 
// This file is part of Json-Type.
// 
// Json-Type is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Json-Type is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Json-Type.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __FILE_BUF_H
#define __FILE_BUF_H

#include "json-defs.h"

enum file_buf_error_type_t
{
    file_buf_error_none,
    file_buf_error_file_open,
    file_buf_error_file_stat,
    file_buf_error_file_read,
    file_buf_error_file_close,
};

struct file_buf_error_info_t
{
    enum file_buf_error_type_t type;
    int sys_error;
};

struct file_buf_t
{
    size_t max_size;
    const char* file_name;
    struct file_buf_error_info_t error_info;
    uchar_t* ptr;
    size_t size;
};

void file_buf_init(struct file_buf_t* buf, const char* file_name,
    size_t max_size);

void file_buf_done(struct file_buf_t* buf);

#endif/*__FILE_BUF_H*/


