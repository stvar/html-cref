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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <dlfcn.h>

#include "dyn-lib.h"
#include "int-traits.h"
#include "char-traits.h"
#include "ptr-traits.h"
#include "common.h"

#define ISALNUM CHAR_IS_ALNUM
#define ISALPHA CHAR_IS_ALPHA

static struct dyn_lib_version_t get_lib_version(
    void* ver_func)
{
    typedef size_t (*ver_func_t)(void);

    enum { M = 10000, m = 100 };
    struct dyn_lib_version_t r;
    size_t v;

    v = ((ver_func_t) ver_func)();
    ASSERT(v > 0 && v < M * 10);

    r.major = v / M;
    v %= M;
    r.minor = v / m;
    r.patch = v % m;

    return r;
}

static size_t max_name_length(
    const struct dyn_lib_entry_t* ent, size_t n)
{
    const struct dyn_lib_entry_t* p = ent;
    size_t r = 0;

    for (ent += n; p < ent; p ++) {
        size_t l = strlen(p->name);
        if (r < l) r = l;
    }
    return r;
}

static char* format_msg(const char* fmt, ...)
    PRINTF(1);

static char* format_msg(const char* fmt, ...)
{
    char* b = NULL;
    size_t n = 0;
    va_list a;
    FILE* f;

    f = open_memstream(&b, &n);
    VERIFY(f != NULL);

    va_start(a, fmt);
    vfprintf(f, fmt, a);
    va_end(a);

    fclose(f);
    ASSERT(n > 0);

    return b;
}

#define ERROR_MSG(...)                          \
    do {                                        \
        if (err_msg != NULL) {                  \
            if (*err_msg) free(*err_msg);       \
            *err_msg = format_msg(__VA_ARGS__); \
        }                                       \
    } while (0)

#define DYN_LIB_ERROR(e)                      \
    ({                                        \
        err_info->type = dyn_lib_error_ ## e; \
        false;                                \
    })
#define DYN_LIB_ERROR_(e, n, v)               \
    ({                                        \
        err_info->type = dyn_lib_error_ ## e; \
        err_info->n = v;                      \
        false;                                \
    })
#define DYN_LIB_SYM_ERROR(v) \
    DYN_LIB_ERROR_(symbol_not_found, sym, v)
#define DYN_LIB_VER_ERROR(r) \
    DYN_LIB_ERROR_(wrong_lib_version, ver, r)

#define DYN_LIB_ENTRY_REF(p)                 \
    (                                        \
        (void**) ((char*) obj + (p)->offset) \
    )

bool dyn_lib_init(struct dyn_lib_t* dl,
    const char* lib, const struct dyn_lib_def_t* def,
    void* obj, struct dyn_lib_error_info_t* err_info,
    char** err_msg)
{
    enum { N = 1024 };

    const struct dyn_lib_entry_t *p, *e;
    size_t m, n, s = 0, u, v, w = 0;
    struct dyn_lib_version_t r;
    char b[N], c[N], *d;
    const char *f, *q;
    char *x, *y;

    ASSERT(def->n_entries > 0);

    if (err_msg) *err_msg = NULL;

    dl->lib_name = lib;
    dl->sobj = dlopen(lib, RTLD_NOW|RTLD_GLOBAL);

    if (dl->sobj == NULL) {
        ERROR_MSG("dlopen failed: %s", dlerror());
        return DYN_LIB_ERROR(load_lib_failed);
    }

    // stev: f != NULL => strlen(f) > 0
    f = def->prefix && *def->prefix
        ? def->prefix : NULL;

    m = max_name_length(
            def->entries, def->n_entries);
    if (f != NULL) {
        w = s = strlen(f);
        ASSERT_SIZE_INC_NO_OVERFLOW(w);
        w ++;
        ASSERT_SIZE_ADD_NO_OVERFLOW(m, w);
        m += w;
    }

    q = strrchr(dl->lib_name, '/');
    if (q == NULL)
        q = dl->lib_name;
    else
        q ++;

    n = strlen(q);
    if (n == 0 || n >= N)
        return DYN_LIB_ERROR(invalid_lib_name);

    memcpy(b, q, n + 1);
    d = strchr(b, '.');
    if (d != NULL) {
        size_t l = PTR_DIFF(b + n, d);
        ASSERT_SIZE_SUB_NO_OVERFLOW(n, l);
        n -= l;
        *d = 0;
    }

    // stev: in case 'b' has suffix "-" followed
    // by the string at 'f', cut that suffix off
    if (f != NULL && n >= SIZE_INC(s)) {
        char* q = b + (n - (s + 1));
        if (*q == '-' && !strcmp(q + 1, f)) {
            n -= s + 1;
            *q = 0;
        }
    }

    if (n == 0 || SIZE_ADD(n, m) >= N || !ISALPHA(*b))
        return DYN_LIB_ERROR(invalid_lib_name);

    for (x = b + 1, y = b + n; x < y; x ++) {
        if (!ISALNUM(*x) && *x != '-')
            return DYN_LIB_ERROR(invalid_lib_name);
        if (*x == '-') *x = '_';
    }
    n += w;

    for (p = def->entries,
         e = p + def->n_entries;
         p < e;
         p ++) {
        const char* e;
        void* s;
        int r;

        u = n + strlen(p->name) + 1;
        ASSERT(u < N);
        r = f != NULL
            ? snprintf(c, N, "%s_%s_%s",
                b, f, p->name)
            : snprintf(c, N, "%s_%s",
                b, p->name);
        v = INT_AS_SIZE(r);
        ASSERT(u == v);

        // stev: clear previous error conditions
        // (conforming to dlsym(3) man page)
        dlerror();

        s = dlsym(dl->sobj, c);
        e = dlerror();
        if (s == NULL || e != NULL) {
            if (e != NULL)
                ERROR_MSG("dlsym failed: %s", e);
            else
                ERROR_MSG("dlsym failed: symbol %s is NULL", c);
            return DYN_LIB_SYM_ERROR(PTR_DIFF(p, def->entries));
        }

        *DYN_LIB_ENTRY_REF(p) = s;
    }

    // stev: 'version' entry is always at 'def->entries[0]'
    r = get_lib_version(*DYN_LIB_ENTRY_REF(def->entries));

    if (r.major != def->ver_major ||
        r.minor != def->ver_minor) {
        ERROR_MSG("lib version: %d.%d.%d -- expected %zu.%zu",
            r.major, r.minor, r.patch,
            def->ver_major,
            def->ver_minor);
        return DYN_LIB_VER_ERROR(r);
    }

    return true;
}

void dyn_lib_done(struct dyn_lib_t* dl)
{
    if (dl->sobj != NULL)
        dlclose(dl->sobj);
}


