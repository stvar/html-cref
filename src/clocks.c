// Copyright (C) 2019  Stefan Vargyas
// 
// This file is part of Html-Cref.
// 
// Html-Cref is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Html-Cref is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Html-Cref.  If not, see <http://www.gnu.org/licenses/>.

#include "config.h"

#include <stddef.h>
#include <stdio.h>

#include "common.h"
#include "clocks.h"
#include "clocks-impl.h"

static void clocks_time_print(
    const struct clocks_time_t* time,
    bool nseconds, FILE* file)
{
    ASSERT(time->nsecs <= CLOCKS_MAX_NSECS);

    if (!nseconds)
        fprintf(file, "%zu.%09zu",
            time->secs, time->nsecs);
    else {
        size_t s = time->secs;

        VERIFY_SIZE_MUL_NO_OVERFLOW(
            s, CLOCKS_LIM_NSECS);
        s *= CLOCKS_LIM_NSECS;

        VERIFY_SIZE_ADD_NO_OVERFLOW(
            s, time->nsecs);
        fprintf(file, "%zu",
            s + time->nsecs);
    }
}

void clocks_init(
    struct clocks_t* clocks)
{
    memset(clocks, 0, sizeof(*clocks));
}

void clocks_add(
    struct clocks_t* clocks,
    const struct clocks_t* clocks2)
{
    clocks_time_add(&clocks->real, &clocks2->real);
    clocks_time_add(&clocks->process, &clocks2->process);
    clocks_time_add(&clocks->thread, &clocks2->thread);
}

void clocks_print(
    const struct clocks_t* clocks,
    const char* name, size_t width,
    bool nseconds, FILE* file)
{
    struct time_def_t
    {
        const char* name;
        size_t offset;
    };
#undef  CASE
#define CASE(n)                 \
    {                           \
        .name = #n,             \
        .offset = offsetof(     \
            struct clocks_t, n) \
    }
#define CLOCKS_TIME(p)                 \
    (                                  \
        (const struct clocks_time_t*)  \
        (((char*) clocks) + p->offset) \
    )
    static const struct time_def_t defs[] = {
        CASE(real),
        CASE(process),
        CASE(thread)
    };
    const struct time_def_t *p, *e;
    size_t l, w;

    l = strlen(name);
    for (p = defs,
         e = defs + ARRAY_SIZE(defs);
         p < e;
         p ++) {
        w = strlen(p->name);
        w = SIZE_ADD(w, l);
        w = SIZE_INC(w);
        w = w < width ? width - w : 0;
        fprintf(file, "%s-%s:%-*s ",
            p->name, name, SIZE_AS_INT(w), "");
        clocks_time_print(
            CLOCKS_TIME(p), nseconds, file);
        fputc('\n', file);
    }
}


