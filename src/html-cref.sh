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

shopt -s extglob

usage()
{
    echo "usage: $1 [$(sed 's/^://;s/-:$/\x0/;s/[^:]/|-\0/g;s/:/ <arg>/g;s/^|//;s/\x0/-<long>/' <<< "$2")]"
}

quote()
{
    local __n__
    local __v__

    [ -z "$1" -o "$1" == "__n__" -o "$1" == "__v__" ] &&
    return 1

    printf -v __n__ '%q' "$1"
    eval __v__="\"\$$__n__\""
    #!!! echo "!!! 0 __v__='$__v__'"
    test -z "$__v__" && return 0
    printf -v __v__ '%q' "$__v__"
    #!!! echo "!!! 1 __v__='$__v__'"
    printf -v __v__ '%q' "$__v__"  # double quote
    #!!! echo "!!! 2 __v__='$__v__'"
    test -z "$SHELL_BASH_QUOTE_TILDE" &&
    __v__="${__v__//\~/\\~}"
    eval "$__n__=$__v__"
}

quote2()
{
    local __n__
    local __v__

    local __q__='q'
    [ "$1" == '-i' ] && {
        __q__=''
        shift
    }

    [ -z "$1" -o "$1" == "__n__" -o "$1" == "__v__" -o "$1" == "__q__" ] &&
    return 1

    printf -v __n__ '%q' "$1"
    eval __v__="\"\$$__n__\""
    __v__="$(sed -nr '
        H
        $!b
        g
        s/^\n//
        s/\x27/\0\\\0\0/g'${__q__:+'
        s/^/\x27/
        s/$/\x27/'}'
        p
    ' <<< "$__v__")"
    test -n "$__v__" &&
    printf -v __v__ '%q' "$__v__"
    eval "$__n__=$__v__"
}

optopt()
{
    local __n__="${1:-$opt}"       #!!!NONLOCAL
    local __v__=''
    test -n "$__n__" &&
    printf -v __v__ '%q' "$__n__"  # paranoia
    test -z "$SHELL_BASH_QUOTE_TILDE" &&
    __v__="${__v__//\~/\\~}"
    eval "$__n__=$__v__"
}

optarg()
{
    local __n__="${1:-$opt}"       #!!!NONLOCAL
    local __v__=''
    test -n "$OPTARG" &&
    printf -v __v__ '%q' "$OPTARG" #!!!NONLOCAL
    test -z "$SHELL_BASH_QUOTE_TILDE" &&
    __v__="${__v__//\~/\\~}"
    eval "$__n__=$__v__"
}

optact()
{
    local __v__="${1:-$opt}"       #!!!NONLOCAL
    printf -v __v__ '%q' "$__v__"  # paranoia
    test -z "$SHELL_BASH_QUOTE_TILDE" &&
    __v__="${__v__//\~/\\~}"
    eval "act=$__v__"
}

optactarg()
{
    optact ${1:+"$1"}
    local __v__=''
    test -n "$OPTARG" &&
    printf -v __v__ '%q' "$OPTARG" #!!!NONLOCAL
    test -z "$SHELL_BASH_QUOTE_TILDE" &&
    __v__="${__v__//\~/\\~}"
    eval "arg=$__v__"
}

optlong()
{
    local a="$1"

    if [ "$a" == '-' ]; then
        if [ -z "$OPT" ]; then                                      #!!!NONLOCAL
            local A="${OPTARG%%=*}"                                 #!!!NONLOCAL
            OPT="-$opt$A"                                           #!!!NONLOCAL
            OPTN="${OPTARG:$((${#A})):1}"                           #!!!NONLOCAL
            OPTARG="${OPTARG:$((${#A} + 1))}"                       #!!!NONLOCAL
        else
            OPT="--$OPT"                                            #!!!NONLOCAL
        fi
    elif [ "$opt" == '-' -o \( -n "$a" -a -z "$OPT" \) ]; then      #!!!NONLOCAL
        OPT="${OPTARG%%=*}"                                         #!!!NONLOCAL
        OPTN="${OPTARG:$((${#OPT})):1}"                             #!!!NONLOCAL
        OPTARG="${OPTARG:$((${#OPT} + 1))}"                         #!!!NONLOCAL
        [ -n "$a" ] && OPT="$a-$OPT"                                #!!!NONLOCAL
    elif [ -z "$a" ]; then                                          #!!!NONLOCAL
        OPT=''                                                      #!!!NONLOCAL
        OPTN=''                                                     #!!!NONLOCAL
    fi
}

optlongchkarg()
{
    test -z "$OPT" &&                               #!!!NONLOCAL
    return 0

    [[ "$opt" == [a-zA-Z] ]] || {                   #!!!NONLOCAL
        error "internal: invalid opt name '$opt'"   #!!!NONLOCAL
        return 1
    }

    local r="^:[^$opt]*$opt(.)"
    [[ "$opts" =~ $r ]]
    local m="$?"

    local s
    if [ "$m" -eq 0 ]; then
        s="${BASH_REMATCH[1]}"
    elif [ "$m" -eq 1 ]; then
        error "internal: opt '$opt' not in '$opts'" #!!!NONLOCAL
        return 1
    elif [ "$m" -eq "2" ]; then
        error "internal: invalid regex: $r"
        return 1
    else
        error "internal: unexpected regex match result: $m: ${BASH_REMATCH[@]}"
        return 1
    fi

    if [ "$s" == ':' ]; then
        test -z "$OPTN" && {                        #!!!NONLOCAL
            error --long -a
            return 1
        }
    else
        test -n "$OPTN" && {                        #!!!NONLOCAL
            error --long -d
            return 1
        }
    fi
    return 0
}

