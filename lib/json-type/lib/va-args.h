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

// From: Laurent Deniau <laurent...@cern.ch>
// Newsgroups: comp.std.c
// Subject: __VA_NARG__
// Date: Mon, 16 Jan 2006 18:43:40 +0100
// https://groups.google.com/forum/message/raw?msg=comp.std.c/d-6Mj5Lko_s/5R6bMWTEbzQJ

#define VA_ARGS_NARGS___() \
    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define VA_ARGS_NARGS__(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) \
    N
#define VA_ARGS_NARGS_(...) \
    VA_ARGS_NARGS__(__VA_ARGS__)
#define VA_ARGS_NARGS(...) \
    VA_ARGS_NARGS_(_, ## __VA_ARGS__, \
        VA_ARGS_NARGS___())

#define VA_ARGS_SELECT_0(_0, ...) _0
#define VA_ARGS_SELECT_1(_0, _1, ...) _1
#define VA_ARGS_SELECT_2(_0, _1, _2, ...) _2
#define VA_ARGS_SELECT_3(_0, _1, _2, _3, ...) _3
#define VA_ARGS_SELECT_4(_0, _1, _2, _3, _4, ...) _4
#define VA_ARGS_SELECT_5(_0, _1, _2, _3, _4, _5, ...) _5
#define VA_ARGS_SELECT_6(_0, _1, _2, _3, _4, _5, _6, ...) _6
#define VA_ARGS_SELECT_7(_0, _1, _2, _3, _4, _5, _6, _7, ...) _7
#define VA_ARGS_SELECT_8(_0, _1, _2, _3, _4, _5, _6, _7, _8, ...) _8
#define VA_ARGS_SELECT_9(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, ...) _9
#define VA_ARGS_SELECT_10(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, ...) _10
#define VA_ARGS_SELECT_11(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, ...) _11
#define VA_ARGS_SELECT_12(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, ...) _12
#define VA_ARGS_SELECT_13(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, ...) _13
#define VA_ARGS_SELECT_14(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, ...) _14
#define VA_ARGS_SELECT_15(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define VA_ARGS_SELECT_16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, ...) _16

#define VA_ARGS_SELECT(i, ...) \
    VA_ARGS_SELECT_ ## i(__VA_ARGS__)

#define VA_ARGS_APPLY(m, ...) \
    m(__VA_ARGS__)
#define VA_ARGS_APPLY_NTH(m, n, ...) \
    VA_ARGS_APPLY(m, VA_ARGS_SELECT(n, __VA_ARGS__))
#define VA_ARGS_APPLY_NTH_N(m, n, ...) \
    VA_ARGS_APPLY(m, n, VA_ARGS_SELECT(n, __VA_ARGS__))

#define VA_ARGS_REPEAT_0(a, s, m, ...)
#define VA_ARGS_REPEAT_1(a, s, m, ...) \
    VA_ARGS_REPEAT_0(a, s, m, __VA_ARGS__) a(m, 0, __VA_ARGS__)
#define VA_ARGS_REPEAT_2(a, s, m, ...) \
    VA_ARGS_REPEAT_1(a, s, m, __VA_ARGS__) s a(m, 1, __VA_ARGS__)
#define VA_ARGS_REPEAT_3(a, s, m, ...) \
    VA_ARGS_REPEAT_2(a, s, m, __VA_ARGS__) s a(m, 2, __VA_ARGS__)
#define VA_ARGS_REPEAT_4(a, s, m, ...) \
    VA_ARGS_REPEAT_3(a, s, m, __VA_ARGS__) s a(m, 3, __VA_ARGS__)
#define VA_ARGS_REPEAT_5(a, s, m, ...) \
    VA_ARGS_REPEAT_4(a, s, m, __VA_ARGS__) s a(m, 4, __VA_ARGS__)
#define VA_ARGS_REPEAT_6(a, s, m, ...) \
    VA_ARGS_REPEAT_5(a, s, m, __VA_ARGS__) s a(m, 5, __VA_ARGS__)
