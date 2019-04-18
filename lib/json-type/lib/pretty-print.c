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
#include <ctype.h>
#include <string.h>
#include <stddef.h>

#include "common.h"
#include "pretty-print.h"
#include "int-traits.h"
#include "char-traits.h"
#include "ptr-traits.h"
#include "json-utf8.h"

#define ISPRINT CHAR_IS_PRINT

size_t pretty_print_len(char ch, size_t flags)
{
    ASSERT(
        !(flags & pretty_print_char_quotes) ||
        !(flags & pretty_print_string_quotes));

    switch (ch) {
    case '\0':
    case '\a':
    case '\b':
    case '\t':
    case '\n':
    case '\v':
    case '\f':
    case '\r':
    case '\\':
        return 2;
    case '\'':
    case '"':
        return flags
            & (pretty_print_char_quotes|pretty_print_string_quotes)
            ? 2
            : 1;
    default:
        return !ISPRINT(ch)
            ? 4
            : 1;
    }
}

const char* pretty_print_fmt(char ch, size_t flags)
{
    static const char* plain_char = "%c";
    static const char* quoted_char = "\\%c";
    static const char* quoted_hex = "\\x%02x";

    ASSERT(
        !(flags & pretty_print_char_quotes) ||
        !(flags & pretty_print_string_quotes));

    switch (ch) {
    case '\0':
        return "\\0";
    case '\a':
        return "\\a";
    case '\b':
        return "\\b";
    case '\t':
        return "\\t";
    case '\n':
        return "\\n";
    case '\v':
        return "\\v";
    case '\f':
        return "\\f";
    case '\r':
        return "\\r";
    case '\\':
        return quoted_char;
    case '\'':
        return flags & pretty_print_char_quotes
            ? quoted_char
            : plain_char;
    case '"':
        return flags & pretty_print_string_quotes
            ? quoted_char
            : plain_char;
    default:
        return !ISPRINT(ch)
            ? quoted_hex
            : plain_char;
    }
}

size_t pretty_print_char(FILE* file, char ch, size_t flags)
{
    size_t r;
    const bool q = flags & pretty_print_char_quotes;
    int n;

    if (q)
        fputc('\'', file);

    n = fprintf(file, pretty_print_fmt(ch, flags), (uchar_t) ch);
    r = VERIFY_INT_AS_SIZE(n);

    if (q) {
        fputc('\'', file);
        SIZE_ADD_EQ(r, SZ(2));
    }

    return r;
}

size_t pretty_print_string(
    FILE* file, const uchar_t* str, size_t len, size_t flags)
{
    size_t r = 0;
    const bool q = flags & pretty_print_string_quotes;

    if (q)
        fputc('"', file);

    while (len --) {
        uchar_t c = *str ++;
        size_t s;
        int n;

        n = fprintf(file, pretty_print_fmt(c, flags), c);
        s = VERIFY_INT_AS_SIZE(n);

        SIZE_ADD_EQ(r, s);
    }

    if (q) {
        fputc('"', file);
        SIZE_ADD_EQ(r, SZ(2));
    }

    return r;
}

size_t pretty_print_strings(
    FILE* file, const char* const* strs, size_t n_strs,
    const char* name, size_t width, size_t flags)
{
    size_t i, r;

    if (n_strs == 0) {
        size_t l = strlen(name);
        int c;

        ASSERT_SIZE_INC_NO_OVERFLOW(l);

        c = fprintf(file,
                "%s:%*s-\n", name, width > l + 1
                ? SIZE_AS_INT(width - (l + 1)) : 0, "");

        return VERIFY_INT_AS_SIZE(c);
    }

    for (r = 0,
         i = 0;
         i < n_strs;
         i ++, strs ++) {
        size_t n;
        int c;

        c = fprintf(file, "%s[%zu]:", name, i);
        n = VERIFY_INT_AS_SIZE(c);
        SIZE_ADD_EQ(r, n);

        if (width > n) {
            c = fprintf(file, "%*s",
                    SIZE_AS_INT(width - n), "");
            n = VERIFY_INT_AS_SIZE(c);
            SIZE_ADD_EQ(r, n);
        }

        n = pretty_print_string(file,
                PTR_UCHAR_CAST_CONST(*strs),
                strlen(*strs), flags);
        SIZE_ADD_EQ(r, n);

        c = fputc('\n', file);
        VERIFY(c >= 0);

        SIZE_PRE_INC(r);
    }
    return r;
}

static void pretty_print_repr_base(
    FILE* file, const uchar_t* str, size_t len,
    bool quote_non_print)
{
    while (len --) {
        uchar_t c = *str ++;

        switch (c) {
        case '"':
            fputs("\\\"", file);
            break;
        case '\\':
            fputs("\\\\", file);
            break;
        case '\b':
            fputs("\\b", file);
            break;
        case '\f':
            fputs("\\f", file);
            break;
        case '\n':
            fputs("\\n", file);
            break;
        case '\r':
            fputs("\\r", file);
            break;
        case '\t':
            fputs("\\t", file);
            break;
        default:
            if (quote_non_print && !ISPRINT(c))
                fprintf(file, "\\x%02x", c);
            else
                fputc(c, file);
        }
    }
}

void pretty_print_repr(
    FILE* file, const uchar_t* str, size_t len,
    enum pretty_print_repr_flags_t flags)
{
    if (PRETTY_PRINT_REPR_FLAGS_IS(print_quotes))
        fputc('"', file);

    if (PRETTY_PRINT_REPR_FLAGS_IS(escape_utf8))
        json_print_escaped_utf8(str, len,
            PRETTY_PRINT_REPR_FLAGS_IS(surrogate_pairs), file);
    else
        pretty_print_repr_base(file, str, len,
            PRETTY_PRINT_REPR_FLAGS_IS(quote_non_print));

    if (PRETTY_PRINT_REPR_FLAGS_IS(print_quotes))
        fputc('"', file);
}


