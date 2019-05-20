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
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include "common.h"
#include "int-traits.h"
#include "char-traits.h"
#include "ptr-traits.h"

#include "html-cref-unicode.h"

#define CHAR_IS_XDIGIT_(c)        \
    (                             \
        ((uchar_t) (c) >= '0'  && \
         (uchar_t) (c) <= '9') || \
        ((uchar_t) (c) >= 'a'  && \
         (uchar_t) (c) <= 'f') || \
        ((uchar_t) (c) >= 'A'  && \
         (uchar_t) (c) <= 'F')    \
    )
#define CHAR_IS_XDIGIT(c) CHAR_IS_(XDIGIT, c)

#define ISXDIGIT CHAR_IS_XDIGIT
#define ISDIGIT  CHAR_IS_DIGIT

// $ html-cref-gen --gen-cref-overrides --heading > html-cref-overrides-impl.h

static const uint16_t html_cref_overrides[] = {
#include "html-cref-overrides-impl.h"
};

// HTML Living Standard
// 12 The HTML syntax
// 12.1 Writing HTML documents
// 12.1.4 Character references
// https://html.spec.whatwg.org/multipage/syntax.html#character-references

// The numeric character reference forms described
// above are allowed to reference any *code point*
// excluding U+000D CR, *noncharacters*, and *controls*
// other than *ASCII whitespace*.

// [https://infra.spec.whatwg.org/#code-point]
// A *code point* is a Unicode code point and is
// represented as a four-to-six digit hexadecimal
// number, typically prefixed with "U+".

// [https://infra.spec.whatwg.org/#noncharacter]
// A *noncharacter* is a *code point* that is in the
// range U+FDD0 to U+FDEF, inclusive, or U+FFFE, U+FFFF,
// U+1FFFE, U+1FFFF, U+2FFFE, U+2FFFF, U+3FFFE, U+3FFFF,
// U+4FFFE, U+4FFFF, U+5FFFE, U+5FFFF, U+6FFFE, U+6FFFF,
// U+7FFFE, U+7FFFF, U+8FFFE, U+8FFFF, U+9FFFE, U+9FFFF,
// U+AFFFE, U+AFFFF, U+BFFFE, U+BFFFF, U+CFFFE, U+CFFFF,
// U+DFFFE, U+DFFFF, U+EFFFE, U+EFFFF, U+FFFFE, U+FFFFF,
// U+10FFFE, or U+10FFFF.

// [https://infra.spec.whatwg.org/#control]
// A *control* is a *C0 control* or a *code point*
// in the range U+007F to U+009F, inclusive. 

// [https://infra.spec.whatwg.org/#c0-control]
// A *C0 control* is a code point in the range
// U+0000 to U+001F, inclusive.

// [https://infra.spec.whatwg.org/#ascii-whitespace]
// *ASCII whitespace* is U+0009 TAB, U+000A LF,
// U+000C FF, U+000D CR, or U+0020 SPACE. 

bool html_cref_unicode_parse_html(
    const char** p, code_point_t* r)
{
    const char* q = *p;
    unsigned long v;
    int b;

    STATIC(UINT32_MAX <= ULONG_MAX);
    STATIC(html_cref_unicode_max_code_point <= UINT32_MAX);

    if (*q != 'x' && *q != 'X') {
        if (!ISDIGIT(*q))
            return false;
        b = 10;
    }
    else {
        q ++;
        if (!ISXDIGIT(*q)) {
            (*p) ++;
            return false;
        }
        b = 16;
    }

    errno = 0;
    v = strtoul(q, (char**) p, b);
    ASSERT(*p > q);

    if (errno || v > html_cref_unicode_max_code_point)
        return false;

    // not C0-control, with exception of
    // U+0009 TAB, U+000A LF, U+000C FF,
    // and U+0020 SPACE
    if (v <= 0x1fu &&
        v != '\t' &&
        v != '\r' &&
        v != '\f' &&
        v != ' ')
        return false; 

    // not in range U+007F to U+009F, inclusive, but
    // allow char reference overrides, according to
    // https://html.spec.whatwg.org/multipage/parsing.html
    // #numeric-character-reference-end-state
    if (v == 0x7fu)
        return false;
    if (v >= 0x80u && v <= 0x9fu) {
        STATIC(ARRAY_SIZE(html_cref_overrides) == 0x20u);

        if ((v = html_cref_overrides[v - 0x80u]))
            goto ret_code;

        return false;
    }

    // not in range U+FDD0 to U+FDEF, inclusive
    if (v >= 0xfdd0u && v <= 0xfdefu)
        return false;

    // not U+nFFFE and U+nFFFF, where n >= 0 && n <= 0x10
    if (((v & 0xffffu) == 0xfffeu ||
         (v & 0xffffu) == 0xffffu) &&
        ((v >> 0x10u) <= 0x10u))
        return false;

ret_code:
    *r = v;
    return true;
}

// stev: see the man page UTF-8(7)

// stev: see Table 3-6, UTF-8 Bit Distribution
// The Unicode Standard Version 8.0 - Core Specification, Chapter 3, p. 126
// http://www.unicode.org/versions/Unicode12.0.0/ch03.pdf

size_t html_cref_unicode_encode_utf8(
    code_point_t c, utf8_t u)
{
    uchar_t* p = u;

    STATIC(UINT32_MAX <= ULONG_MAX);
    STATIC(html_cref_unicode_max_code_point <= UINT32_MAX);

    ASSERT(c <= html_cref_unicode_max_code_point);
    ASSERT(c < 0xd800u || c > 0xdfffu);

    if (c < 0x80u)
        *p ++ = (uchar_t) c;  
    else
    if (c < 0x800u) {
        *p ++ = (uchar_t) ((c >> 6) | 0xc0u);
        *p ++ = (uchar_t) ((c & 0x3fu) | 0x80u);
    }
    else
    if (c < 0x10000u) {
        *p ++ = (uchar_t) ((c >> 12) | 0xe0u);
        *p ++ = (uchar_t) (((c >> 6) & 0x3fu) | 0x80u);
        *p ++ = (uchar_t) ((c & 0x3fu) | 0x80u);
    }
    else {
        *p ++ = (uchar_t) ((c >> 18) | 0xf0u);
        *p ++ = (uchar_t) (((c >> 12) & 0x3fu) | 0x80u);
        *p ++ = (uchar_t) (((c >> 6) & 0x3fu) | 0x80u);
        *p ++ = (uchar_t) ((c & 0x3fu) | 0x80u);
    }

    return PTR_DIFF(p, u);
}