#define VA_ARGS_REPEAT_7(a, s, m, ...) \
    VA_ARGS_REPEAT_6(a, s, m, __VA_ARGS__) s a(m, 6, __VA_ARGS__)
#define VA_ARGS_REPEAT_8(a, s, m, ...) \
    VA_ARGS_REPEAT_7(a, s, m, __VA_ARGS__) s a(m, 7, __VA_ARGS__)
#define VA_ARGS_REPEAT_9(a, s, m, ...) \
    VA_ARGS_REPEAT_8(a, s, m, __VA_ARGS__) s a(m, 8, __VA_ARGS__)
#define VA_ARGS_REPEAT_10(a, s, m, ...) \
    VA_ARGS_REPEAT_9(a, s, m, __VA_ARGS__) s a(m, 9, __VA_ARGS__)
#define VA_ARGS_REPEAT_11(a, s, m, ...) \
    VA_ARGS_REPEAT_10(a, s, m, __VA_ARGS__) s a(m, 10, __VA_ARGS__)
#define VA_ARGS_REPEAT_12(a, s, m, ...) \
    VA_ARGS_REPEAT_11(a, s, m, __VA_ARGS__) s a(m, 11, __VA_ARGS__)
#define VA_ARGS_REPEAT_13(a, s, m, ...) \
    VA_ARGS_REPEAT_12(a, s, m, __VA_ARGS__) s a(m, 12, __VA_ARGS__)
#define VA_ARGS_REPEAT_14(a, s, m, ...) \
    VA_ARGS_REPEAT_13(a, s, m, __VA_ARGS__) s a(m, 13, __VA_ARGS__)
#define VA_ARGS_REPEAT_15(a, s, m, ...) \
    VA_ARGS_REPEAT_14(a, s, m, __VA_ARGS__) s a(m, 14, __VA_ARGS__)
#define VA_ARGS_REPEAT_16(a, s, m, ...) \
    VA_ARGS_REPEAT_15(a, s, m, __VA_ARGS__) s a(m, 15, __VA_ARGS__)

#define VA_ARGS_REPEAT__(n, a, s, m, ...) \
    VA_ARGS_REPEAT_ ## n(a, s, m, __VA_ARGS__)
#define VA_ARGS_REPEAT_(n, a, s, m, ...) \
    VA_ARGS_REPEAT__(n, a, s, m, __VA_ARGS__)

#define VA_ARGS_REPEAT(s, m, ...)   \
    VA_ARGS_REPEAT_(                \
        VA_ARGS_NARGS(__VA_ARGS__), \
        VA_ARGS_APPLY_NTH,          \
        s, m, __VA_ARGS__)

#define VA_ARGS_REPEAT_N(s, m, ...) \
    VA_ARGS_REPEAT_(                \
        VA_ARGS_NARGS(__VA_ARGS__), \
        VA_ARGS_APPLY_NTH_N,        \
        s, m, __VA_ARGS__)

#define VA_ARGS_IS_EMPTY(...) \
    VA_ARGS_SELECT_16(_, ## __VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)

#define VA_ARGS_SELECT_BOOL_0(f, t) f
#define VA_ARGS_SELECT_BOOL_1(f, t) t
#define VA_ARGS_SELECT_BOOL_(b, f, t) \
    VA_ARGS_SELECT_BOOL_ ## b(f, t)
#define VA_ARGS_SELECT_BOOL(b, f, t) \
    VA_ARGS_SELECT_BOOL_(b, f, t)

#define VA_ARGS_REPEAT_N_EMPTY(e, s, m, ...) \
    VA_ARGS_SELECT_BOOL(                     \
        VA_ARGS_IS_EMPTY(__VA_ARGS__),       \
        e, VA_ARGS_REPEAT_N(                 \
            s, m, ## __VA_ARGS__) )


