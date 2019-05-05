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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "su-size.h"
#include "int-traits.h"
#include "ptr-traits.h"
#include "char-traits.h"
#include "common.h"

#define ISDIGIT CHAR_IS_DIGIT

struct su_size_t su_size(size_t sz)
{
    struct su_size_t r;

    r.sz = sz;
    if (r.sz  % MB(1) == 0) {
        r.sz /= MB(1);
        r.su = "M";
    }
    else
    if (r.sz  % KB(1) == 0) {
        r.sz /= KB(1);
        r.su = "K";
    }
    else
        r.su = "";

    return r;
}

#if SIZE_MAX < ULONG_MAX
static size_t strtosz(
    const char* ptr, char** end, int base)
{
    unsigned long r;

    errno = 0;
    r = strtoul(ptr, end, base);

    if (errno == 0 && r > SIZE_MAX)
        errno = ERANGE;

    return r;
}
#define STR_TO_SIZE strtosz
#elif SIZE_MAX == ULONG_MAX
#define STR_TO_SIZE strtoul
#elif SIZE_MAX == UULONG_MAX
#define STR_TO_SIZE strtoull
#else
#error unexpected SIZE_MAX > UULONG_MAX
#endif

static size_t parse_num(
    const char* ptr, const char** end)
{
    if (!ISDIGIT(*ptr)) {
        *end = ptr;
        errno = EINVAL;
        return 0;
    }
    errno = 0;
    return STR_TO_SIZE(ptr, (char**) end, 10);
}

#define ERROR(n)                              \
    ({                                        \
        if (err != NULL)                      \
            *err = su_size_parse_error_ ## n; \
        false;                                \
    })

bool su_size_parse(
    const char* str, size_t min, size_t max,
    size_t* result, enum su_size_parse_error_t* err)
{
    const char *p, *q;
    size_t n, v, d;

    if (!(n = strlen(str)))
        return ERROR(invalid_num);

    v = parse_num(p = str, &q);
    d = PTR_OFFSET(q, p, n);

    if (errno || (d != n))
        return ERROR(invalid_num);

    if ((min > 0 && v < min) || (max > 0 && v > max))
        return ERROR(illegal_num);

    *result = v;
    return true;
}

bool su_size_parse_su(
    const char* str, size_t min, size_t max,
    size_t* result, enum su_size_parse_error_t* err)
{
    const char *p, *q;
    size_t n, v, d;

    if (!(n = strlen(str)))
        return ERROR(invalid_num);

    v = parse_num(p = str, &q);
    d = PTR_OFFSET(q, p, n);

    if (errno ||
        (d == 0) ||
        (d < n - 1) ||
        (d == n - 1 && *q != 'k' && *q != 'K' &&
            *q != 'm' && *q != 'M'))
        return ERROR(invalid_num);

    switch (*q) {
    case 'k':
    case 'K':
        STATIC(KB(1) <= SIZE_MAX);
        if (!SIZE_MUL_NO_OVERFLOW(v, KB(1)))
            return ERROR(illegal_num);
        v *= KB(1);
        break;

    case 'm':
    case 'M':
        STATIC(MB(1) <= SIZE_MAX);
        if (!SIZE_MUL_NO_OVERFLOW(v, MB(1)))
            return ERROR(illegal_num);
        v *= MB(1);
        break;
    }

    if ((min > 0 && v < min) || (max > 0 && v > max))
        return ERROR(illegal_num);

    *result = v;
    return true;
}

#undef  ERROR
#define ERROR(n, p)                                \
    ({                                             \
        if (err != NULL)                           \
            *err = su_size_parse_list_error_ ## n; \
        if (ptr != NULL)                           \
            *ptr = p;                              \
        false;                                     \
    })

bool su_size_parse_list(
    const char* str, size_t min, size_t max,
    char sep, size_t* list, size_t n_list,
    enum su_size_parse_list_error_t* err,
    const char** ptr, size_t* n_elems)
{
    const char *p = str, *q;
    size_t r = 0, v;

    ASSERT(sep != 0);

    do {
        v = parse_num(p, &q);
        ASSERT(p <= q);

        if (errno || q == p)
            return ERROR(invalid_num, p);

        if ((*q && *q != sep) ||
            (*q == sep && *++ q == 0))
            return ERROR(invalid_list, q);

        if ((min > 0 && v < min) || (max > 0 && v > max))
            return ERROR(illegal_num, p);

        if (++ r > n_list)
            return ERROR(list_overflow, p);

        *list ++ = v;
    } while (*(p = q));

    *n_elems = r;
    return true;
}

