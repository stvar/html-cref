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

#include "va-args.h"

typedef unsigned clock_types_t;
typedef uint64_t clocks_time_t;

#define CLOCKS_TIME_MAX UINT64_MAX

#ifdef CLOCK_CYCLES
typedef uint64_t clock_cycles_t;

#define CLOCK_CYCLES_MAX UINT64_MAX
#endif

enum {
    clock_type_real,
    clock_type_process,
    clock_type_thread,
#ifdef CLOCK_CYCLES
    clock_type_cycles,
#endif
};

#define CLOCK_BIT(c) \
    (                \
        1U << c      \
    )
#define CLOCK_TYPES_HAS_(s, t) \
    (                          \
        (s) & CLOCK_BIT(t)     \
    )
#define CLOCK_TYPE(n) \
    CLOCK_BIT(clock_type_ ## n)
#define CLOCK_TYPES(...) \
    (VA_ARGS_REPEAT(|, CLOCK_TYPE, __VA_ARGS__))

// stev: not including 'cycles' in
// 'clock_types_all' since 'cycles'
// isn't measuring time -- but CPU
// instruction cycles
enum {
    clock_types_all =
    CLOCK_TYPES(real, process, thread)
};

struct clocks_t
{
    clock_types_t types;

    clocks_time_t real;
    clocks_time_t process;
    clocks_time_t thread;
#ifdef CLOCK_CYCLES
    clock_cycles_t cycles;
#endif
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