error()
{
    local __self__="$self"     #!!!NONLOCAL
    local __help__="$help"     #!!!NONLOCAL
    local __OPTARG__="$OPTARG" #!!!NONLOCAL
    local __opts__="$opts"     #!!!NONLOCAL
    local __opt__="$opt"       #!!!NONLOCAL
    local __OPT__="$OPT"       #!!!NONLOCAL

    local self="error"

    # actions: \
    #  a:argument for option -$OPTARG not found|
    #  o:when $OPTARG != '?': invalid command line option -$OPTARG, or, \
    #    otherwise, usage|
    #  i:invalid argument '$OPTARG' for option -$opt|
    #  d:option '$OPTARG' does not take arguments|
    #  e:error message|
    #  w:warning message|
    #  u:unexpected option -$opt|
    #  g:when $opt == ':': equivalent with 'a', \
    #    when $opt == '?': equivalent with 'o', \
    #    when $opt is anything else: equivalent with 'u'

    local act="e"
    local A="$__OPTARG__" # $OPTARG
    local h="$__help__"   # $help
    local m=""            # error msg
    local O="$__opts__"   # $opts
    local P="$__opt__"    # $opt
    local L="$__OPT__"    # $OPT
    local S="$__self__"   # $self

    local long=''         # short/long opts (default)

    #!!! echo "!!! A='$A'"
    #!!! echo "!!! O='$O'"
    #!!! echo "!!! P='$P'"
    #!!! echo "!!! L='$L'"
    #!!! echo "!!! S='$S'"

    local opt
    local opts=":aA:degh:iL:m:oO:P:S:uw-:"
    local OPTARG
    local OPTERR=0
    local OPTIND=1
    while getopts "$opts" opt; do
        case "$opt" in
            [adeiouwg])
                act="$opt"
                ;;
            #[])
            #	optopt
            #	;;
            [AhLmOPS])
                optarg
                ;;
            \:)	echo "$self: error: argument for option -$OPTARG not found" >&2
                return 1
                ;;
            \?)	if [ "$OPTARG" != "?" ]; then
                    echo "$self: error: invalid command line option -$OPTARG" >&2
                else
                    echo "$self: $(usage $self $opts)"
                fi
                return 1
                ;;
            -)	case "$OPTARG" in
                    long|long-opts)
                        long='l' ;;
                    short|short-opts)
                        long='' ;;
                    *)	echo "$self: error: invalid command line option --$OPTARG" >&2
                        return 1
                        ;;
                esac
                ;;
            *)	echo "$self: error: unexpected option -$OPTARG" >&2
                return 1
                ;;
        esac
    done
    #!!! echo "!!! A='$A'"
    #!!! echo "!!! O='$O'"
    #!!! echo "!!! P='$P'"
    #!!! echo "!!! L='$L'"
    #!!! echo "!!! S='$S'"
    shift $((OPTIND - 1))
    test -n "$1" && m="$1"
    local f="2"
    if [ "$act" == "g" ]; then
        if [ "$P" == ":" ]; then
            act="a"
        elif [ "$P" == "?" ]; then
            act="o"
        else 
            act="u"
        fi
    fi
    local o=''
    if [ -n "$long" -a -n "$L" ]; then
        test "${L:0:1}" != '-' && o+='--'
        o+="$L"
    elif [[ "$act" == [aod] ]]; then
        o="-$A"
    elif [[ "$act" == [iu] ]]; then
        o="-$P"
    fi
    case "$act" in
        a)	m="argument for option $o not found"
            ;;
        o)	if [ "$A" != "?" ]; then
                m="invalid command line option $o"
            else
                act="h"
                m="$(usage $S $O)"
                f="1"
            fi
            ;;
        i)	m="invalid argument for $o: '$A'"
            ;;
        u)	m="unexpected option $o"
            ;;
        d)	m="option $o does not take arguments"
            ;;
        *)	# [ew]
            if [ "$#" -ge "2" ]; then
                S="$1"
                m="$2"
            elif [ "$#" -ge "1" ]; then
                m="$1"
            fi
            ;;
    esac
    if [ "$act" == "w" ]; then
        m="warning${m:+: $m}"
    elif [ "$act" != "h" ]; then
        m="error${m:+: $m}"
    fi
    if [ -z "$S" -o "$S" == "-" ]; then
        printf "%s\n" "$m" >&$f
    else
        printf "%s: %s\n" "$S" "$m" >&$f
    fi
    if [ "$act" == "h" ]; then
        test -n "$1" && h="$1"
        test -n "$h" &&
        printf "%s\n" "$h" >&$f
    fi
    return $f
}

