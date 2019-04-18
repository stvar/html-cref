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

#ifndef __PRETTY_PRINT_H
#define __PRETTY_PRINT_H

#include <stdio.h>

#include "json-defs.h"
#include "va-args.h"

enum {
    pretty_print_char_quotes   = 1U << 0,
    pretty_print_string_quotes = 1U << 1,
};

JSON_API size_t pretty_print_len(char ch, size_t flags);
JSON_API const char* pretty_print_fmt(char ch, size_t flags);

JSON_API size_t pretty_print_char(
    FILE* file, char ch, size_t flags);
JSON_API size_t pretty_print_string(
    FILE* file, const uchar_t* str, size_t len, size_t flags);
JSON_API size_t pretty_print_strings(
    FILE* file, const char* const* strs, size_t n_strs,
    const char* name, size_t width, size_t flags);

enum pretty_print_repr_flags_t {
    pretty_print_repr_quote_non_print = 1U << 0,
    pretty_print_repr_print_quotes    = 1U << 1,
    pretty_print_repr_escape_utf8     = 1U << 2,
    pretty_print_repr_surrogate_pairs = 1U << 3,
};

JSON_API void pretty_print_repr(
    FILE* file, const uchar_t* str, size_t len,
    enum pretty_print_repr_flags_t flags);

#define PRETTY_PRINT_REPR_FLAGS_(n) \
    pretty_print_repr_ ## n
#define PRETTY_PRINT_REPR_FLAGS(...) \
    (VA_ARGS_REPEAT(|, PRETTY_PRINT_REPR_FLAGS_, __VA_ARGS__))

#define PRETTY_PRINT_REPR_FLAGS_IS__(n) \
    (flags & PRETTY_PRINT_REPR_FLAGS_(n))
#define PRETTY_PRINT_REPR_FLAGS_IS_(o, ...) \
    (VA_ARGS_REPEAT(o, PRETTY_PRINT_REPR_FLAGS_IS__, __VA_ARGS__))
#define PRETTY_PRINT_REPR_FLAGS_IS_OR(...) \
    PRETTY_PRINT_REPR_FLAGS_IS_(||, __VA_ARGS__)
#define PRETTY_PRINT_REPR_FLAGS_IS_AND(...) \
    PRETTY_PRINT_REPR_FLAGS_IS_(&&, __VA_ARGS__)
#define PRETTY_PRINT_REPR_FLAGS_IS \
    PRETTY_PRINT_REPR_FLAGS_IS_AND

#endif/*__PRETTY_PRINT_H*/


