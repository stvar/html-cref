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
#include <stdlib.h>
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
#ifdef CLOCK_CYCLES
    clocks->cycles = 0;
#endif
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
#ifdef CLOCK_CYCLES
    if (CLOCK_TYPES_HAS(cycles))
        CLOCKS_TIME_ADD(clocks->cycles, clocks2->cycles);
#endif
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
#ifdef CLOCK_CYCLES
    if (CLOCK_TYPES_HAS(cycles))
        CLOCKS_TIME_SUB(clocks->cycles, clocks2->cycles);
#endif
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
    CASE(thread),
#ifdef CLOCK_CYCLES
    CASE(cycles)
#endif
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
    size_t i = clocks_n_defs - 1;
    clocks_time_t s = 0, t;
    struct clocks_t a;

    ASSERT(clocks->types == overhead->types);
    a.types = clocks->types;

#ifdef CLOCK_CYCLES
    if (CLOCK_TYPES_HAS(clock_type_cycles))
        ASSERT(!(clocks->types & clock_types_all));
#endif

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
    bits_t verbose: 1;
};

// $ . ~/lookup-gen/commands.sh
// $ print() { printf '%s\n' "$@"; }
// $ adjust-func() { ssed -R '1s/^/static /;1s/\&/*/;s/\bt\s+=\s+/*t = /;1s/(?<=\()/\n\t/;s/([a-z0-9_]+)s_t::([a-z0-9]+)/\U\1\E(\2)/'; }

// $ gen-lookup-func0() { print "$@"|gen-func -f clocks_lookup_type -r clock_types_t -Pf -q \!strcmp|adjust-func; }
// $ gen-lookup-func() { diff -DCLOCK_CYCLES <(gen-lookup-func0 real process thread) <(gen-lookup-func0 real process thread cycles)|ssed -R 's/(?<=^#else|^#endif)\s+\/\*\s*CLOCK_CYCLES\s*\*\/\s*$//'; }

// $ gen-lookup-func

static bool clocks_lookup_type(
    const char* n, clock_types_t* t)
{
#ifndef CLOCK_CYCLES
    // pattern: process|real|thread
#else
    // pattern: cycles|process|real|thread
#endif
    switch (*n ++) {
#ifdef CLOCK_CYCLES
    case 'c':
        if (!strcmp(n, "ycles")) {
            *t = CLOCK_TYPE(cycles);
            return true;
        }
        return false;
#endif
    case 'p':
        if (!strcmp(n, "rocess")) {
            *t = CLOCK_TYPE(process);
            return true;
        }
        return false;
    case 'r':
        if (!strcmp(n, "eal")) {
            *t = CLOCK_TYPE(real);
            return true;
        }
        return false;
    case 't':
        if (!strcmp(n, "hread")) {
            *t = CLOCK_TYPE(thread);
            return true;
        }
    }
    return false;
}

static const struct clocks_options_t*
    clocks_options(int argc, char* argv[])
{
    static const char help[] = 
"usage: ./" CLOCKS " [-?|--help] [-v|--verbose] [NAME...]\n"
#ifndef CLOCK_CYCLES
"where NAME is either 'real', 'process' or 'thread'";
#else
"where NAME is either 'real', 'process', 'thread' or 'cycles'";
#endif
    static struct clocks_options_t opts = {
        .types   = 0,
        .verbose = 0,
    };
    clock_types_t t;
    bool u = false;

    ASSERT(argc > 0);
    argv ++;
    argc --;

    for (; argc > 0; argc --, argv ++) {
        if (!strcmp(*argv, "-?") ||
             !strcmp(*argv, "--help"))
            u = true;
        else
        if (!strcmp(*argv, "-v") ||
             !strcmp(*argv, "--verbose"))
            opts.verbose = 1;
        else
        if (*argv[0] == '-')
            error("invalid command line option '%s'", *argv);
        else {
            if (!clocks_lookup_type(*argv, &t))
                error("invalid clock type '%s'", *argv);
            opts.types |= t;
        }
    }

    if (u) {
        puts(help);
        exit(0);
    }

    if (opts.types == 0)
#ifndef CLOCK_CYCLES
        opts.types = clock_types_all;
#else
        opts.types = clock_types_all |
                     CLOCK_TYPE(cycles);
#endif

    return &opts;
}