static size_t su_size_to_null(
    char* buf, size_t len)
{
    ASSERT(len >= 2);
    strncpy(buf, "-", 2);
    return 1;
}

size_t su_size_to_string(
    char* buf, size_t len, size_t val, bool null)
{
    int n;

    if (val == 0 && null)
        return su_size_to_null(buf, len);

    n = snprintf(buf, len, "%zu", val);
    VERIFY(n >= 0);

    return INT_AS_SIZE(n);
}

size_t su_size_to_string_su(
    char* buf, size_t len, size_t val, bool null)
{
    struct su_size_t s;
    int n;

    if (val == 0 && null)
        return su_size_to_null(buf, len);

    s = su_size(val);
    n = snprintf(buf, len, "%zu%s", s.sz, s.su);
    VERIFY(n >= 0);

    return INT_AS_SIZE(n);
}

size_t su_size_list_to_string(
    char* buf, size_t len, const size_t* list,
    size_t n_list, const char* sep, bool null)
{
    const size_t *p, *e;
    size_t r, l = len;
    char *q = buf;
    int n;

    if (n_list == 0) {
        if (null)
            return su_size_to_null(buf, len);
        else {
            ASSERT(len > 0);
            *buf = 0;
            return 0;
        }
    }

    for (p = list,
         e = p + n_list;
         p < e;
         p ++) {
        n = snprintf(q, l, "%s%zu",
                p > list ? sep : "", *p);
        VERIFY(n >= 0);

        r = INT_AS_SIZE(n);
        if (r >= l) return len;

        q += r;
        l -= r;
    }
    return len - l;
}

void su_size_print_sizes(
    const struct su_size_param_t* params,
    size_t n_params,
    FILE* file)
{
    static const char fmt[] =
        "%-*s  %*s  %*s  %*s  %s\n";
    typedef size_t (*to_string_func_t)(
        char*, size_t, size_t, bool);
    static const to_string_func_t to_strings[] = {
        [su_size_param_plain] = su_size_to_string,
        [su_size_param_su] = su_size_to_string_su,
    };
    const size_t m = TYPE_DIGITS10(size_t) + 1;
    struct values_t
    {
        char min[m];
        char max[m];
        char def[m];
    };
    const size_t n = n_params;
    struct values_t values[n];
    struct max_widths_t
    {
        size_t name;
        size_t min;
        size_t max;
        size_t def;
    };
    struct max_widths_t w = {
        .name = 4,
        .min = 3,
        .max = 3,
        .def = 7,
    };
    const struct su_size_param_t *p, *e;
    struct values_t *v, *f;

    memset(values, 0, sizeof(values));

    for (p = params,
         v = values,
         e = params + n,
         f = values + n;
         p < e && v < f;
         p ++,
         v ++) {
        to_string_func_t f =
            ARRAY_NULL_ELEM(to_strings, p->type);
        size_t l;

        ASSERT(f != NULL);
        f(v->min, m, p->min, true);
        f(v->max, m, p->max, true);
        f(v->def, m, p->def, false);

        l = strlen(p->name);
        if (w.name < l)
            w.name = l;

        l = strlen(v->min);
        if (w.min < l)
            w.min = l;

        l = strlen(v->max);
        if (w.max < l)
            w.max = l;

        l = strlen(v->def);
        if (w.def < l)
            w.def = l;
    }

    fprintf(file, fmt,
        SIZE_AS_INT(w.name), "name",
        SIZE_AS_INT(w.min), "min",
        SIZE_AS_INT(w.max), "max",
        SIZE_AS_INT(w.def), "default",
        "description");

    for (p = params,
         v = values,
         e = params + n,
         f = values + n;
         p < e && v < f;
         p ++,
         v ++) {
        fprintf(file, fmt,
            SIZE_AS_INT(w.name), p->name,
            SIZE_AS_INT(w.min), v->min,
            SIZE_AS_INT(w.max), v->max,
            SIZE_AS_INT(w.def), v->def,
            p->desc);
    }
}


