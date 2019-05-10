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

#define _GNU_SOURCE
#include <getopt.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"
#include "ptr-traits.h"
#include "char-traits.h"
#include "pretty-print.h"
#include "file-buf.h"
#include "su-size.h"

#ifndef BUILTIN
#include "dyn-lib.h"
#endif

#ifdef TIMINGS
#include "clocks-impl.h"
#endif

#include "html-cref.h"
#include "html-cref-table.h"
#include "html-cref-unicode.h"

#ifdef BUILTIN
#include BUILTIN_HEADER
#endif

#define ISASCII CHAR_IS_ASCII
#define ISALNUM CHAR_IS_ALNUM

const char stdin_name[] = "<stdin>";

const char program[] = STRINGIFY(PROGRAM);
const char verdate[] = "0.6 -- 2019-05-05 10:58"; // $ date +'%F %R'

const char license[] =
"Copyright (C) 2019  Stefan Vargyas.\n"
"License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n"
"This is free software: you are free to change and redistribute it.\n"
"There is NO WARRANTY, to the extent permitted by law.\n";

const char help[] = 
"usage: %s [ACTION|OPTION]...\n"
"where actions are specified as:\n"
"  -S|--subst-cref             substitute all HTML char references in the\n"
"                                given input file (default)\n"
"  -P|--print-cref             print out the HTML char references for each\n"
"                                named reference; take the names from the\n"
"                                input file, each given on a separate line\n"
"the options are:\n"
"  -f|--input-file=FILE        take input from the named file (the default\n"
"                                is '-', aka stdin)\n"
"  -t|--input-type=TYPE        process input file as specified by TYPE:\n"
"                                'liner' for line-by-line operations, or\n"
"                                'sponge' for operations applied at once\n"
"                                on a buffer containing the whole input\n"
"                                file; the default is 'liner'\n"
#ifndef BUILTIN
"  -p|--cref-parser=NAME       use the specified HTML character reference\n"
"                                parser: that is load the external module\n"
"                                `html-cref-NAME.so' (a shared library)\n"
"                                that defines proper parse functions\n"
"                                (the default is 'ietrie')\n"
#endif
"  -e|--[no-]semicolons        when action is `-S|--subst-cref', require\n"
"                                that all char references be terminated\n"
"                                with semicolon -- even those that were\n"
"                                allowed to the contrary for historical\n"
"                                reasons (that is the default behavior)\n"
"  -s|--sponge-max=NUM[KM]     the maximum size of the input buffer when\n"
"                                the input type is 'sponge' (by default\n"
"                                is 5M; not allowed to be more than 10M)\n"
#ifdef TIMINGS
"  -m|--timings[=NUM,NUM,NUM]  print out on stderr the total amount of\n"
"     --real-timings[=NUM]       nanoseconds spent by the HTML character\n"
"     --process-timings[=NUM]    reference parser (default do not); use\n"
"     --thread-timings[=NUM]     the given three numbers as the amount of\n"
"     --no-timings               nanoseconds of overhead per call of the C\n"
"                                library function 'clock_gettime(3)'; NUM\n"
"                                are decimal non-negative integers, each\n"
"                                corresponding to one of the clock ids:\n"
"                                CLOCK_REALTIME, CLOCK_PROCESS_CPUTIME_ID,\n"
"                                and CLOCK_THREAD_CPUTIME_ID; the options\n"
"                                `--{real,process,thread}-timings' are to\n"
"                                be used to specify timings on subsets of\n"
"                                the mentioned three clock types\n"
#endif // TIMINGS
"  -w|--warnings[-only]        print out a warning message on stderr for\n"
"     --no-warnings              each invalid HTML char reference found\n"
"                                in the input given (default not); the\n"
"                                option `--warnings-only' prevents the\n"
"                                program to produce anything, but these\n"
"                                warning messages on stdout; `-w' cuts\n"
"                                short option `--warnings-only'\n"
"     --dump-options           print options and exit\n"
"     --version                print version numbers and exit\n"
"  -?|--help                   display this help info and exit\n";

#ifndef BUILTIN
static void error_fmt(const char* fmt, ...)
    PRINTF(1);

static void error_fmt(const char* fmt, ...)
{
    va_list a;

    fprintf(stderr,
        "%s: error: ", program);

    va_start(a, fmt);
    vfprintf(stderr, fmt, a); 
    va_end(a);
}
#endif

enum options_action_t
{
    options_subst_cref_action,
    options_print_cref_action,
};

enum options_input_type_t
{
    options_input_type_liner,
    options_input_type_sponge
};