enum { clocks_n_overhead = SZ(10000000) };

struct clocks_t clocks_gettime_overhead(
    size_t clock_type)
{
#undef  CASE
#define CASE(t, n) \
    [clock_type_ ## t] = CLOCK_ ## n
    static const clockid_t ids[] = {
        CASE(real, REALTIME),
        CASE(process, PROCESS_CPUTIME_ID),
        CASE(thread, PROCESS_CPUTIME_ID),
    };
    size_t n = clocks_n_overhead;
    struct timespec s;
    struct ntime_t t;
    clockid_t c;

    ASSERT(ARRAY_INDEX(ids, clock_type));
    c = ids[clock_type];

    ntime_init(&t, clock_types_all);

    while (n --)
        clock_gettime(c, &s);

    return ntime_clocks(&t);
}

#ifdef CLOCK_CYCLES

struct clocks_t clocks_rdtsc_overhead(
    size_t clock_type UNUSED)
{
    size_t n = clocks_n_overhead;
    clocks_time_t s, e;
    struct clocks_t r;

    clocks_init(&r,
        CLOCK_TYPE(cycles));

    r.cycles = CLOCK_CYCLES_MAX;

    while (n --) {
        CLOCK_GET_RDTSC_CYCLES(s);
        CLOCK_GET_RDTSCP_CYCLES(e);

        CLOCKS_TIME_SUB(e, s);
        if (r.cycles > e)
            r.cycles = e;
    }

    return r;
}

#undef  CLOCK_TYPES_HAS
#define CLOCK_TYPES_HAS(t) \
    CLOCK_TYPES_HAS_(clock_types, t)

#endif // CLOCK_CYCLES

void clocks_print_line(
    const struct clocks_t* clocks,
#ifdef CLOCK_CYCLES
    clock_types_t clock_types,
#endif
    bool empty, bool averages,
    FILE* file)
{
    const struct clocks_def_t *p, *e;
#ifdef CLOCK_CYCLES
    size_t i = 0;
#endif

#ifndef CLOCK_CYCLES
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
#else // CLOCK_CYCLES
    for (p = clocks_defs,
         e = clocks_defs + clocks_n_defs;
         p < e;
         i ++,
         p ++) {
        int w = averages || i >= clock_type_cycles
            ? 5 : 10;

        if (empty || !CLOCK_TYPES_HAS(i))
            fprintf(file, " %*s", w, "-");
        else
        if (averages && i < clock_type_cycles)
            fprintf(file, " %5.0f",
                (double) CLOCKS_TIME(p) /
                clocks_n_overhead);
        else
            fprintf(file, " %*" PRIu64,
                w, CLOCKS_TIME(p));
    }
#endif // CLOCK_CYCLES
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
        struct clocks_t (*func)(size_t);
    };
#undef  CASE
#define CASE(n, t) \
    { .name = #n, .func = clocks_ ## t ## _overhead }
    static const struct clock_def_t clocks[] = {
        CASE(real, gettime),
        CASE(process, gettime),
        CASE(thread, gettime),
#ifdef CLOCK_CYCLES
        CASE(cycles, rdtsc),
#endif
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
            t = p->func(i);

        l = strlen(p->name) + 1;
        ASSERT(l <= 8);
        l = 8 - l;

        fprintf(stdout, "%s:%-*s %8d",
            p->name, SIZE_AS_INT(l), "",
            c ? clocks_n_overhead : 0);

        clocks_print_line(&t,
#ifdef CLOCK_CYCLES
            i == clock_type_cycles
            ? ~clock_types_all
            : clock_types_all, 
#endif
            !c, !opts->verbose, stdout);
    }

    return 0;
}

#endif // MAIN


