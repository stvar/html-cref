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
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "common.h"
#include "clocks.h"
#include "clocks-impl.h"

void clocks_init(
    struct clocks_t* clocks,
    clock_types_t types)
{
    clocks->types = types;
    clocks->real = 0;
    clocks->process = 0;
    clocks->thread = 0;
}

#undef  CLOCK_TYPES_HAS
#define CLOCK_TYPES_HAS(n) \
    CLOCK_TYPES_HAS_(clocks->types, clock_type_ ## n)

void clocks_add(
    struct clocks_t* clocks,
    const struct clocks_t* clocks2)
{
    ASSERT(clocks->types == clocks2->types);

    if (CLOCK_TYPES_HAS(real))
        CLOCKS_TIME_ADD(clocks->real, clocks2->real);
    if (CLOCK_TYPES_HAS(process))
        CLOCKS_TIME_ADD(clocks->process, clocks2->process);
    if (CLOCK_TYPES_HAS(thread))
        CLOCKS_TIME_ADD(clocks->thread, clocks2->thread);
}

void clocks_sub(
    struct clocks_t* clocks,
    const struct clocks_t* clocks2)
{
    ASSERT(clocks->types == clocks2->types);

    if (CLOCK_TYPES_HAS(real))
        CLOCKS_TIME_SUB(clocks->real, clocks2->real);
    if (CLOCK_TYPES_HAS(process))
        CLOCKS_TIME_SUB(clocks->process, clocks2->process);
    if (CLOCK_TYPES_HAS(thread))
        CLOCKS_TIME_SUB(clocks->thread, clocks2->thread);
}

struct clocks_def_t
{
    size_t      offset;
    const char* name;
};

#define CLOCKS_TIME_(c, p, q)     \
    (                             \
        (q clocks_time_t*)        \
        (((char*) c) + p->offset) \
    )

#undef  CASE
#define CASE(n)                  \
    {                            \
        .offset = offsetof(      \
            struct clocks_t, n), \
        .name = #n               \
    }
static const struct clocks_def_t clocks_defs[] = {
    CASE(real),
    CASE(process),
    CASE(thread)
};

enum { clocks_n_defs = ARRAY_SIZE(clocks_defs) };

// stev: here is the reasoning applied for adjusting
// the timing measurements; the notations to be seen
// below become obvious once noticing that:
// * 'm' is the code of which timings are measured;
// * 't' is 'x + y', the timing overhead introduced
//   by the respective clock library function;
// * 'x' is the part of the overhead 't' amounting
//   for the time spend before assigning the value
//   to be returned by the clock library function;
// * 'y' is the part of the overhead 't' amounting
//   for the time spend after assigning the value
//   to be returned by the clock library function;
// * 'd' is the difference between two consecutive
//   measurements of the same clock type.
// The values of 'x' and 'y' are unknown, since the
// clock library functions are considered opaque.
// However, the value of 't' is estimated for each
// clock type separately -- see the function below
// 'clocks_gettime_overhead'.
//
//       |<-------------------d[k+1]------------------->|
//       |           |<--------d[k]-------->|           |
//       |           |                      |           |
// x[k+1]!y[k+1] x[k]!y[k] ... m[1] ... x[k]!y[k] x[k+1]!y[k+1]
//              |         |    ...     |         |
//              |         |<---m[k]--->|         |
//              |<------------m[k+1]------------>|
//
// We have, for all k >= 1:
//
// t[k] = x[k] + y[k]
// d[k] = m[k] + t[k]
// m[k+1] = m[k] + 2 * t[k]
//
// For all k > 1:
//
// d[k] = m[k] + t[k]
//      = m[k-1] + 2 * t[k-1] + t[k]
//      ...
//      = m[1] + 2 * (t[1] + ... + t[k-1]) + t[k]
//
// Let a[k] be the adjustment to be applied to the
// measured clock timing d[k]: a[k] := d[k] - m[1].
//    
// => a[k] = 2 * sum{t[i], i=1..k-1} + t[k], k > 1,
//    a[1] = t[1].

#undef  CLOCK_TYPES_HAS
#define CLOCK_TYPES_HAS(t) \
    CLOCK_TYPES_HAS_(clocks->types, t)

#define OVERHEAD_TIME(p) CLOCKS_TIME_(overhead, p, const)
#define CLOCKS_TIME(p)   CLOCKS_TIME_(&a, p, )

void clocks_adjust(
    struct clocks_t* clocks,
    const struct clocks_t* overhead,
    size_t count)
{
    const struct clocks_def_t *b, *p;
    const clocks_time_t c = count;
    clocks_time_t s = 0, t;
    struct clocks_t a;
    size_t i = 2;

    ASSERT(clocks->types == overhead->types);
    a.types = clocks->types;

    for (b = clocks_defs,
         p = clocks_defs + clocks_n_defs;
         p > b; i --) {
         p --;
        if (!CLOCK_TYPES_HAS(i))
            continue;

        t = *OVERHEAD_TIME(p);
        CLOCKS_TIME_MUL(t, c);

        *CLOCKS_TIME(p) =
        CLOCKS_TIME_ADD(s, t);
        CLOCKS_TIME_ADD(s, t);
    }

    clocks_sub(clocks, &a);
}

#undef  CLOCKS_TIME
#define CLOCKS_TIME(p) \
    (*CLOCKS_TIME_(clocks, p, const))

void clocks_print(
    const struct clocks_t* clocks,
    const char* name, size_t width,
    FILE* file)
{
    const struct clocks_def_t *p, *e;
    size_t i = 0, l, w;

    ASSERT(name != NULL);
    l = strlen(name) + 1;

    for (p = clocks_defs,
         e = clocks_defs + clocks_n_defs;
         p < e;
         p ++,
         i ++) {
        if (!CLOCK_TYPES_HAS(i))
            continue;
        w = strlen(p->name);
        w = SIZE_ADD(w, l);
        w = w < width ? width - w : 0;
        fprintf(file, "%s-%s:%-*s %" PRIu64 "\n",
            p->name, name, SIZE_AS_INT(w), "",
            CLOCKS_TIME(p));
    }
}

#undef  CLOCK_TYPES_HAS
#define CLOCK_TYPES_HAS(t) \
    CLOCK_TYPES_HAS_(clocks, t)

void clocks_print_names(
    clock_types_t clocks,
    FILE* file)
{
    const struct clocks_def_t *p, *e;
    size_t i = 0, j = 0;

    for (p = clocks_defs,
         e = clocks_defs + clocks_n_defs;
         p < e;
         p ++,
         i ++) {
        if (!CLOCK_TYPES_HAS(i))
            continue;
        if (j ++) fputc(',', file);
        fputs(p->name, file);
    }
}

#ifdef MAIN

#define CLOCKS STRINGIFY(PROGRAM)

const char program[] = CLOCKS;

struct clocks_options_t
{
    clock_types_t types;
    bool verbose: 1;
};

// $ . ~/lookup-gen/commands.sh
// $ print() { printf '%s\n' "$@"; }
// $ adjust-func() { ssed -R '1s/^/static /;1s/\&/*/;s/\bt\s+=\s+/*t = 1U << /;1s/(?<=\()/\n\t/;s/_t::/_/'; }

// $ adjust-func() { ssed -R '1s/^/static /;1s/\&/*/;s/\bt\s+=\s+/*t = 1U << /;1s/(?<=\()/\n\t/;s/s_t::/_/'; }

// $ print real process thread|gen-func -f clocks_lookup_type -r clock_types_t -Pf -q \!strcmp|adjust-func

static bool clocks_lookup_type(
    const char* n, clock_types_t* t)
{
    // pattern: process|real|thread
    switch (*n ++) {
    case 'p':
        if (!strcmp(n, "rocess")) {
            *t = 1U << clock_type_process;
            return true;
        }
        return false;
    case 'r':
        if (!strcmp(n, "eal")) {
            *t = 1U << clock_type_real;
            return true;
        }
        return false;
    case 't':
        if (!strcmp(n, "hread")) {
            *t = 1U << clock_type_thread;
            return true;
        }
    }
    return false;
}

static const struct clocks_options_t*
    clocks_options(int argc, char* argv[])
{
    static const char help[] = 
        CLOCKS ": usage: ./" CLOCKS " [-v] [NAME...]\n"
        CLOCKS ": where NAME is either 'real', 'process' or 'thread'";
    static struct clocks_options_t opts = {
        .types   = 0,
        .verbose = 0,
    };
    clock_types_t t;

    ASSERT(argc > 0);
    argv ++;
    argc --;

    opts.verbose = argc > 0
        && (!strcmp(*argv, "-v")
            || !strcmp(*argv, "--verbose"));

    opts.types = 0;

    for (; argc > 0; argc --, argv ++) {
        if (!clocks_lookup_type(*argv, &t))
            error("invalid clock type '%s'\n%s",
                *argv, help);
        opts.types |= t;
    }

    if (opts.types == 0)
        opts.types = clock_types_all;

    return &opts;
}

enum { clocks_n_overhead = SZ(10000000) };

struct clocks_t clocks_gettime_overhead(
    clockid_t id)
{
    size_t n = clocks_n_overhead;
    struct timespec s;
    struct ntime_t t;

    ntime_init(&t,
        clock_types_all);

    while (n --)
        clock_gettime(id, &s);

    return ntime_clocks(&t);
}

void clocks_print_line(
    const struct clocks_t* clocks,
    bool empty, bool averages, FILE* file)
{
    const struct clocks_def_t *p, *e;

    for (p = clocks_defs,
         e = clocks_defs + clocks_n_defs;
         p < e;
         p ++) {
        if (empty)
            fprintf(file, " %*s",
                averages ? 5 : 10, "-");
        else
        if (averages)
            fprintf(file, " %5.0f",
                (double) CLOCKS_TIME(p) /
                clocks_n_overhead);
        else
            fprintf(file, " %10" PRIu64,
                CLOCKS_TIME(p));
    }
    fputc('\n', file);
}

#undef  CLOCK_TYPES_HAS
#define CLOCK_TYPES_HAS(i) \
    CLOCK_TYPES_HAS_(opts->types, i)

int main(int argc, char* argv[])
{
    struct clock_def_t
    {
        const char* name;
        clockid_t   id;
    };
#undef  CASE
#define CASE(n, t) { .name = #n, .id = CLOCK_ ## t }
    static const struct clock_def_t clocks[] = {
        CASE(real, REALTIME),
        CASE(process, PROCESS_CPUTIME_ID),
        CASE(thread, THREAD_CPUTIME_ID),
    };
    const struct clocks_options_t* opts =
        clocks_options(argc, argv);
    const struct clock_def_t *p, *e;
    struct clocks_t t;
    size_t i = 0, l;
    bool c;

    for (p = clocks,
         e = p + ARRAY_SIZE(clocks);
         p < e;
         p ++,
         i ++) {
        if ((c = CLOCK_TYPES_HAS(i)))
            t = clocks_gettime_overhead(p->id);

        l = strlen(p->name) + 1;
        ASSERT(l <= 8);
        l = 8 - l;

        fprintf(stdout, "%s:%-*s %8d",
            p->name, SIZE_AS_INT(l), "",
            c ? clocks_n_overhead : 0);

        clocks_print_line(&t,
            !c, !opts->verbose, stdout);
    }

    return 0;
}

#endif // MAIN


