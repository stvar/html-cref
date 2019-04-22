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

#include "config.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "file-buf.h"
#include "int-traits.h"
#include "ptr-traits.h"
#include "common.h"

#define FILE_BUF_SYS_ERROR(c, e)       \
    do {                               \
        buf->error_info.type =         \
            file_buf_error_file_ ## c; \
        buf->error_info.sys_error = e; \
    } while (0)

static bool file_buf_read_at_once(struct file_buf_t* buf,
    int fd, size_t size)
{
    ssize_t n;

    buf->size = size;
    if (buf->size > buf->max_size) {
        FILE_BUF_SYS_ERROR(read, EFBIG);
        return false;
    }

    buf->ptr = malloc(SIZE_INC(buf->size));
    ENSURE(buf->ptr != NULL, "malloc failed");

    n = read(fd, buf->ptr, buf->size);
    if (n < 0 || INT_AS_SIZE(n) != buf->size) {
        FILE_BUF_SYS_ERROR(read, errno);
        return false;
    }
    buf->ptr[buf->size] = 0;

    return true;
}

#define FILE_BUF_REALLOC(b, s)               \
    do {                                     \
        b = realloc(b, s);                   \
        ENSURE(b != NULL, "realloc failed"); \
    } while (0)
#define FILE_BUF_ENLARGE(b, s)                     \
    do {                                           \
        if (s == 0)                                \
            s = sz;                                \
        else {                                     \
            ASSERT_SIZE_MUL_NO_OVERFLOW(s, SZ(2)); \
            s *= SZ(2);                            \
        }                                          \
        FILE_BUF_REALLOC(b, s);                    \
    } while (0)

static bool file_buf_read_incremental(struct file_buf_t* buf,
    int fd)
{
    const size_t sz = 16 * 1024;
    char c[sz];

    uchar_t* b = NULL;
    size_t d = 0;
    size_t s = 0;

    while (1) {
        size_t r;
        ssize_t n;

        n = read(fd, c, sizeof(c));
        if (n < 0) {
            FILE_BUF_SYS_ERROR(read, errno);
            goto error;
        }
        if (n == 0)
            break;

        r = INT_AS_SIZE(n);
        ASSERT_SIZE_ADD_NO_OVERFLOW(d, r);
        if (d + r > buf->max_size) {
            FILE_BUF_SYS_ERROR(read, EFBIG);
            goto error;
        }

        ASSERT(d <= s);

        ASSERT_SIZE_ADD_NO_OVERFLOW(d, r);
        if (d + r > s) {
            FILE_BUF_ENLARGE(b, s);
            ASSERT(d + r <= s);
        }

        memcpy(b + d, c, r);
        d += r;
    }

    ASSERT_SIZE_INC_NO_OVERFLOW(d);
    FILE_BUF_REALLOC(b, d + 1);
    b[d] = 0;

    buf->size = d;
    buf->ptr = b;
    return true;

error:
    free(b);
    return false;
}

void file_buf_read(struct file_buf_t* buf)
{
    struct stat st;
    int fd;

    if (buf->file_name == NULL)
        fd = 0;
    else
    if ((fd = open(buf->file_name, O_RDONLY)) < 0) {
        FILE_BUF_SYS_ERROR(open, errno);
        return;
    }

    if (fstat(fd, &st)) {
        FILE_BUF_SYS_ERROR(stat, errno);
        goto close;
    }
    if (S_ISREG(st.st_mode))
        file_buf_read_at_once(
            buf, fd, INT_AS_SIZE(st.st_size));
    else
        file_buf_read_incremental(buf, fd);

close:
    if (close(fd))
        FILE_BUF_SYS_ERROR(close, errno);
}

void file_buf_init(struct file_buf_t* buf,
    const char* file_name,
    size_t max_size)
{
    ENSURE(max_size > 0, "max_size is 0");

    memset(buf, 0, sizeof(struct file_buf_t));
    buf->file_name = file_name;
    buf->max_size = max_size;

    file_buf_read(buf);
}

void file_buf_done(struct file_buf_t* buf)
{
    free(buf->ptr);
}


