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

#ifndef __COMMON_H
#define __COMMON_H

#include <stdbool.h>

#ifndef __GNUC__
#error we need a GCC compiler
#elif __GNUC__ < 4
#error we need a GCC compiler of version >= 4
#endif

#define UNUSED    __attribute__((unused))
#define PRINTF(F) __attribute__((format(printf, F, F + 1)))
#define NORETURN  __attribute__((noreturn))

#define STRINGIFY_(s) #s
#define STRINGIFY(s)  STRINGIFY_(s)

#if defined(BUILTIN) && defined(TIMINGS)
#error cannot have both BUILTIN and TIMINGS parameters defined 
#endif

#ifndef BUILTIN
#define API      __attribute__ ((visibility("default")))
#define WEAK_API __attribute__ ((visibility("default"), weak))
#else // BUILTIN
#define API
#define WEAK_API
#endif // BUILTIN

// stev: important requirement: VERIFY evaluates E only once!

#define VERIFY(E)             \
    do {                      \
        if (!(E))             \
            UNEXPECT_ERR(#E); \
    }                         \
    while (0)

// stev: important requirement: ENSURE evaluates E only once!

#define ENSURE(E, M, ...)                     \
    do {                                      \
        if (!(E))                             \
            ensure_failed(__FILE__, __LINE__, \
                __func__, M, ## __VA_ARGS__); \
    }                                         \
    while (0)

#define UNEXPECT_ERR(M, ...)               \
    do {                                   \
        unexpect_error(__FILE__, __LINE__, \
            __func__, M, ## __VA_ARGS__);  \
    }                                      \
    while (0)

#define UNEXPECT_VAR(F, N) UNEXPECT_ERR(#N "=" F, N)

#ifdef DEBUG
# define ASSERT(E)                            \
    do {                                      \
        if (!(E))                             \
            assert_failed(__FILE__, __LINE__, \
                __func__, #E);                \
    }                                         \
    while (0)
#else
# define ASSERT(E) \
    do {} while (0)
#endif

#if !CONFIG_ERROR_FUNCTION_ATTRIBUTE
#error we need GCC to support the 'error' function attribute
#else
#define STATIC(E)                                   \
    ({                                              \
        extern int __attribute__                    \
            ((error("assertion failed: '" #E "'"))) \
            static_assert();                        \
        (void) ((E) ? 0 : static_assert());         \
    })
#endif

#if defined(BUILTIN)
#define MODULE_API
#elif defined(HTML_CREF_MODULE)
#define MODULE_API WEAK_API
#else
#define MODULE_API API
#endif

MODULE_API void error(const char* fmt, ...)
    PRINTF(1)
    NORETURN;

MODULE_API void ensure_failed(
    const char* file, int line, const char* func,
    const char* msg, ...)
    PRINTF(4)
    NORETURN;

MODULE_API void assert_failed(
    const char* file, int line, const char* func,
    const char* expr)
    NORETURN;

MODULE_API void unexpect_error(
    const char* file, int line, const char* func,
    const char* msg, ...)
    PRINTF(4)
    NORETURN;

#ifndef JSON_UNSIGNED_CHAR_DEFINED
#define JSON_UNSIGNED_CHAR_DEFINED
typedef unsigned char uchar_t;
#endif

#endif // __COMMON_H