struct options_t
{
    enum options_action_t
                 action;
    enum options_input_type_t
                 input_type;
    const char*  input_file;
#ifndef BUILTIN
    const char*  cref_parser;
#endif
    size_t       sponge_max;
#ifdef TIMINGS
    bits_t       timings: 3;
    size_t       overhead[3];
#endif
    bits_t       semicolons: 1;
    bits_t       warnings: 2;

    size_t       argc;
    char* const *argv;
};

static void options_version(void)
{
    fprintf(stdout, "%s: version %s\n\n%s",
        program, verdate, license);
}

static void options_usage(void)
{
    fprintf(stdout, help, program);
}

#ifdef TIMINGS

static char* options_dump_timings(
    const struct options_t* opts)
{
    char* b = NULL;
    size_t n = 0;
    FILE* f;

    if (opts->timings == 0)
        return strdup("-");

    f = open_memstream(&b, &n);
    clocks_print_names(opts->timings, f);

    fclose(f);
    ASSERT(n > 0);

    return b;
}

#endif // TIMINGS

static void options_dump(const struct options_t* opts)
{
    static const char* const noyes[] = {
        [0] "no", [1] "yes"
    };
    static const char* const warnings[] = {
        [0] "no", [1] "yes", [2] "only"
    };
#define CASE2(n0, n1) \
    [options_ ## n0 ## _ ## n1 ## _action] = #n0 "-" #n1
    static const char* const actions[] = {
        CASE2(subst, cref),
        CASE2(print, cref),
    };
#define CASE(n) \
    [options_input_type_ ## n] = #n
    static const char* const input_types[] = {
        CASE(liner),
        CASE(sponge)
    };
    struct su_size_t sponge_su = su_size(
        opts->sponge_max);
#ifdef TIMINGS
    char b[128], *c;
#endif

#define NAME_(x, t)                  \
    ({                               \
        size_t __v = opts->x;        \
        ARRAY_NON_NULL_ELEM(t, __v); \
    })
#define NAME(x)  NAME_(x, x ## s)
#define NNUL(x)  (opts->x ? opts->x : "-")
#define NOYES(x) NAME_(x, noyes)
#define ARRAY(x) NAME_(x, x)

#ifdef TIMINGS
    su_size_list_to_string(
        b, ARRAY_SIZE(b), opts->overhead,
        ARRAY_SIZE(opts->overhead),
        ",", true);
    c = options_dump_timings(opts);
#endif

    fprintf(stdout,
        "action:      %s\n"
        "input-file:  %s\n"
        "input-type:  %s\n"
#ifndef BUILTIN
        "cref-parser: %s\n"
#endif
        "sponge-max:  %zu%s\n"
        "semicolons:  %s\n"
#ifdef TIMINGS
        "timings:     %s\n"
        "overhead:    %s\n"
#endif
        "warnings:    %s\n"
        "argc:        %zu\n",
        NAME(action),
        NNUL(input_file),
        NAME(input_type),
#ifndef BUILTIN
        opts->cref_parser,
#endif
        sponge_su.sz,
        sponge_su.su,
        NOYES(semicolons),
#ifdef TIMINGS
        c, b,
#endif
        ARRAY(warnings),
        opts->argc);

#undef ARRAY

#ifdef TIMINGS
    free(c);
#endif

    pretty_print_strings(stdout,
        PTR_CONST_PTR_CAST(opts->argv, char),
        opts->argc, "argv", 13, 0);
}

static void options_invalid_opt_arg(
    const char* opt_name, const char* opt_arg)
{
    error("invalid argument for '%s' option: '%s'",
        opt_name, opt_arg);
}

static void options_illegal_opt_arg(
    const char* opt_name, const char* opt_arg)
{
    error("illegal argument for '%s' option: '%s'",
        opt_name, opt_arg);
}

static void options_invalid_opt_arg2(
    const char* opt_name, const char* opt_arg,
    const char* reason)
{
    error("invalid argument for '%s' option: '%s': %s",
        opt_name, opt_arg, reason);
}

#define SU_SIZE_OPT_NAME options
#define SU_SIZE_OPT_NEED_PARSE_SIZE
#define SU_SIZE_OPT_NEED_PARSE_SIZE_SU
#include "su-size-opt-impl.h"

// $ . ~/lookup-gen/commands.sh
// $ print() { printf '%s\n' "$@"; }
// $ adjust-func() { ssed -R '1s/^/static /;1s/\&/*/;1s/(,\s+)/\1enum /;s/(?=t =)/*/;1s/(?<=\()/\n\t/;s/_t::/_/'; }

// $ print liner sponge|gen-func -f options_lookup_input_type -r options_input_type_t -Pf -q \!strcmp|adjust-func

static bool options_lookup_input_type(
    const char* n, enum options_input_type_t* t)
{
    // pattern: liner|sponge
    switch (*n ++) {
    case 'l':
        if (!strcmp(n, "iner")) {
            *t = options_input_type_liner;
            return true;
        }
        return false;
    case 's':
        if (!strcmp(n, "ponge")) {
            *t = options_input_type_sponge;
            return true;
        }
    }
    return false;
}

static enum options_input_type_t
    options_parse_input_type_optarg(
        const char* opt_name, const char* opt_arg)
{
    enum options_input_type_t r;

    ASSERT(opt_arg != NULL);
    if (!options_lookup_input_type(opt_arg, &r))
        options_invalid_opt_arg(opt_name, opt_arg);

    return r;
}

static const char*
    options_parse_file_optarg(
        const char* opt_name, const char* opt_arg)
{
    struct stat s;
    int r, e;

    if (!strcmp(opt_arg, "-"))
        return NULL;

    r = stat(opt_arg, &s);
    e = errno;

    if (r && e != ENOENT && e != ENOTDIR)
        error("%s: stat failed: %s",
            opt_arg, strerror(e));
    if (r)
        options_invalid_opt_arg2(
            opt_name, opt_arg,
            "file not found");

    return opt_arg;
}

#ifndef BUILTIN
static const char*
    options_parse_cref_parser_optarg(
        const char* opt_name, const char* opt_arg)
{
    const char *p, *e;

    for (p = opt_arg,
         e = p + strlen(p);
         p < e;
         p ++) {
        if (!ISALNUM(*p) && *p != '-')
            options_invalid_opt_arg(
                opt_name, opt_arg);
    }
    return opt_arg;
}
#endif

#ifdef TIMINGS

static size_t options_parse_overheads_optarg(
    const char* opt_name, const char* opt_arg,
    size_t* vals, size_t n_vals)
{
    size_t r;

    ASSERT(n_vals == 3);

    if (opt_arg == NULL) {
        memset(vals, 0,
            SIZE_MUL(sizeof(*vals), n_vals));
        return clock_types_all;
    }

    if (!su_size_parse_list(
            opt_arg, 0, 1000, ',',
            vals, n_vals, NULL, NULL, &r))
        options_invalid_opt_arg(opt_name, opt_arg);
    ASSERT(r <= n_vals);

    if (r < n_vals)
        options_illegal_opt_arg(opt_name, opt_arg);

    return clock_types_all;
}

static size_t options_parse_clock_overhead_optarg(
    size_t clock, const char* opt_name, const char* opt_arg,
    size_t* vals, size_t n_vals)
{
    ASSERT(n_vals == 3);
    ASSERT(clock < 3);

    vals[clock] = opt_arg != NULL
        ? options_parse_size_optarg(opt_name, opt_arg,
            0, 1000)
        : 0;

    return 1U << clock;
}

#endif // TIMINGS

static const struct options_t* options(
    int argc, char* argv[])
{
    static struct options_t opts = {
        .action      = options_subst_cref_action,
        .input_type  = options_input_type_liner,
#ifndef BUILTIN
        .cref_parser = "etrie",
#endif
        .sponge_max  = MB(5),
    };

    enum {
        // stev: actions:
        subst_cref_act    = 'S',
        print_cref_act    = 'P',

        // stev: options:
        input_file_opt    = 'f',
        input_type_opt    = 't',
#ifndef BUILTIN
        cref_parser_opt   = 'p',
#endif
        sponge_max_opt    = 's',
        semicolons_opt    = 'e',
#ifdef TIMINGS
        timings_opt       = 'm',
#endif
        warnings_only_opt = 'w',
        help_opt          = '?',
        dump_opt          = 128,
        version_opt,

        warnings_opt,
        no_semicolons_opt,
#ifdef TIMINGS
        real_timings_opt,
        process_timings_opt,
        thread_timings_opt,
        no_timings_opt,
#endif
        no_warnings_opt,
    };

    static const struct option longs[] = {
        { "subst-cref",      0,       0, subst_cref_act },
        { "print-cref",      0,       0, print_cref_act },
        { "input-file",      1,       0, input_file_opt },
        { "input-type",      1,       0, input_type_opt },
#ifndef BUILTIN
        { "cref-parser",     1,       0, cref_parser_opt },
#endif
        { "sponge-max",      1,       0, sponge_max_opt },
        { "semicolons",      0,       0, semicolons_opt },
        { "no-semicolons",   0,       0, no_semicolons_opt },
#ifdef TIMINGS
        { "timings",         2,       0, timings_opt },
        { "real-timings",    2,       0, real_timings_opt },
        { "process-timings", 2,       0, process_timings_opt },
        { "thread-timings",  2,       0, thread_timings_opt },
        { "no-timings",      0,       0, no_timings_opt },
#endif
        { "warnings",        0,       0, warnings_opt },
        { "warnings-only",   0,       0, warnings_only_opt },
        { "no-warnings",     0,       0, no_warnings_opt },
        { "dump-options",    0,       0, dump_opt },
        { "version",         0,       0, version_opt },
        { "help",            0, &optopt, help_opt },
        { 0,                 0,       0, 0 }
    };
    static const char shorts[] = ":" "PS" "ef:s:t:w"
#ifndef BUILTIN
        "p:"
#endif
#ifdef TIMINGS
        "m:"
#endif
    ;
    struct bits_opts_t
    {
        bits_t dump: 1;
        bits_t usage: 1;
        bits_t version: 1;
    };
    struct bits_opts_t bits = {
        .dump    = false,
        .usage   = false,
        .version = false
    };
    int opt;

#define argv_optind()                      \
    ({                                     \
        size_t i = INT_AS_SIZE(optind);    \
        ASSERT_SIZE_DEC_NO_OVERFLOW(i);    \
        ASSERT(i - 1 < INT_AS_SIZE(argc)); \
        argv[i - 1];                       \
    })

#define optopt_char()                   \
    ({                                  \
        ASSERT(ISASCII((char) optopt)); \
        (char) optopt;                  \
    })

#define missing_opt_arg_str(n) \
    error("argument for option '%s' not found", n)
#define missing_opt_arg_ch(n) \
    error("argument for option '-%c' not found", n);
#define not_allowed_opt_arg(n) \
    error("option '%s' does not allow an argument", n)
#define invalid_opt_str(n) \
    error("invalid command line option '%s'", n)
#define invalid_opt_ch(n) \
    error("invalid command line option '-%c'", n)

    opterr = 0;
    optind = 1;
    while ((opt = getopt_long(
        argc, argv, shorts, longs, 0)) != EOF) {
        switch (opt) {
        case subst_cref_act:
            opts.action = options_subst_cref_action;
            break;
        case print_cref_act:
            opts.action = options_print_cref_action;
            break;
        case input_file_opt:
            opts.input_file = options_parse_file_optarg(
                "input-file", optarg);
            break;
        case input_type_opt:
            opts.input_type = options_parse_input_type_optarg(
                "input-type", optarg);
            break;
#ifndef BUILTIN
        case cref_parser_opt:
            opts.cref_parser = options_parse_cref_parser_optarg(
                "cref-parser", optarg);
            break;
#endif
        case sponge_max_opt:
            opts.sponge_max = options_parse_su_size_optarg(
                "sponge-max", optarg, 1, MB(10));
            break;
        case semicolons_opt:
            opts.semicolons = true;
            break;
        case no_semicolons_opt:
            opts.semicolons = false;
            break;
#ifdef TIMINGS
        case timings_opt:
            opts.timings = options_parse_overheads_optarg(
                "timings", optarg, opts.overhead,
                ARRAY_SIZE(opts.overhead));
            break;
        case real_timings_opt:
            opts.timings |= options_parse_clock_overhead_optarg(
                clock_type_real, "real-timings", optarg,
                opts.overhead, ARRAY_SIZE(opts.overhead));
            break;
        case process_timings_opt:
            opts.timings |= options_parse_clock_overhead_optarg(
                clock_type_process, "process-timings", optarg,
                opts.overhead, ARRAY_SIZE(opts.overhead));
            break;
        case thread_timings_opt:
            opts.timings |= options_parse_clock_overhead_optarg(
                clock_type_thread, "thread-timings", optarg,
                opts.overhead, ARRAY_SIZE(opts.overhead));
            break;
        case no_timings_opt:
            opts.timings = 0;
            break;
#endif
        case warnings_only_opt:
            opts.warnings = 2;
            break;
        case warnings_opt:
            opts.warnings = 1;
            break;
        case no_warnings_opt:
            opts.warnings = 0;
            break;
        case dump_opt:
            bits.dump = true;
            break;
        case version_opt:
            bits.version = true;
            break;
        case 0:
            bits.usage = true;
            break;
        case ':': {
            const char* opt = argv_optind();
            if (opt[0] == '-' && opt[1] == '-')
                missing_opt_arg_str(opt);
            else
                missing_opt_arg_ch(optopt_char());
            break;
        }
        case '?':
        default:
            if (optopt == 0)
                invalid_opt_str(argv_optind());
            else
            if (optopt != '?') {
                char* opt = argv_optind();
                if (opt[0] == '-' && opt[1] == '-') {
                    char* end = strchr(opt, '=');
                    if (end) *end = '\0';
                    not_allowed_opt_arg(opt);
                }
                else
                    invalid_opt_ch(optopt_char());
            }
            else
                bits.usage = true;
            break;
        }
    }

    ASSERT(optind > 0);
    ASSERT(optind <= argc);

    argc -= optind;
    argv += optind;

    opts.argc = INT_AS_SIZE(argc);
    opts.argv = argv;

    if (opts.argc > 0) {
        opts.input_file = *opts.argv ++;
        opts.argc --;
    }

    if (bits.version)
        options_version();
    if (bits.dump)
        options_dump(&opts);
    if (bits.usage)
        options_usage();

    if (bits.dump ||
        bits.version ||
        bits.usage)
        exit(0);

    if (opts.input_file != NULL &&
        !strcmp(opts.input_file, "-"))
        opts.input_file = NULL;

    return &opts;
}

enum process_cref_flags_t
{
    process_cref_strict_semis = 1U << 0,
    process_cref_warn_invalid = 1U << 1,
    process_cref_print_output = 1U << 2,
};

#define PROCESS_CREF_FLAGS_(n) \
    process_cref_ ## n
#define PROCESS_CREF_FLAGS(...) \
    (VA_ARGS_REPEAT(|, PROCESS_CREF_FLAGS_, __VA_ARGS__))

#define PROCESS_CREF_FLAGS_HAS_(n) \
    (flags & PROCESS_CREF_FLAGS_(n))
#define PROCESS_CREF_FLAGS_HAS(...) \
    (VA_ARGS_REPEAT(&&, PROCESS_CREF_FLAGS_HAS_, __VA_ARGS__))
#define PROCESS_CREF_FLAGS_HAS_ONE(...) \
    (VA_ARGS_REPEAT(||, PROCESS_CREF_FLAGS_HAS_, __VA_ARGS__))

#define FLAGS_HAS     PROCESS_CREF_FLAGS_HAS
#define FLAGS_HAS_ONE PROCESS_CREF_FLAGS_HAS_ONE

static void process_cref_warn(
    const char* ptr, size_t len, bool subst,
    enum process_cref_flags_t flags)
{
    enum { N = 2 * html_cref_max_name_len };
    FILE* f = FLAGS_HAS(print_output)
        ? stderr : stdout;
    const char* q;

    if (subst) {
        ASSERT(*ptr == '&');
        ptr ++;

        if (!len) {
            q = ptr;
            while (ISALNUM(*q))
                q ++;

            len = PTR_DIFF(q, ptr);
            ASSERT(len > 0);

            if (len > N)
                len = N;
        }
    }

    fprintf(
        f, "%s: warning: invalid char reference '",
        program);
    pretty_print_string(
        f, PTR_UCHAR_CAST_CONST(ptr), len, 0);
    fputs("'\n", f);
}

#ifdef TIMINGS
static size_t process_cref_count = 0;
#endif

#ifndef BUILTIN
typedef
    int (*process_cref_func_t)(const char*);
#else
#define PROCESS_CREF_(n) html_cref_ ## n ## _parse
#define PROCESS_CREF(n)  PROCESS_CREF_(n)
#define process_cref     PROCESS_CREF(BUILTIN)
#endif

static void process_subst_cref(
    char* buf, size_t len,
#ifndef BUILTIN
    process_cref_func_t process_cref,
#endif
    enum process_cref_flags_t flags)
{
    const size_t mask = SZ(1) << 7;
    const char *p = buf, *q;
    size_t l = len, d;

    STATIC(CHAR_BIT == 8);

    ASSERT(buf != NULL);
    ASSERT(len > 0);

    ASSERT(FLAGS_HAS_ONE(
        warn_invalid, print_output));

    ASSERT(buf[len] == 0);

    while ((q = memchr(p, '&', l))) {
        d = PTR_DIFF(q, p);
        if (FLAGS_HAS(print_output))
            fwrite(p, 1, d, stdout);
        p += d;
        l -= d;

        if (q[1] == '#') {
            code_point_t c;
            utf8_t u;
            size_t n;
            bool b;

            q += 2;
            b = html_cref_unicode_parse_html(&q, &c);
            d = PTR_DIFF(q, p);

            if (!b || *q != ';') {
                if (FLAGS_HAS(warn_invalid))
                    process_cref_warn(
                        p, d, true, flags);
                if (FLAGS_HAS(print_output))
                    fwrite(p, 1, d, stdout);
            }
            else {
                n = html_cref_unicode_encode_utf8(c, u);
                ASSERT(n < 5);

                if (FLAGS_HAS(print_output))
                    fwrite(u, 1, n, stdout);
                q ++;
                d ++;
            }
        }
        else
        if (ISALNUM(q[1])) {
            const uchar_t* t;
            size_t j;
            bool b;
            int i;

#ifdef TIMINGS
            process_cref_count ++;
#endif
            if ((i = process_cref(q + 1)) < 0) {
                if (FLAGS_HAS(warn_invalid))
                    process_cref_warn(
                        p, 0, true, flags);
                if (FLAGS_HAS(print_output))
                    fwrite(p, 1, 2, stdout);
                q += 2;
            }
            else {
                j = INT_AS_SIZE(i);
                ASSERT(j < html_cref_table_size);
                t = &html_cref_table[j];

                d = (*t & ~mask) + 1;
                b = !FLAGS_HAS(strict_semis) &&
                    (*t & mask);

                if (q[d] != ';' && !b) {
                    if (FLAGS_HAS(warn_invalid))
                        process_cref_warn(
                            p, 0, true, flags);
                    if (FLAGS_HAS(print_output))
                        fwrite(q, 1, d, stdout);
                }
                else {
                    if (FLAGS_HAS(print_output))
                        fwrite(t + 2, 1, t[1], stdout);
                }

                q += d;
                if (*q == ';') q ++;
            }
            d = PTR_DIFF(q, p);
        }
        else {
            if (FLAGS_HAS(print_output))
                fwrite(p, 1, 2, stdout);
            q += 2;
            d = 2;
        }

        ASSERT_SIZE_SUB_NO_OVERFLOW(l, d);
        l -= d;
        p = q;
    }
    if (FLAGS_HAS(print_output))
        fwrite(p, 1, l, stdout);
}

static inline void pretty_print_cref(
    const uchar_t* ptr,
    size_t len)
{
    const uchar_t *p, *e;

    ASSERT(ptr != NULL);
    ASSERT(len > 0);

    for (p = ptr,
         e = p + len;
         p < e;
         p ++) {
        if (ISASCII(*p))
            pretty_print_char(stdout, *p, 0);
        else
            fprintf(stdout, "\\%03o", *p);
    }
}

#ifdef BUILTIN
#undef  PROCESS_CREF_
#define PROCESS_CREF_(n) html_cref_ ## n ## _lookup
#endif

static void process_print_cref(
    char* buf, size_t len,
#ifndef BUILTIN
    process_cref_func_t process_cref,
#endif
    enum process_cref_flags_t flags)
{
    const size_t mask = SZ(1) << 7;
    const char *p = buf;
    const uchar_t* t;
    char *q, *e;
    size_t d, j;
    int i;

    ASSERT(buf != NULL);
    ASSERT(len > 0);

    ASSERT(FLAGS_HAS_ONE(
        warn_invalid, print_output));

    e = buf + len;
    ASSERT(*e == 0);

    do {
        // stev: the module functions
        // 'html_cref_*_lookup*' need
        // their input argument to be
        // NUL-terminated
        if ((q = strchr(p, '\n')))
            *q = 0;
        else
            q = e;

        d = PTR_DIFF(q, p);

        if ((i = process_cref(p)) < 0) {
        not_found:
            if (FLAGS_HAS(warn_invalid))
                process_cref_warn(
                    p, d, false, flags);
        }
        else {
            j = INT_AS_SIZE(i);
            ASSERT(j < html_cref_table_size);
            t = &html_cref_table[j];

            if ((*t & ~mask) != d)
                goto not_found;

            if (FLAGS_HAS(print_output)) {
                fprintf(stdout, "%s ", p);
                pretty_print_cref(t + 2, t[1]);
                if (*t & mask) fputs(" *", stdout);
                fputc('\n', stdout);
            }
        }

        if (q < e) q ++;
    } while ((p = q) < e);
}

#undef FLAGS_HAS_ONE
#undef FLAGS_HAS

struct input_liner_t
{
    FILE*  file;
    bits_t opened: 1;
};

static void input_liner_init(
    struct input_liner_t* input,
    const char* file_name)
{
    if (file_name == NULL)
        input->file = stdin;
    else
    if (!(input->file = fopen(file_name, "r")))
        error("opening '%s' failed: %s",
            file_name, strerror(errno));

    input->opened = file_name != NULL; 
}

static void input_liner_done(
    struct input_liner_t* input)
{
    if (input->opened)
        fclose(input->file);
}

typedef
    void (*process_buf_func_t)(
        char* buf, size_t len,
#ifndef BUILTIN
        process_cref_func_t process_cref,
#endif
        enum process_cref_flags_t flags);

static bool input_liner_process(
    struct input_liner_t* input,
    process_buf_func_t process_buf,
#ifndef BUILTIN
    process_cref_func_t process_cref,
#endif
    enum process_cref_flags_t flags)
{
    char* b = NULL;
    size_t n = 0;
    ssize_t r;

    while ((r = getdelim(
        &b, &n, '\n', input->file)) >= 0) {
        size_t l = INT_AS_SIZE(r);

        ASSERT(b != NULL);
        ASSERT(n > 0);

        ASSERT(l > 0);
        ASSERT(l < n);

        process_buf(b, l,
#ifndef BUILTIN
            process_cref,
#endif
            flags);
    }

    if (b != NULL) {
        ASSERT(n > 0);
        free(b);
    }

    return true;
}

#define input_sponge_t    file_buf_t
#define input_sponge_init file_buf_init
#define input_sponge_done file_buf_done

#undef  CASE
#define CASE(n) [file_buf_error_file_ ## n] = #n

static void input_sponge_error(
    const struct input_sponge_t* input)
    NORETURN;

static void input_sponge_error(
    const struct input_sponge_t* input)
{
    static const char* const types[] = {
        CASE(open),
        CASE(stat),
        CASE(read),
        CASE(close),
    };
    const char* t = ARRAY_NULL_ELEM(types,
        input->error_info.type);

    ASSERT(t != NULL);

    error("%s: %s error: %s",
        input->file_name
        ? input->file_name : stdin_name,
        t, strerror(input->error_info.sys_error));
}

static bool input_sponge_process(
    struct input_sponge_t* input,
    process_buf_func_t process_buf,
#ifndef BUILTIN
    process_cref_func_t process_cref,
#endif
    enum process_cref_flags_t flags)
{
    if (input->error_info.type !=
            file_buf_error_none)
        input_sponge_error(input);

    process_buf(
        PTR_CHAR_CAST(input->ptr),
        input->size,
#ifndef BUILTIN
        process_cref,
#endif
        flags);

    return true;
}

typedef
    void (*input_done_func_t)(void*);
typedef
    bool (*input_process_func_t)(void*,
        process_buf_func_t,
#ifndef BUILTIN
        process_cref_func_t,
#endif
        enum process_cref_flags_t);

struct input_t
{
    union {
        struct input_liner_t  liner;
        struct input_sponge_t sponge;
    };

    void                *impl;
    input_done_func_t    done;
    input_process_func_t process;
};

#define INPUT_INIT(n, ...)            \
    do {                              \
        input->done =                 \
            (input_done_func_t)       \
             input_ ## n ## _done;    \
        input->process =              \
            (input_process_func_t)    \
             input_ ## n ## _process; \
        input_ ## n ## _init(         \
            input->impl = &input->n,  \
            opts->input_file,         \
            ## __VA_ARGS__);          \
    } while (0)

static void input_init(
    struct input_t* input,
    const struct options_t* opts)
{
    memset(input, 0, sizeof(*input));

    switch (opts->input_type) {
    case options_input_type_liner:
        INPUT_INIT(liner);
        break;
    case options_input_type_sponge:
        INPUT_INIT(sponge, opts->sponge_max);
        break;
    default:
        UNEXPECT_VAR("%d", opts->input_type);
    }
}

static void input_done(
    struct input_t* input)
{
    input->done(input->impl);
}

static bool input_process(
    struct input_t* input,
    process_buf_func_t process_buf,
#ifndef BUILTIN
    process_cref_func_t process_cref,
#endif
    enum process_cref_flags_t flags)
{
    return input->process(
        input->impl,
        process_buf,
#ifndef BUILTIN
        process_cref,
#endif
        flags);
}

#ifndef BUILTIN

typedef
    size_t (*get_version_func_t)(void);

struct module_lib_funcs_t
{
    get_version_func_t  get_version;

    process_cref_func_t lookup;
    process_cref_func_t parse;
#ifdef TIMINGS
    process_cref_func_t lookup2;
    process_cref_func_t parse2;
#endif
};

#define module_lib_t dyn_lib_t

#undef  CASE
#define CASE(n) \
    DYN_LIB_ENTRY(module_lib_funcs_t, n)
static const struct dyn_lib_entry_t
module_lib_entries[] = {
    CASE(get_version),
    CASE(lookup),
    CASE(parse),
#ifdef TIMINGS
    CASE(lookup2),
    CASE(parse2),
#endif
};

#undef  CASE
#define CASE(n) case dyn_lib_error_ ## n

static void module_lib_print_error_desc(
    struct dyn_lib_error_info_t* error,
    FILE* file)
{
    switch (error->type) {

    CASE(invalid_lib_name):
        fputs("invalid library name", file);
        break;
    CASE(load_lib_failed):
        fputs("failed loading library", file);
        break;
    CASE(symbol_not_found): {
        const struct dyn_lib_entry_t* e =
            ARRAY_NULL_ELEM_REF(
                module_lib_entries,
                error->sym);

        ASSERT(e != NULL);
        fprintf(file,
            "'%s' symbol not found", e->name);
        break;
    }
    CASE(wrong_lib_version):
        // stev: not using 'error->ver' since
        // a more detailed error message is
        // produced by the calling function
        // 'module_lib_error's argument 'msg'
        fputs("wrong library version", file);
        break;

    default:
        UNEXPECT_VAR("%d", error->type);
    }
}

static void module_lib_error(
    struct module_lib_t* lib,
    struct dyn_lib_error_info_t* info,
    const char* msg)
    NORETURN;

static void module_lib_error(
    struct module_lib_t* lib,
    struct dyn_lib_error_info_t* info,
    const char* msg)
{
    error_fmt("%s: module lib error: ",
        lib->lib_name);
    module_lib_print_error_desc(
        info, stderr);
    if (msg != NULL)
        fprintf(stderr, ": %s\n", msg);
    else
        fputc('\n', stderr);
    exit(1);
}

static char* module_lib_make_name(
    const char* base)
{
    char* s = NULL;
    int r;

    ASSERT(base != NULL);

    r = asprintf(&s,
            "html-cref-%s.so", base);
    if (r < 0)
        return NULL;

    ASSERT(r > 0);
    ASSERT(s != NULL);

    return s;
}

#ifndef TIMINGS
#define LOOKUP(l) (l.lookup)
#define PARSE(l)  (l.parse)
#else 
#define LOOKUP(l) \
    (opts->timings ? l.lookup2 : l.lookup)
#define PARSE(l) \
    (opts->timings ? l.parse2 : l.parse)
#endif // TIMINGS

#define NONE(n) \
    ({ UNEXPECT_VAR("%d", n); NULL; })

static process_cref_func_t
    module_lib_load(
        struct module_lib_t* lib,
        const struct options_t* opts)
{
    static const struct dyn_lib_def_t def = {
        .ver_major = HTML_CREF_VERSION_MAJOR,
        .ver_minor = HTML_CREF_VERSION_MINOR,
        .n_entries = ARRAY_SIZE(module_lib_entries),
        .entries = module_lib_entries,
    };
    struct dyn_lib_error_info_t e;
    struct module_lib_funcs_t l;
    char *m = NULL, *n;

    n = module_lib_make_name(opts->cref_parser);
    ASSERT(n != NULL);

    if (!dyn_lib_init(lib, n, &def, &l, &e, &m))
        module_lib_error(lib, &e, m);
    ASSERT(m == NULL);

    return
        opts->action == options_print_cref_action
      ? LOOKUP(l)
      : opts->action == options_subst_cref_action
      ? PARSE(l)
      : NONE(opts->action);
}

static void module_lib_done(
    struct module_lib_t* lib)
{
    free(CONST_CAST(lib->lib_name, char));
    dyn_lib_done(lib);
}

#endif // BUILTIN

#ifdef TIMINGS
struct clocks_t clocks;

static void timings_init(
    const struct options_t* opts)
{
    clocks_init(&clocks, opts->timings);
}

static void timings_adjust(
    const struct options_t* opts)
{
    struct clocks_t o;

    STATIC(
        ARRAY_SIZE(opts->overhead) == 3);
    STATIC(
        SIZE_MAX <= CLOCKS_TIME_MAX);

    o.types = clocks.types;
    o.real = opts->overhead[0];
    o.process = opts->overhead[1];
    o.thread = opts->overhead[2];

    clocks_adjust(
        &clocks, &o, process_cref_count);
}

static void timings_print(void)
{
    clocks_print(
        &clocks, "timings", 15,
        stderr);
}
#endif // TIMINGS

int main(int argc, char* argv[])
{
#undef  CASE
#define CASE(n)                       \
    [options_ ## n ## _cref_action] = \
     process_ ## n ## _cref
    static const process_buf_func_t funcs[] = {
        CASE(subst),
        CASE(print)
    };

    const struct options_t* opts =
        options(argc, argv);
    enum process_cref_flags_t flags = 0;
#ifndef BUILTIN
    process_cref_func_t cref_func;
#endif
    process_buf_func_t buf_func;
#ifndef BUILTIN
    struct module_lib_t lib;
#endif
    struct input_t input;
    bool r;

    buf_func = ARRAY_NULL_ELEM(funcs, opts->action);
    ASSERT(buf_func != NULL);

#ifndef BUILTIN
    cref_func = module_lib_load(&lib, opts);
    ASSERT(cref_func != NULL);
#endif

    if (opts->semicolons)
        flags |= process_cref_strict_semis;
    if (opts->warnings)
        flags |= process_cref_warn_invalid;
    if (opts->warnings < 2)
        flags |= process_cref_print_output;

#ifdef TIMINGS
    if (opts->timings)
        timings_init(opts);
#endif

    input_init(&input, opts);
    r = input_process(&input,
            buf_func,
#ifndef BUILTIN
            cref_func,
#endif
            flags);
    input_done(&input);

#ifdef TIMINGS
    if (opts->timings) {
        timings_adjust(opts);
        timings_print();
    }
#endif

#ifndef BUILTIN
    module_lib_done(&lib);
#endif

    return !r;
}


