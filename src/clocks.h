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

#ifndef __CLOCKS_H
#define __CLOCKS_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef unsigned clock_types_t;
typedef uint64_t clocks_time_t;

#define CLOCKS_TIME_MAX UINT64_MAX

enum {
    clock_type_real,
    clock_type_process,
    clock_type_thread,
};

#define CLOCK_TYPES_HAS_(s, t) \
    (                          \
        (s) & (1U << t)        \
    )

enum {
    clock_types_all = 
        (1U << clock_type_real) |
        (1U << clock_type_process) |
        (1U << clock_type_thread)
};

struct clocks_t
{
    clock_types_t types;

    clocks_time_t real;
    clocks_time_t process;
    clocks_time_t thread;
};

void clocks_init(
    struct clocks_t* clocks,
    clock_types_t types);

void clocks_add(
    struct clocks_t* clocks,
    const struct clocks_t* clocks2);

void clocks_sub(
    struct clocks_t* clocks,
    const struct clocks_t* clocks2);

void clocks_adjust(
    struct clocks_t* clocks,
    const struct clocks_t* overhead,
    size_t count);

void clocks_print(
    const struct clocks_t* clocks,
    const char* name, size_t width,
    FILE* file);

void clocks_print_names(
    clock_types_t clocks,
    FILE* file);

#endif // __CLOCKS_H


