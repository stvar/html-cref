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

#ifndef __CHAR_TRAITS_H
#define __CHAR_TRAITS_H

#if !( \
    '\a' ==   7 && \
    '\b' ==   8 && \
    '\t' ==   9 && \
    '\n' ==  10 && \
    '\v' ==  11 && \
    '\f' ==  12 && \
    '\r' ==  13 && \
    ' '  ==  32 && '!'  ==  33 && '"'  ==  34 && '#'  ==  35 && \
    '$'  ==  36 && '%'  ==  37 && '&'  ==  38 && '\'' ==  39 && \
    '('  ==  40 && ')'  ==  41 && '*'  ==  42 && '+'  ==  43 && \
    ','  ==  44 && '-'  ==  45 && '.'  ==  46 && '/'  ==  47 && \
    '0'  ==  48 && '1'  ==  49 && '2'  ==  50 && '3'  ==  51 && \
    '4'  ==  52 && '5'  ==  53 && '6'  ==  54 && '7'  ==  55 && \
    '8'  ==  56 && '9'  ==  57 && ':'  ==  58 && ';'  ==  59 && \
    '<'  ==  60 && '='  ==  61 && '>'  ==  62 && '?'  ==  63 && \
    '@'  ==  64 && 'A'  ==  65 && 'B'  ==  66 && 'C'  ==  67 && \
    'D'  ==  68 && 'E'  ==  69 && 'F'  ==  70 && 'G'  ==  71 && \
    'H'  ==  72 && 'I'  ==  73 && 'J'  ==  74 && 'K'  ==  75 && \
    'L'  ==  76 && 'M'  ==  77 && 'N'  ==  78 && 'O'  ==  79 && \
    'P'  ==  80 && 'Q'  ==  81 && 'R'  ==  82 && 'S'  ==  83 && \
    'T'  ==  84 && 'U'  ==  85 && 'V'  ==  86 && 'W'  ==  87 && \
    'X'  ==  88 && 'Y'  ==  89 && 'Z'  ==  90 && '['  ==  91 && \
    '\\' ==  92 && ']'  ==  93 && '^'  ==  94 && '_'  ==  95 && \
    '`'  ==  96 && 'a'  ==  97 && 'b'  ==  98 && 'c'  ==  99 && \
    'd'  == 100 && 'e'  == 101 && 'f'  == 102 && 'g'  == 103 && \
    'h'  == 104 && 'i'  == 105 && 'j'  == 106 && 'k'  == 107 && \
    'l'  == 108 && 'm'  == 109 && 'n'  == 110 && 'o'  == 111 && \
    'p'  == 112 && 'q'  == 113 && 'r'  == 114 && 's'  == 115 && \
    't'  == 116 && 'u'  == 117 && 'v'  == 118 && 'w'  == 119 && \
    'x'  == 120 && 'y'  == 121 && 'z'  == 122 && '{'  == 123 && \
    '|'  == 124 && '}'  == 125 && '~'  == 126)
#error we need an execution charset extending ASCII
#endif

#define TYPEOF_IS_CHAR(c)     \
    (                         \
        TYPEOF_IS(c, char) || \
        TYPEOF_IS(c, uchar_t) \
    )

#define CHAR_IS_ASCII_(c)     \
    (                         \
        (uchar_t) (c) < 0x80u \
    )
#define CHAR_IS_CNTRL_(c)         \
    (                             \
        (uchar_t) (c) <= 0x1fu || \
        (uchar_t) (c) == 0x7fu    \
    )
#define CHAR_IS_SPACE_(c)        \
    (                            \
        (uchar_t) (c) == ' '  || \
        (uchar_t) (c) == '\t' || \
        (uchar_t) (c) == '\f' || \
        (uchar_t) (c) == '\n' || \
        (uchar_t) (c) == '\r' || \
        (uchar_t) (c) == '\v'    \
    )
#define CHAR_IS_DIGIT_(c)       \
    (                           \
        (uchar_t) (c) >= '0' && \
        (uchar_t) (c) <= '9'    \
    )
#define CHAR_IS_PRINT_(c)       \
    (                           \
        (uchar_t) (c) >= ' ' && \
        (uchar_t) (c) <= '~'    \
    )
#define CHAR_IS_ALPHA_(c)                    \
    (                                        \
        (((uchar_t) (c)) & ~0x20u) >= 'A' && \
        (((uchar_t) (c)) & ~0x20u) <= 'Z'    \
    )
#define CHAR_IS_LOWER_(c)           \
    (                               \
        (((uchar_t) (c))) >= 'a' && \
        (((uchar_t) (c))) <= 'z'    \
    )
#define CHAR_IS_UPPER_(c)           \
    (                               \
        (((uchar_t) (c))) >= 'A' && \
        (((uchar_t) (c))) <= 'Z'    \
    )
#define CHAR_IS_ALNUM_(c)    \
    (                        \
        CHAR_IS_ALPHA_(c) || \
        CHAR_IS_DIGIT_(c)    \
    )

#define CHAR_IS_(w, c)             \
    (                              \
        STATIC(TYPEOF_IS_CHAR(c)), \
        CHAR_IS_ ## w ## _(c)      \
    )

#define CHAR_IS_ASCII(c) CHAR_IS_(ASCII, c)
#define CHAR_IS_CNTRL(c) CHAR_IS_(CNTRL, c)
#define CHAR_IS_SPACE(c) CHAR_IS_(SPACE, c)
#define CHAR_IS_DIGIT(c) CHAR_IS_(DIGIT, c)
#define CHAR_IS_PRINT(c) CHAR_IS_(PRINT, c)
#define CHAR_IS_ALPHA(c) CHAR_IS_(ALPHA, c)
#define CHAR_IS_UPPER(c) CHAR_IS_(UPPER, c)
#define CHAR_IS_LOWER(c) CHAR_IS_(LOWER, c)
#define CHAR_IS_ALNUM(c) CHAR_IS_(ALNUM, c)

#endif/*__CHAR_TRAITS_H*/