html-cref-gen()
{
    local self="html-cref-gen"
    local json="html-mathml.json"
    local over="html-cref-overrides.txt"
    local gent="@(sub|map)-table"
    local genx="@($gent|func|re2c-def|cref-overrides)"
    local geno="gen-$genx"

    local x="eval"
    local act="S"   # actions: \
                    #   N: print out HTML char refs names table (--cref-names)|
                    #   O: print out HTML char refs names with optional semicolon (--cref-names-semicolon)|
                    #   M: compute HTML char cref names' min and max length (--cref-names-min-max)|
                    #   S: print out HTML char refs substitution table (default) (--cref-subst)|
                    #   G: generate SUB table (--gen-sub-table)|
                    #   G: generate MAP table (--gen-map-table)|
                    #   G: generate RE2C def text (--gen-re2c-def)|
                    #   G: generate trie lookup/parse function (--gen-func)|
                    #   G: generate char ref overrides table (--gen-cref-overrides)
    local a=""      # when action is `-G|--gen-func' do not adjust the generated trie function (--no-adjust-func)
    local e=""      # when action is `-G|--gen-func' pass `-Pe' to 'gen-func' instead of `-Pf' (--expanded-path)
    local h=""      # when action is `-G|--gen-*' generate a heading comment text (--heading[-comment])
    local i=""      # when action is `-G|--gen-func' generate code that uses indexed access into the input string instead of pointer arithmetic (--indexed-access)
    local j="+"     # input HTML mathml JSON file (default: `html-mathml.json') (--mathml-json=FILE)
    local l=""      # when action is `-O|--cref-names-semicolon' or `-G|--gen-map-table' produce a comma-separated list instead of a table (--list)
    local m=""      # when action is `-G|--gen-func' apply only mapping to the generated trie function (--map-only)
    local n=""      # when action is `-G|--gen-func' generate code for processing semicolon-terminated input (--semicolon)
    local o="+"     # input HTML char ref overrides file (default: `html-cref-overrides.txt') (--cref-overrides=FILE)
    local s=""      # when action is `-G|--gen-func' make the generated trie function 'static' (--static-func)
    local w=""      # when action is `-G|--gen-func' pass `-w|--wide-code' to 'gen-func' (--wide-code)

    local arg=''    # action argument

    local opt
    local OPT
    local OPTN
    local opts=":adeG:hij:lmMnNo:OsSwx-:"
    local OPTARG
    local OPTERR=0
    local OPTIND=1
    while getopts "$opts" opt; do
        # discriminate long options
        optlong

        # translate long options to short ones
        test -n "$OPT" &&
        case "$OPT" in
            no-adjust-func)
                opt='a' ;;
            expanded-path)
                opt='e' ;;
            $geno)
                opt='G' ;;
            heading|heading-comment)
                opt='h' ;;
            indexed-access)
                opt='i' ;;
            mathml-json)
                opt='j' ;;
            list)
                opt='l' ;;
            map-only)
                opt='m' ;;
            cref-names-min-max)
                opt='M' ;;
            semicolon)
                opt='n' ;;
            cref-names)
                opt='N' ;;
            cref-overrides)
                opt='o' ;;
            cref-names-semicolon)
                opt='O' ;;
            static-func)
                opt='s' ;;
            cref-subst)
                opt='S' ;;
            wide-code)
                opt='w' ;;
            *)	error --long -o
                return 1
                ;;
        esac

        # check long option argument
        [[ "$opt" == [G] ]] ||
        optlongchkarg ||
        return 1

        # handle short options
        case "$opt" in
            d)	x="echo"
                ;;
            x)	x="eval"
                ;;
            [NMOS])
                optactarg
                ;;
            [aehilmnsw])
                optopt
                ;;
            [jo])
                optarg
                ;;
            G)	[[ -n "$OPT" || "$OPTARG" == $genx ]] || {
                    error -i
                    return 1
                }
                optlong "gen"

                [ -z "$OPTN" ] || {
                    error --long -d
                    return 1
                }
                case "${OPT:4}" in
                    $genx)
                        arg="${OPT:4}"
                        ;;
                    *)	error --long -o
                        return 1
                        ;;
                esac
                optact
                ;;
            *)	error --long -g
                return 1
                ;;
        esac
    done
    shift $((OPTIND - 1))

    if [ "$act$arg" != 'Gcref-overrides' ]; then
        [ "$j" == '+' ] && j="$json"
        if [ -z "$j" ]; then
            error "mathml JSON file name is null"
            return 1
        elif [ ! -f "$j" ]; then
            error "mathml JSON file '$j' not found"
            return 1
        fi
        quote j
    else
        [ "$o" == '+' ] && o="$over"
        if [ -z "$o" ]; then
            error "char ref overrides file name is null"
            return 1
        elif [ ! -f "$o" ]; then
            error "char ref overrides file '$o' not found"
            return 1
        fi
        quote o
    fi

    local c2
    local c3
    local s2
    local s3
    local o2
    local o3

    [ "$act" == 'G' -a \( "$arg" == 'func' -o -n "$h" \) ] && {
        while [ "$#" -gt 0 ]; do
            if [ -z "$1" ]; then
                o3="''"
            else
                o3="$1"
                quote o3
                [ "$o3" != "$1" ] && {
                    o3="$1"
                    quote2 o3
                }
            fi
            o2+="${o2:+ }$o3"
            shift
        done
    }

    if [[ "$act" == [MNOS] || "$act$arg" == G@($gent|re2c-def) ]]; then
        [[ "$act" == [GMNS] ]] && s2='
            \|^/characters/([^=]+)=(.*)$| {'
        [[ "$act" == [MN] ]] && s2+='
                s//\1/
                p'
        [[ "$act" == [GS] ]] && s2+='
                s//\1 \2/
                s/^NewLine $/\0\n/
                l'
        [[ "$act" == [GMNS] ]] && s2+='
            }'
        [ "$act" == 'O' ] && s2+='
            \|^/optional-;=(.*)$| {
                s//\1/'
        [ "$act" == 'O' -a -z "$l" ] && s2+='
                p'
        [ "$act" == 'O' -a -n "$l" ] && s2+='
                H'
        [ "$act" == 'O' ] && s2+='
            }'
        [ "$act" == 'O' -a -n "$l" ] && s2+='
            $!b
            g
            s/^\n//
            s/\n/,/g
            p'

        [[ "$act" == [GS] ]] && s3='s/\$$//'

        c2="\
json -Js $j|
sed -nr '$s2'${s3:+|
sed -r '$s3'}"

        [ "$act" == 'G' ] && c2+="|
