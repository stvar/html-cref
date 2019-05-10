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

#ifndef __CLOCKS_IMPL_H
#define __CLOCKS_IMPL_H

#include <time.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "common.h"
#include "clocks.h"
#include "ptr-traits.h"

#define ALWAYS_INLINE inline __attribute__((always_inline))

#define CLOCKS_MAX_NSECS SZ(999999999)
#define CLOCKS_LIM_NSECS SZ(1000000000)

#define CLOCKS_TIME_INIT(t, s)           \
    ({                                   \
        uint64_t __s, __n;               \
        STATIC(TYPEOF_IS(s,              \
            struct timespec));           \
        STATIC(TYPEOF_IS(t, uint64_t));  \
        __s = INT_AS_SIZE(s.tv_sec);     \
        __n = INT_AS_SIZE(s.tv_nsec);    \
        ASSERT(__s <= UINT64_MAX /       \
            CLOCKS_LIM_NSECS);           \
        __s *= CLOCKS_LIM_NSECS;         \
        ASSERT(__s <= UINT64_MAX - __n); \
        (t) = __s + __n;                 \
    })

#define CLOCKS_TIME_ADD(x, y)            \
    ({                                   \
        STATIC(TYPEOF_IS(x, uint64_t));  \
        STATIC(TYPEOF_IS(y, uint64_t));  \
        ASSERT((x) <= UINT64_MAX - (y)); \
        (x) += (y);                      \
    })

#define CLOCKS_TIME_SUB(x, y)            \
    ({                                   \
        STATIC(TYPEOF_IS(x, uint64_t));  \
        STATIC(TYPEOF_IS(y, uint64_t));  \
        ASSERT((x) >= (y));              \
        (x) -= (y);                      \
    })

#define CLOCKS_TIME_MUL(x, y)            \
    ({                                   \
        STATIC(TYPEOF_IS(x, uint64_t));  \
        STATIC(TYPEOF_IS(y, uint64_t));  \
        ASSERT(                          \
            (y) == 0 ||                  \
            (x) <= UINT64_MAX / (y));    \
        (x) *= (y);                      \
    })

#ifdef CLOCK_CYCLES

// Gabriele Paoloni:
// How to Benchmark Code Execution Times on Intel
// IA-32 and IA-64 Instruction Set Architectures.
// White Paper, Intel Corporation, September 2010
// https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/
// ia-32-ia-64-benchmark-code-execution-paper.pdf

#define CLOCK_GET_RDTSC_CYCLES(c)  \
    ({                             \
        uint32_t __l, __h;         \
        STATIC(TYPEOF_IS(c,        \
            uint64_t));            \
        asm volatile (             \
            "cpuid\n"              \
            "rdtsc\n"              \
            "mov %%edx, %0\n"      \
            "mov %%eax, %1\n" :    \
            "=r" (__h),            \
            "=r" (__l) ::          \
            "%rax", "%rbx",        \
            "%rcx", "%rdx"         \
        );                         \
        c = (uint64_t) __h << 32 | \
            __l;                   \
    })
#define CLOCK_GET_RDTSCP_CYCLES(c) \
    ({                             \
        uint32_t __l, __h;         \
        STATIC(TYPEOF_IS(c,        \
            uint64_t));            \
        asm volatile (             \
            "rdtscp\n"             \
            "mov %%edx, %0\n"      \
            "mov %%eax, %1\n"      \
            "cpuid\n" :            \
            "=r" (__h),            \
            "=r" (__l) ::          \
            "%rax", "%rbx",        \
            "%rcx", "%rdx"         \
        );                         \
        c = (uint64_t) __h << 32 | \
            __l;                   \
    })

#endif // CLOCK_CYCLES

struct ntime_t
{
    clock_types_t   types;
    struct timespec real;
    struct timespec process;
    struct timespec thread;
#ifdef CLOCK_CYCLES
    clock_cycles_t  cycles;
#endif
};

#ifdef CLOCK_CYCLES
#define CLOCKS_INIT_CYCLES(c, n) \
    (c).cycles = (n)->cycles
#else
#define CLOCKS_INIT_CYCLES(c, n)
#endif

#define CLOCKS_INIT(c, n)                                \
    do {                                                 \
        STATIC(TYPEOF_IS(c, struct clocks_t));           \
        STATIC(TYPEOF_IS(n, struct ntime_t*) ||          \
               TYPEOF_IS(n, const struct ntime_t*));     \
        if (CLOCK_TYPES_HAS(thread))                     \
            CLOCKS_TIME_INIT((c).thread, (n)->thread);   \
        if (CLOCK_TYPES_HAS(process))                    \
            CLOCKS_TIME_INIT((c).process, (n)->process); \
        if (CLOCK_TYPES_HAS(real))                       \
            CLOCKS_TIME_INIT((c).real, (n)->real);       \
        CLOCKS_INIT_CYCLES(c, n);                        \
        (c).types = (n)->types;                          \
    } while (0)

#define CLOCK_TYPES_HAS(n) \
    CLOCK_TYPES_HAS_(ntime->types, clock_type_ ## n)

static ALWAYS_INLINE
    void ntime_init(
        struct ntime_t* ntime,
        clock_types_t types)
{
    ntime->types = types;

    if (CLOCK_TYPES_HAS(real))
        clock_gettime(CLOCK_REALTIME, &ntime->real);
    if (CLOCK_TYPES_HAS(process))
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ntime->process);
    if (CLOCK_TYPES_HAS(thread))
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ntime->thread);
#ifdef CLOCK_CYCLES
    if (CLOCK_TYPES_HAS(cycles))
        CLOCK_GET_RDTSC_CYCLES(ntime->cycles);
#endif
}

static ALWAYS_INLINE
    struct clocks_t ntime_clocks(
        const struct ntime_t* ntime)
{
    struct clocks_t p, r;
    struct ntime_t t;

#ifdef CLOCK_CYCLES
    if (CLOCK_TYPES_HAS(cycles))
        CLOCK_GET_RDTSCP_CYCLES(t.cycles);
#endif
    if (CLOCK_TYPES_HAS(thread))
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t.thread);
    if (CLOCK_TYPES_HAS(process))
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t.process);
    if (CLOCK_TYPES_HAS(real))
        clock_gettime(CLOCK_REALTIME, &t.real);

    t.types = ntime->types;

    CLOCKS_INIT(p, ntime);
    CLOCKS_INIT(r, &t);
    clocks_sub(&r, &p);

    return r;
}

#endif // __CLOCKS_IMPL_H


