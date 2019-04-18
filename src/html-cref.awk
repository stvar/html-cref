#!/bin/awk -f

# Copyright (C) 2019  Stefan Vargyas
# 
# This file is part of Html-Cref.
# 
# Html-Cref is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Html-Cref is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Html-Cref.  If not, see <http://www.gnu.org/licenses/>.

function warning(s)
{
    printf("warning:%d: %s\n", FNR, s) > "/dev/stderr"
}

function error(s)
{
    printf("error:%d: %s\n", FNR, s) > "/dev/stderr"
    exit 1
}

function assert(v, m)
{
    if (!v) error(sprintf("assertion failed: %s", m))
}

BEGIN {
    if (act == "gen-sub-table") {
        n = split(opt, A, /,/)
        for (i = 1; i <= n; i ++)
            O[A[i]] = 1
        delete A
    }
    o = 0
}

function gen_sub_entry(n, v, l, w)
{
    printf("\t\"\\x%02x\" \"\\x%02x\" \"%s\"%*s// %s%s\n", \
        or(length(n), lshift(O[n], 7)), w, v, \
        25 - l, "", n, O[n] ? "*" : "")
}

function gen_map_entry(o)
{
    printf("%s %d\n", FNR - 1, o)
}

function gen_re2c_entry(n, o)
{
    printf("\t\"%s\" { return %d; }\n", n, o)
}

{
    n = $1
    v = $2

    e = 0
    l = length(v)
    if (v == "\\" ||
        v == "\"") {
        v = "\\" v
        l ++
        e ++
    }
    else
    if (substr(v, 1, 1) == "\\")
        e ++

    w = l >= 4 ? (l / 4 + l % 4) : l - e

    if (act == "gen-sub-table" &&
        length(n) + !O[n] + 1 < w && warn)
        warning(sprintf("length(n) + !O[n] + 1 >= w " \
            "[n=\"%s\" v=\"%s\" O[n]=%d w=%d]", \
            n, v, O[n], w))

    if (act == "gen-sub-table")
        gen_sub_entry(n, v, l, w)
    else
    if (act == "gen-map-table") {
        gen_map_entry(o)
        o += w + 2
    }
    else
    if (act == "gen-re2c-def") {
        gen_re2c_entry(n, o)
        o += w + 2
    }
    else
        assert(0, sprintf( \
            "invalid act='%s'", act))
}

BEGIN {
    if (act == "gen-re2c-def") {
        printf("/*!re2c\n")
        printf("\tre2c:define:YYCTYPE = char;\n")
        printf("\tre2c:yyfill:enable = 0;\n")
        printf("\tre2c:indent:top = 1;\n\n")
    }
}

END {
    if (act == "gen-re2c-def") {
        printf("\n\t* { return -1; }\n")
        printf("*/\n\n")
    }
}


