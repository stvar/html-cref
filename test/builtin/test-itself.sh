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

#
# File generated by a command like:
# $ html-cref-gentest -C builtin:itself
#

[[ "$1" =~ ^-u[0-9]+$ ]] &&
u="${1:2}" ||
u=""

diff -u$u -L itself.old <(echo \
'$ shopt -s extglob
$ . test-funcs.sh
$ is-builtin() { html-cref --help|{ grep -Fqe '\''-p|--cref-parser='\'' && echo no || true; }; }
$ is-builtin
$ validate-names
$ validate-names -r
$ validate-strict-prefixes
$ validate-strict-prefixes -r
$ test-crefs'
) -L itself.new <(
echo '$ shopt -s extglob'
shopt -s extglob 2>&1 ||
echo 'command failed: shopt -s extglob'

echo '$ . test-funcs.sh'
. test-funcs.sh 2>&1 ||
echo 'command failed: . test-funcs.sh'

echo '$ is-builtin() { html-cref --help|{ grep -Fqe '\''-p|--cref-parser='\'' && echo no || true; }; }'
is-builtin() { html-cref --help|{ grep -Fqe '-p|--cref-parser=' && echo no || true; }; } 2>&1 ||
echo 'command failed: is-builtin() { html-cref --help|{ grep -Fqe '\''-p|--cref-parser='\'' && echo no || true; }; }'

echo '$ is-builtin'
is-builtin 2>&1 ||
echo 'command failed: is-builtin'

echo '$ validate-names'
validate-names 2>&1 ||
echo 'command failed: validate-names'

echo '$ validate-names -r'
validate-names -r 2>&1 ||
echo 'command failed: validate-names -r'

echo '$ validate-strict-prefixes'
validate-strict-prefixes 2>&1 ||
echo 'command failed: validate-strict-prefixes'

echo '$ validate-strict-prefixes -r'
validate-strict-prefixes -r 2>&1 ||
echo 'command failed: validate-strict-prefixes -r'

echo '$ test-crefs'
test-crefs 2>&1 ||
echo 'command failed: test-crefs'
)

