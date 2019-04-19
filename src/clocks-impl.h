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

#define CLOCKS_MAX_NSECS SZ(999999999)
#define CLOCKS_LIM_NSECS SZ(1000000000)

#define ALWAYS_INLINE inline __attribute__((always_inline))

static ALWAYS_INLINE void
    clocks_time_init(
        struct clocks_time_t* time,
        const struct timespec* timespec)
{
    time->secs = INT_AS_SIZE(timespec->tv_sec),
    time->nsecs = INT_AS_SIZE(timespec->tv_nsec);

    if (time->nsecs >= CLOCKS_LIM_NSECS) {
        size_t q = time->nsecs / CLOCKS_LIM_NSECS;

        time->nsecs %= CLOCKS_LIM_NSECS;
        ASSERT_SIZE_ADD_NO_OVERFLOW(
            time->secs, q);
        time->secs += q;
    }
}

static ALWAYS_INLINE void
    clocks_time_assign(
        struct clocks_time_t* time,
        const struct clocks_time_t* time2)
{
    ASSERT(time2->nsecs <= CLOCKS_MAX_NSECS);

    time->secs = time2->secs;
    time->nsecs = time2->nsecs;
}

static ALWAYS_INLINE void
    clocks_time_add(
        struct clocks_time_t* time,
        const struct clocks_time_t* time2)
{
    ASSERT(time->nsecs <= CLOCKS_MAX_NSECS);
    ASSERT(time2->nsecs <= CLOCKS_MAX_NSECS);

    // (0): L := CLOCKS_LIM_NSECS;
    // (1): nsecs, nsecs2 < L =>
    //      nsecs + nsecs2 < 2 * L
    // (2): nsecs + nsecs2 >= L =>
    //      (nsecs + nsecs2) % L =
    //      (nsecs + nsecs2) - L:
    //      by 0 < b <= a < 2 * b => a % b = a - b

    SIZE_ADD_EQ(time->secs, time2->secs);
    SIZE_ADD_EQ(time->nsecs, time2->nsecs);

    if (time->nsecs >= CLOCKS_LIM_NSECS) {
        time->nsecs -= CLOCKS_LIM_NSECS;
        ASSERT_SIZE_INC_NO_OVERFLOW(
            time->secs);
        time->secs ++;
    }
}

static ALWAYS_INLINE void
    clocks_time_sub(
        struct clocks_time_t* time,
        const struct clocks_time_t* time2)
{
    ASSERT(time->nsecs <= CLOCKS_MAX_NSECS);
    ASSERT(time2->nsecs <= CLOCKS_MAX_NSECS);

    SIZE_SUB_EQ(time->secs, time2->secs);

    if (time->nsecs < time2->nsecs) {
        // time->nsecs + CLOCKS_MAX_NSECS
        //   >= CLOCKS_MAX_NSECS
        //   >= time2->nsecs

        ASSERT_SIZE_ADD_NO_OVERFLOW(
            time->nsecs, CLOCKS_MAX_NSECS);
        time->nsecs += CLOCKS_MAX_NSECS;

        ASSERT_SIZE_DEC_NO_OVERFLOW(time->secs);
        time->secs --;
    }

    SIZE_SUB_EQ(time->nsecs, time2->nsecs);
}

struct ntime_t
{
    struct clocks_time_t rtime;
    struct clocks_time_t ptime;
    struct clocks_time_t ttime;
};

static ALWAYS_INLINE void
    ntime_init(
        struct ntime_t* ntime)
{
    struct timespec r, p, t;

    clock_gettime(CLOCK_REALTIME, &r);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &p);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t);

    clocks_time_init(&ntime->rtime, &r);
    clocks_time_init(&ntime->ptime, &p);
    clocks_time_init(&ntime->ttime, &t);
}

static ALWAYS_INLINE struct clocks_t
    ntime_clocks(
        const struct ntime_t* time)
{
    struct clocks_t r;
    struct ntime_t t;

    ntime_init(&t);

    clocks_time_assign(&r.real, &t.rtime);
    clocks_time_sub(&r.real, &time->rtime);

    clocks_time_assign(&r.process, &t.ptime);
    clocks_time_sub(&r.process, &time->ptime);

    clocks_time_assign(&r.thread, &t.ttime);
    clocks_time_sub(&r.thread, &time->ttime);

    return r;
}

#endif // __CLOCKS_IMPL_H