awk -f html-cref.awk -v act=gen-$arg"

        [ "$act$arg" == 'Gsub-table' ] && {
            c3="$self --cref-names-semicolon --list"
            [ "$j" != "$json" ] && c3+=" -j $j"

            [ "$x" == 'echo' ] && echo "$c3"
            o3="$(eval "$c3")" && [ -n "$o3" ] || {
                error "inner command failed: $c3"
                return 1
            }
            quote2 o3

            c2+=" -v opt=$o3"
        }

        [ "$act$arg" == 'Gmap-table' -a -n "$l" ] && c2+='|
sed -nr '\''s/\s+/:/;H;$!b;g;s/^\n//;s/\n/,/g;p'\'

        [ "$act" == 'M' ] && c2+="|
awk -F '\n' '{
            l = length(\$0)
            if (m == 0 || m > l)
                m = l
            if (M < l)
                M = l
        }
        END {
            printf(\"\thtml_cref_max_name_len = %d,\n\", M);
            printf(\"\thtml_cref_min_name_len = %d,\n\", m);
        }'"
    elif [ "$act$arg" == 'Gfunc' ]; then
        c2="\
$self --cref-names"
        [ "$j" != "$json" ] && c2+=" -j $j"

        local e2
        [ -n "$e" ] && e2='e' || e2='f'

        c2+="|
gen-func ${o2:+$o2 }-r- -P$e2 -z -e-1${w:+ -w}"

        [ -z "$a" ] && {
            c3="$self --gen-map-table --list"
            [ "$j" != "$json" ] && c3+=" -j $j"

            [ "$x" == 'echo' ] && echo "$c3"
            o3="$(eval "$c3")" && [ -n "$o3" ] || {
                error "inner command failed: $c3"
                return 1
            }
            quote2 o3
        }

        [ -z "$a" ] && c2+="|
awk -F '\n' -f gen-func-adjust.awk"
        [ -z "$a" -a -n "$i" ] &&
        c2+=' -v indexed_access=1'
        [ -z "$a" -a -n "$n" ] &&
        c2+=' -v semicolon=1'
        [ -z "$a" -a -n "$s" ] &&
        c2+=' -v static_func=1'
        [ -z "$a" -a -n "$m" ] &&
        c2+=' -v map_only=1'
        [ -z "$a" ] &&
        c2+=" -v map=$o3"
    elif [ "$act$arg" == 'Gcref-overrides' ]; then
        c2="\
awk '{ printf(\"\t[0x%02x] = 0x%04x,\n\", strtonum(\$1) - 0x80, strtonum(\$2)) }' \\
$o"
    else
        error "unexpected act='$act'${arg:+ arg='$arg'}"
        return 1
    fi

    [ -n "$h" ] && {
        h="$self --gen-$arg"
        [ "$j" != "$json" ] &&
        h+=" --mathml-json=$j"
        h+=' --heading'
        [ -n "$a" ] &&
        h+=' --no-adjust-func'
        [ -z "$a" -a -n "$e" ] &&
        h+=' --expanded-path'
        [ -z "$a" -a -n "$i" ] &&
        h+=' --indexed-access'
        [ -z "$a" -a -n "$m" ] &&
        h+=' --map-only'
        [ -z "$a" -a -n "$n" ] &&
        h+=' --semicolon'
        [ -z "$a" -a -n "$s" ] &&
        h+=' --static-func'
        [ -z "$a" -a -n "$w" ] &&
        h+=' --wide-code'
        [ -n "$o2" ] &&
        h+=" -- $o2"

        quote2 -i h

        c2="\
(
echo '//'
echo '// This file was generated by a command like:'
echo '// \$ $h'
echo '//'
echo
$c2
echo
)
"
    }

    $x "$c2"
}

