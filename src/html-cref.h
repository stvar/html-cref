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

#ifndef __HTML_CREF_H
#define __HTML_CREF_H

#ifdef HTML_CREF_MODULE
#include <stdbool.h>
#endif

#define HTML_CREF_VERSION_MAJOR 0
#define HTML_CREF_VERSION_MINOR 1
#define HTML_CREF_VERSION_PATCH 0

#define HTML_CREF_VERSION                 \
    (                                     \
        HTML_CREF_VERSION_MAJOR * 10000 + \
        HTML_CREF_VERSION_MINOR * 100 +   \
        HTML_CREF_VERSION_PATCH           \
    )

#define API_NAME(m, n)     html_cref_ ## m ## _ ## n

#define API_STRING__(n)    #n
#define API_STRING_(n)     API_STRING__(n)
#define API_STRING(m, f)   API_STRING_(API_NAME(m, f))

#define API_ALIAS(m, f, a) \
    TYPEOF(API_NAME(m, f)) API_NAME(m, a) \
    __attribute__ ((alias (API_STRING(m, f))));

#ifdef TIMINGS

#include "clocks.h"

extern MODULE_API struct clocks_t clocks;

#define HTML_CREF_FUNC_DEF(t, n) \
int html_cref_ ## t ##           \
             _ ## n ## 2(        \
    const char* p)               \
{                                \
    struct clocks_t c;           \
    struct ntime_t s;            \
    int r;                       \
                                 \
    ntime_init(&s);              \
    r = html_cref_ ## t ##       \
                 _ ## n(p);      \
    c = ntime_clocks(&s);        \
    clocks_add(&clocks, &c);     \
                                 \
    return r;                    \
}

#endif // TIMINGS

#ifdef HTML_CREF_MODULE

static inline bool prefix(
    const char* p, const char* q)
{
    while (*p && *p == *q)
         ++ p, ++ q;
    return *p == 0;
}

#endif // HTML_CREF_MODULE

enum {
    // $ html-cref-gen --cref-names-min-max
    html_cref_max_name_len = 31,
    html_cref_min_name_len = 2,
};

#endif /* __HTML_CREF_H */


