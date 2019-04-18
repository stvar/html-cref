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

function error(s)
{
    printf("error:%d: %s\n", FNR, s) > "/dev/stderr"
    exit 1
}

function assert(v, m)
{
    if (!v) error(sprintf("assertion failed: %s", m))
}

function print_lines(	i, l)
{
    for (i = 1; i <= N; i ++) {
        if (D[i])
            continue
        l = L[i]
        if (match(l, /^\t+/)) {
            l = sprintf("%*s", 4 * RLENGTH, "") \
                substr(l, RLENGTH + 1)
        }
        printf("%s\n", l)
    }
}

# 1{s/^/static /;s/(?<=\()/\n\t/}

function process_static_func(	a)
{
    L[1] = "static " L[1]
    if (match(L[1], /^(.*\()(.*)$/, a))
        L[1] = a[1] "\n\t" a[2]
}

# s/equal\(p,\s*(".*?")\)/prefix(\1, p)/

function process_subst_equal(	i, a)
{
    for (i = 1; i <= N; i ++) {
        if (match(L[i], /^(.*)equal\(p, (\x22[^\x22]+\x22)(\).*)$/, a))
            L[i] = a[1] "prefix(" a[2] ", p" a[3]
    }
}

# s/(?<=return )%d(?=;)/$$M[\0]$$/

function process_return_mapping(	i, a)
{
    for (i = 1; i <= N; i ++) {
        if (match(L[i], /^(.*return )([0-9]+)(;.*)$/, a))
            L[i] = a[1] M[a[2]] a[3]
    }
}

# s/(?<=\*p == )0\b/\x27;\x27/
# s/equal\(p, "(%d)"\)/strncmp(p, "\1;", $$length(\1) + 1$$)/

function process_semicolon(	i, a)
{
    for (i = 1; i <= N; i ++) {
        if (match(L[i], /^(.*\*p == )0([^0-9].*)$/, a))
            L[i] = a[1] "';'" a[2]
        else
        if (match(L[i], /^(.*)equal(\(p, \x22)([^\x22]+)\x22(\).*)$/, a))
            L[i] = a[1] "!strncmp" a[2] a[3] ";\", " (length(a[3]) + 1) a[4]
    }
}

# s/(\n\t+)if \(\*p == 0\)\1\t(return \d+;)((?:\1(?:if|switch|case|\t|\})[^\n]*)*)(?:\1return -1;)?/\3\1\2/g

function starts_with(a, b)
{ return substr(a, 1, length(b)) == b }

function process_star_p_eq_zero(	i, a, b, s, j, k, l)
{
    for (i = 1; i <= N; i ++) {
        if (D[i])
            continue
        if (match(L[i], /^(\t+)if \(\*p == 0\)$/, a) &&
            match(L[i + 1], /^(\t+)return ([0-9]+);$/, b) &&
            a[1] "\t" == b[1]) {
            s = sprintf("return -1; // %s", b[2])
            for (j = i + 2; j <= N; j ++) {
                if (!starts_with(L[j], a[1]))
                    break
            }
            if (j == i + 2)
                l = 0
            else {
                for (k = i; k < j - 2; k ++)
                    L[k] = L[k + 2]
                for (k = i; k < j - 2; k ++)
                     l = gsub(/return -1;/, s, L[k])
                l = l && starts_with(L[j - 3], a[1])
            }
            if (l) {
                D[j - 2] = 1
                D[j - 1] = 1
            }
            else {
                L[j - 2] = a[1] s
                D[j - 1] = 1
            }
        }
    }
    for (i = 1; i <= N; i ++) {
        if (D[i])
            continue
        if (match(L[i], /^(\t+return )-1; \/\/ ([0-9]+).*$/, a))
            L[i] = a[1] a[2] ";"
    }
}

# s/(\*p) \+\+( == \x27.\x27) \&\&\n\t+\*p == 0)(?=\n)/\1\2)/g

function process_star_p_plus_plus(	i, a)
{
    for (i = 1; i <= N; i ++) {
        if (D[i])
            continue
        if (match(L[i], /^(.*\*p) \+\+( == \x27.\x27) \&\&$/, a) &&
            match(L[i + 1], /^\t+\*p == 0\)$/)) {
            L[i] = a[1] a[2] ")"
            D[i + 1] = 1
        }
    }
}

function process_indexed_access(	i, a, k)
{
    for (i = 1; i <= N; i ++) {
        if (D[i])
            continue

        k = match(L[i], /^\t+/) ? RLENGTH : 0

        if (match(L[i], /^\t+p \+= ([0-9]+);$/, a)) {
            I[k] = I[k - 1] + a[1]
            D[i] = 1
        }
        else
        if (match(L[i], /^(\t+.*)\*p \+\+(.*)$/, a)) {
            L[i] = a[1] "p[" int(I[k]) "]" a[2]
            I[k + 1] = ++ I[k]
        }
        else
        if (match(L[i], /^(\t+.*\()\*p( == .*)$/, a)) {
            L[i] = a[1] "p[" int(I[k]) "]" a[2]
        }
        else
        if (match(L[i], /^(\t+)\*p( == (.*))$/, a)) {
            if (match(a[3], /^0[^0-9]/) &&
                !match(L[i + 1], /^\t+return [0-9]+;$/))
                assert(0, "`*p == 0' not followed by `return \\d;'")
            L[i] = a[1] "p[" int(I[k]) "]" a[2]
        }
        else
        if (match(L[i], /^(\t+if \(!?[a-zA-Z_][a-zA-Z0-9_]*\()p(, .*)$/, a)) {
            L[i] = a[1] "&p[" int(I[k]) "]" a[2]
        }
        else
        if (match(L[i], /^(\t+if \(!?[a-zA-Z_][a-zA-Z0-9_]*\(\x22.*\x22, )p(\).*)$/, a)) {
            L[i] = a[1] "&p[" int(I[k]) "]" a[2]
        }
        else
        if (match(L[i], /^\t+case /)) {
            I[k + 1] = I[k]
        }
    }
}

BEGIN {
    n = split(map, A, /,/)
    for (i = 1; i <= n; i ++) {
        k = split(A[i], a, /:/)
        assert(k == 2, "k == 2")
        M[a[1]] = a[2]
    }
    delete A
    delete a

    FS = "\n"
}

{
    L[++ N] = $0
}

END {
    if (static_func)
        process_static_func()

    if (length(M))
        process_return_mapping()

    if (!map_only) {
        if (semicolon)
            process_semicolon()
        else {
            process_star_p_eq_zero()
            process_star_p_plus_plus()
            process_subst_equal()
        }

        if (indexed_access)
            process_indexed_access()
    }

    print_lines()
}