html-cref-test()
{
    local self="html-cref-test"
    local tstl='itrie etrie wtrie ietrie iwtrie bre2c re2c trie'
    local tsto="${tstl// /|}"
    local tsts="@($tsto)*(,@($tsto))"
    local timc='real process thread'
    local timo="@(${timc// /|})?(-timings)"
    local timx='^[0-9]+(,[0-9]+){2}$'
    local defP='etrie'
    local defr='100'
    local defs='5'
    local defw='9'

    local x="eval"
    local act="T"   # actions: \
                    #   N: print out the names of known HTML char ref parsers (--names)|
                    #   T: test named HTML char ref parsers; NAMES is a comma-separated list of HTML char ref parser names (default: '+', i.e. all)  (--test-set[=NAMES])|
                    #   P: process percents relative to the specified HTML char ref parser (default: '+' , i.e. 'etrie') (--percents[=NAME])
    local f=""      # force overwriting the output timings file if that already exists when action is `-T|--test-set' (--overwrite)
    local g=""      # group by names and sum up timings of input table when action is `-P|--percents' (--group)
    local i=""      # input test file when action is `-T|--test-set' or input timings file when action is `-P|--percents' (--input=FILE)
    local o="-"     # output timings file when action is `-T|--test-set'; '-' means to not generate such file at all (default); '+[SUFFIX]' stands for computing a name based on the input test file name: replace FILE's shortest `.' suffix with `.output[.SUFFIX]'; note that regardless of the argument these options have, the timings table is printed out on stdout (--output=FILE)
    local r="+"     # number of times to repeat the 'html-cref' command (default: 100) (--repeat=NUM)
    local m="+"     # pass `-m|--timings[=NUM,NUM,NUM]' or, by case, `--{real,process,thread}-timings[=NUM]' to 'html-cref'; the default NUM is '+', i.e. query that number from 'clocks' (--timings[=NUM,NUM,NUM]|--real[-timings][=NUM]|--process[-timings][=NUM]|--thread[-timings][=NUM])
    local c=""      # pass `-c|--clock-cycles[=NUM]' to 'html-cref' (default do not); the default NUM is '+', i.e. query that number from 'clocks' (--[clock-]cycles[=NUM])
    local s="+"     # sort or not the output table by the named timings column when action is `-P|--percents'; for sorting, NAME can be either 'real', 'process', 'thread' or 'cycles'; for not sorting the table at all, NAME must be '-' (default is sorting by '+', i.e. by 'thread') (--sort=NAME|--no-sort)
    local w="+"     # width of timings columns' integral part when action is `-T|--test-set' (default: 9) (--width=NUM)

    local arg='+'   # action argument

    local opt
    local OPT
    local OPTN
    local opts=":c:dfgi:m:No:P:r:s:T:w:x-:"
    local OPTARG
    local OPTERR=0
    local OPTIND=1
    while getopts "$opts" opt; do
        # discriminate long options
        optlong

        # translate long options to short ones
        test -n "$OPT" &&
        case "$OPT" in
            cycles|clock-cycles)
                opt='c' ;;
            overwrite)
                opt='f' ;;
            input)
                opt='i' ;;
            group)
                opt='g' ;;
            timings|$timo)
                opt='m' ;;
            names)
                opt='N' ;;
            output)
                opt='o' ;;
            percents)
                opt='P' ;;
            repeat)
                opt='r' ;;
            sort|no-sort)
                opt='s' ;;
            test-set)
                opt='T' ;;
            width)
                opt='w' ;;
            *)	error --long -o
                return 1
                ;;
        esac

        # check long option argument
        [[ "$opt" == [cmPsT] ]] ||
        optlongchkarg ||
        return 1

        # handle short options
        case "$opt" in
            d)	x="echo"
                ;;
            x)	x="eval"
                ;;
            [N])
                optactarg
                ;;
            [fg])
                optopt
                ;;
            [io])
                optarg
                ;;
            c)	#!!! echo >&2 "!!! OPT='$OPT' OPTN='$OPTN' OPTARG='$OPTARG'"
                case "$OPT" in
                    "")
                        if [[ "$OPTARG" =~ ^\+$|^[0-9]+$ ]]; then
                            c="$OPTARG"
                        else
                            error --long -i
                            return 1
                        fi
                        ;;
                    ?(clocks-)cycles)
                        [[ -z "$OPTN" || "$OPTARG" =~ ^\+$|^[0-9]+$ ]] || {
                            error --long -i
                            return 1
                        }
                        c="${OPTARG:-+}"
                        ;;
                    *)	error "internal: unexpected OPT='$OPT'"
                        return 1
                        ;;
                esac
                m=(- - -) # stev: reset $m
                ;;
            m)	#!!! echo >&2 "!!! OPT='$OPT' OPTN='$OPTN' OPTARG='$OPTARG'"
                case "$OPT" in
                    ""|timings)
                        if [[ "$OPTARG" == $timx ]]; then
                            m=(${OPTARG//,/ })
                        elif [[ "$OPTARG" == '+' || \
                                -n "$OPT" && -z "$OPTN" ]]; then
                            m='+'
                        else
                            error --long -i
                            return 1
                        fi
                        ;;
                    $timo)
                        [[ -z "$OPTN" || "$OPTARG" =~ ^\+$|^[0-9]+$ ]] || {
                            error --long -i
                            return 1
                        }
                        [ "$m" == '+' ] && m=(- - -)
                        [ "${OPT:0:1}" == 'r' ] && m[0]="${OPTARG:-+}"
                        [ "${OPT:0:1}" == 'p' ] && m[1]="${OPTARG:-+}"
                        [ "${OPT:0:1}" == 't' ] && m[2]="${OPTARG:-+}"
                        ;;
                    *)	error "internal: unexpected OPT='$OPT'"
                        return 1
                        ;;
                esac
                c='' # stev: reset $c
                ;;
            P)	#!!! echo >&2 "!!! OPT='$OPT' OPTN='$OPTN' OPTARG='$OPTARG'"
                if [ -n "$OPT" -a -z "$OPTN" ]; then
                    OPTARG='+'
                elif [[ "$OPTARG" != @(+|$tsto) ]]; then
                    error --long -i
                    return 1
                fi
                optactarg
                ;;
            s)	#!!! echo >&2 "!!! OPT='$OPT' OPTN='$OPTN' OPTARG='$OPTARG'"
                if [ "$OPT" == 'no-sort' ]; then
                    [ -z "$OPTN" ] || {
                        error --long -d
                        return 1
                    }
                    s=''
                else
                    [ -z "$OPT" -o -n "$OPTN" ] || {
                        error --long -a
                        return 1
                    }
                    case "$OPTARG" in
                        real)
                            s=3 ;;
                        process)
                            s=4 ;;
                        +|thread)
                            s=5 ;;
                        cycles)
                            s=6 ;;
                        -)	s=''
                            ;;
                        *)	error --long -i
                            return 1
                            ;;
                    esac
                fi
                ;;
            T)	#!!! echo >&2 "!!! OPT='$OPT' OPTN='$OPTN' OPTARG='$OPTARG'"
                if [ -n "$OPT" -a -z "$OPTN" ]; then
                    OPTARG='+'
                elif [[ "$OPTARG" != @(+|$tsts) ]]; then
                    error --long -i
                    return 1
                fi
                optactarg
                ;;
            [rw])
                [[ "$OPTARG" == @(+|+([0-9])) ]] || {
                    error --long -i
                    return 1
                }
                optarg
                ;;
            *)	error --long -g
                return 1
                ;;
        esac
    done
    shift $((OPTIND - 1))

    [ "$act" == 'N' ] && {
        echo -e "${tstl// /\\n}"
        return 1
    }

    # stev: from now on [ $act != 'N' ]

    [ -n "$1" ] && i="$1"
    [ "$act" == 'T' ] && {
        if [ -z "$i" ]; then
            error "input file not given"
            return 1
        elif [ "$i" == '-' ]; then
            error "input cannot be stdin when action is \`-T|--test-set'"
            return 1
        fi
    }
    [ "$i" == '-' ] && i=''
    [ -n "$i" -a ! -f "$i" ] && {
        error "input file '$i' not found"
        [ "$x" == 'eval' ] && return 1
    }
    quote i

    if [ "$r" == '+' ]; then
        r="$defr"
    elif [ "$r" -eq 0 ]; then
        error "repeat count cannot be 0"
        return 1
    fi
    # stev: need not quote $r

    if [ "$s" == '+' ]; then
        s="$defs"
    elif [ -n "$s" ] && \
         [ "$s" -lt 3 -o "$s" -gt 6 ]; then
        error "internal: unexpected s='$s'"
        return 1
    fi
    # stev: need not quote $s

    if [ "$w" == '+' ]; then
        w="$defw"
    elif [ "$w" -eq 0 ]; then
        error "column number width cannot be 0"
        return 1
    fi
    # stev: need not quote $w

    local t
    [ "$act" == 'T' ] && {
        [ "$arg" != '+' ] &&
        t=(${arg//,/ }) ||
        t=($tstl)
    }

    [ "${#t[@]}" -gt 0 ] || {
        error "internal: unexpected \${#t[@]}='${#t[@]}'"
        return 1
    } 

    local i2
    case "$o" in
        -)	o=''
            ;;
        +*)	i2="${i%.*}"
            [ -z "$i2" ] &&
            i2="$self"
            [ "${#o}" -gt 1 ] &&
            o="$i2.output.${o:1}" ||
            o="$i2.output"
            ;;
        *)	# stev: nop
            ;;
    esac
    [ "$act" == 'T' -a -z "$f" -a -n "$o" -a -e "$o" ] && {
        error "output timings file '$o' already exists"
        [ "$x" == 'eval' ] && return 1
    }
    [ -n "$f" -a -n "$o" -a -e "$o" -a ! -f "$o" ] && {
        error "output timings file '$o' is not a regular file"
        [ "$x" == 'eval' ] && return 1
    }
    quote o

    [ "$act" == 'T' ] && {
        { ./html-cref --help|grep -Fqe '-m|--timings'; } || {
            error "'html-cref' wasn't built with TIMINGS=yes"
            return 1
        }
    }
    [ "$act" == 'T' -a -n "$c" ] && {
        { ./html-cref --help|grep -Fqe '-c|--[clock-]cycles'; } || {
            error "'html-cref' wasn't built with CYCLES=yes"
            return 1
        }
    }

    local y=3
    [ "$act" == 'T' ] && {
        { ./clocks --help|grep -Fqwe 'cycles'; } && (( y++ ))
        [ -z "$c" -o "$y" -eq 4 ] || {
            error "'clocks' wasn't built with CYCLES=yes"
            return 1
        }
    }

    local c2
    local a
    local a2
    local s2
    local m2
    local k

    # stev: normalize $m
    [ "${#m[@]}" -eq 1 ] && m=($m $m $m)

    [[ -z "$c" && "${m[@]}" != '- - -' ||
       -n "$c" && "${m[@]}" == '- - -' ]] || {
        error "internal: unexpected m=(${m[@]}) and c='$c'"
        return 1
    }
    m[3]="${c:--}"

    [ "$act" == 'T' ] &&
    a=(real process thread cycles)

    [[ "$act" == 'T' && "${m[@]}" =~ \+ ]] && {
        a2=''
        for ((k=0; k<4; k++)); do
            [ "${m[$k]}" == '-' ] &&
            continue
            a2+="${a2:+ }${a[$k]}"
        done
        # stev: do not quote $a2 below
        c2="\
./clocks${a2:+ $a2}|awk '{ print \$(NR + 2) }'"

        if [ "$x" == 'echo' ]; then
            echo "$c2"
            m2=(1 2 3 4)
        else
            m2=($(set -o pipefail && eval "$c2")) &&
            [[ "${#m2[@]}" -eq $y ]] || {
                error "inner command failed: $c2"
                return 1
            }
            [[ "${#m2[@]}" -eq 3 ]] && m2[3]='-'
        fi
        c2=''
    }
    [ "$act" == 'T' ] && {
        a2=''
        for ((k=0; k<4; k++)); do
            [ "${m[$k]}" == '-' ] &&
            continue
            a2+="${a2:+ }--${a[$k]}="
            [ "${m[$k]}" == '+' ] &&
            a2+="${m2[$k]}" ||
            a2+="${m[$k]}"
        done
        if [ -z "$a2" ]; then
            [ -z "$c" ] &&
            m="-m ${m2[0]},${m2[1]},${m2[2]}" ||
            m="-c ${m2[3]}"
        else
            m="$a2"
        fi
    }

    if [ "$act" == 'T' -a "${#t[@]}" -eq 1 ]; then
        a='
            BEGIN {
                C["real"] = 1
                C["process"] = 2
                C["thread"] = 3'
        [ -n "$c" ] && a+='
                C["cycles"] = 4
                N = 4'
        [ -z "$c" ] && a+='
                N = 3'
        a+='
            }
            {
                if (!match($0, /^('"${timc// /|}${c:+|cycles}"')-timings:[ \t]*([^ \t]+)[ \t]*$/, a))
                    print $0 > "/dev/stderr"
                else {
                    i = C[a[1]]
                    V[i] = a[2]
                    A[i] = 1
                }
            }
            END {
                for (i = 1; i <= N; i ++) {
                    printf("%s%s", \
                        A[i] ? V[i] : "-", \
                        i < N ? " " : "\n")
                }
            }'

        (( w += 3 ))
        [ -z "$c" ] && a2='
            BEGIN { N = 3 }'
        [ -n "$c" ] && a2='
            BEGIN { N = 4 }'
        a2+='
            function mean(i)
            {
                return !E[i] \
                    ? sprintf("%'"$w"'.2f", V[i] / FNR) \
                    : sprintf("%'"$w"'s", "-")
            }
            {
                for (i = 1; i <= N; i ++) {
                    if ($i != "-")
                        V[i] += $i
                    else
                        E[i] = 1
                }
            }
            END {
                printf("%d", FNR)
                for (i = 1; i <= N; i ++)
                    printf(" %s", mean(i))
                printf("\n")
            }'

        # stev: need not quote $arg below
        # stev: do not quote $m below
        c2="\
r=$r
set -o pipefail
while [ \"\$((r --))\" -gt 0 ]; do
    LD_LIBRARY_PATH=. \\
    ./html-cref ${i:+-f $i }-t sponge -p $arg $m 2>&1 >/dev/null|
    awk -F '\n' '$a' ||
    break
done|
awk '$a2'"
        [ -n "$o" ] &&
        c2+="|
stdbuf -oL tee $o"
    elif [ "$act" == 'T' ]; then
        local k
        local n
        local p=0
        for ((k=0;k<${#t[@]};k++)); do
            n="${t[$k]}"
            [ "$p" -lt "${#n}" ] &&
            p="${#n}"
        done
        printf -v p '%*s' $((p + 2))

        local r2="$r"
        [ "$r2" -eq "$defr" ] && r2=''
        local w2="$w"
        [ "$w2" -eq "$defw" ] && w2=''

        [ -n "$o" ] && c2+="\
truncate -s0 $o"
        # stev: need not quote $t, $r2, $w2 below
        # stev: do not quote $m below
        c2+=${c2:+$'\n'}"\
time for t in ${t[@]}; do
    $self ${i:+-i $i }${r2:+-r $r2 }${w2:+-w $w2 }-o- -T \$t $m|
    sed -ru \"s/^/\$t:$p/;s/^(.{${#p}})\s*/\1/\""
        [ -n "$o" ] && c2+="|
    stdbuf -oL tee -a $o"
        c2+='
done'
    elif [ "$act" == 'P' ]; then
        [ "$arg" == '+' ] && arg="$defP"

        a='
            function record_group(	r, i)
            {
                r = sprintf("%3d", C[1])
                for (i = 2; i <= N; i ++)
                    r = r sprintf(" %12.f", C[i])
                G[p] = r
            }
            {
                if (p != $1) {
                    if (length(p))
                        record_group()
                    delete C
                    p = $1
                }
                for (i = 1; i < NF; i ++)
                    C[i] += $(i + 1)
                N = NF - 1
            }
            END{
                record_group()
                for (p in G)
                    printf("%-7s %s\n", p, G[p])
            }'

        # stev: no need to quote $arg below
        a2='
            function percents(t, v,	r)
            {
                if (t != "-" && v != "-" && t) {
                    r = sprintf("%.2f", (t - v) / t * 100)
                    return r == "-0.00" ? substr(r, 2) : r
                }
                return "-"
            }
            {
                L[++ N] = $0

                for (i = 1; i < 3; i ++) {
                    l = length($i)
                    if (W[i] < l)
                        W[i] = l
                }

                if ($1 != "'"$arg"':")
                    next
                for (j = 3; j <= NF; j ++)
                    R[j] = $j
            }
            END {
                W[1] ++

                for (i = 1; i <= N; i ++) {
                    n = split(L[i], A)
                    for (j = 1; j < 3; j ++)
                        printf("%-*s", W[j], A[j])
                    for (j = 3; j <= n; j ++)
                        printf(" %6s", percents(A[j], R[j]))
                    printf("\n")
                }
            }'

        [ -n "$g" ] && c2="\
sort -k1,1 -s${i:+ $i}|
awk '$a'|"$'\n'
        c2+="\
awk '$a2'"
        [ -z "$g" ] && c2+="${i:+ \\
$i}"
        [ -n "$s" ] && c2+="|
sort -k${s}g,$s"
        [ -z "$s" -a -n "$g" ] && c2+='|
sort -k1,1'
    else
        error "internal: unexpected act='$act'"
        return 1
    fi

    $x "$c2"
}

git-repo-diff()
{
    local self="git-repo-diff"
    local trgx='@(json-type)'
    local deft='json-type'

    local x="eval"
    local b=""          # pass '-b|--ignore-space-change' to diff (--ignore-space-change)
    local h="+"         # home dir (default: '.') (--home=DIR)
    local g="+"         # 'git' repo directory (default: '$HOME/$target') (--git-dir=DIR)
    local s="+"         # sha1 hashes file name ('-' means stdin, the default is '$home/lib/$target-files.txt') (--sha1-hashes=FILE)
    local t="+"         # target name: 'json-type' (--target=NAME|--json-type)
    local u=""          # pass '-u|--unified=NUM' to diff (--unified=NUM)

    local opt
    local OPT
    local OPTN
    local opts=":bdg:h:s:t:u:x-:"
    local OPTARG
    local OPTERR=0
    local OPTIND=1
    while getopts "$opts" opt; do
        # discriminate long options
        optlong

        # translate long options to short ones
        test -n "$OPT" &&
        case "$OPT" in
            ignore-space-change)
                opt='b' ;;
            git-dir)
                opt='g' ;;
            home)
                opt='h' ;;
            sha1-hashes)
                opt='s' ;;
            target|$trgx)
                opt='t' ;;
            unified)
                opt='u' ;;
            *)	error --long -o
                return 1
                ;;
        esac

        # check long option argument
        [[ "$opt" == [t] ]] ||
        optlongchkarg ||
        return 1

        # handle short options
        case "$opt" in
            d)	x="echo"
                ;;
            x)	x="eval"
                ;;
            [ghs])
                optarg
                ;;
            [b])
                optopt
                ;;
            t)	[[ -n "$OPT" || "$OPTARG" == $trgx ]] || {
                    error -i
                    return 1
                }
                optlong -

                [[ "${OPT:2}" == @(target|$trgx) ]] || {
                    error --long -o
                    return 1
                }
                case "${OPT:2}" in
                    target)
                        [ -n "$OPTN" ] || {
                            error --long -a
                            return 1
                        }
                        [[ "$OPTARG" == $trgx ]] || {
                            error --long -i
                            return 1
                        }
                        t="$OPTARG"
                        ;;
                    $trgx)
                        [ -z "$OPTN" ] || {
                            error --long -d
                            return 1
                        }
                        t="${OPT:2}"
                        ;;
                    *)	error "internal: unexpected OPT='$OPT'"
                        return 1
                        ;;
                esac
                ;;
            u)	[[ "$OPTARG" == +([0-9]) ]] || {
                    error --long -i
                    return 1
                }
                optarg
                ;;
            *)	error --long -g
                return 1
                ;;
        esac
    done
    shift $((OPTIND - 1))

    [ -z "$h" ] && {
        error "home dir cannot be null"
        return 1
    }
    [ "$h" == '+' ] && h='.'
    [ ! -d "$h" ] && {
        error "home dir '$h' not found"
        test "$x" == "eval" && return 1
    }

    [ "$t" == '+' ] && t="$deft"

    [ -n "$g" ] || {
        error "git directory cannot be null"
        return 1
    }
    [ "$g" == '+' ] && g="$HOME/$t"
    [ -d "$g" ] || {
        error "git directory '$g' not found"
        return 1
    }
    quote g

    [ -n "$s" ] || {
        error "sha1 hashes file name cannot be null"
        return 1
    }
    [ "$s" == '-' ] && s=""
    [ "$s" == '+' ] && s="$h/lib/$t-files.txt"
    [ -z "$s" -o -f "$s" ] || {
        error "sha1 hashes file '$s' not found"
        return 1
    }
    quote s

    local c="\
git \\
--git-dir=${g%%+(/)?(.git*(/))}/.git rev-parse \\
--git-dir"
    [ "$x" == "echo" ] && echo "$c"

    local g2
    g2="$(eval "$c")" && [ -n "$g2" ] || {
        error "inner command failed: git" #!!! ${c//@(\\|$'\n')/}"
        return 1
    }
    quote g2

    local s2='/^[0-9a-f]{40}\s/p'
    local f

    c="\
sed -nr '$s2'${s:+ \\
$s}|
while read h f; do
    git --git-dir=$g2 cat-file -e \"\$h\" || {
        error \"git object \$h not found\"
        continue
    }
    diff -${b}u$u \\
    -L \"\$f\" <(git --git-dir=$g2 cat-file -p \$h) \\
    -L \"\$f\" \"lib/$t/\$f\"
done"

    $x "$c"
}

