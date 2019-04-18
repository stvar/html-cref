#!/bin/bash

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

html-cref()
{ LD_LIBRARY_PATH=../src ../src/html-cref -t sponge "$@"; }

names()
{
    if [ "$1" == '-r' ]; then
        sed -r 's/^/\&/;s/$/;/' html-cref-names.txt|sort
    else
        sort html-cref-names.txt
    fi
}

prefixes()
{
    local r=0
    [ "$1" == '-r' ] && r=1

    awk -F '\n' -v r=$r '{
        n = length($1)
        for (i = 1; i < n; i ++)
            printf(r ? "&%s;\n" : "%s\n", substr($1,1,i))
    }' \
    html-cref-names.txt|
    sort -u
}

strict-prefixes()
{
    local r=''
    [ "$1" == '-r' ] && r='-r'

    join -v2 <(names $r) <(prefixes $r)
}

clear-output()
{
    local b=''
    local e=''

    [ "$1" == '-r' ] && {
        b='\&'
        e=';'
    }

    sed -r 's/^html-cref: warning: invalid char reference \x27([^\x27]+)\x27$/'"$b"'\1'"$e"'/'
}

validate-names()
{
    local a='P'
    local r=''

    [ "$1" == '-r' ] && {
        a='S'
        r='-r'
        shift
    }

    names $r|
    html-cref ${1:+-p "$1"} -$a -w|
    clear-output $r
}

validate-strict-prefixes()
{
    local a='P'
    local r=''

    [ "$1" == '-r' ] && {
        a='S'
        r='-r'
        shift
    }

    diff -u0 \
-Lold <(strict-prefixes $r) \
-Lnew <(strict-prefixes $r|html-cref ${1:+-p "$1"} -$a -e -w|clear-output $r)
}

test-crefs()
{ html-cref ${1:+-p "$1"} -S -w test-html-crefs.txt; }


