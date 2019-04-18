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

#if !defined(SU_SIZE_OPT_NEED_PARSE_SIZE) && \
    !defined(SU_SIZE_OPT_NEED_PARSE_SIZE_SU)
#error neither \
    SU_SIZE_OPT_NEED_PARSE_SIZE nor \
    SU_SIZE_OPT_NEED_PARSE_SIZE_SU were defined
#endif

#ifdef  SU_SIZE_OPT_NEED_STATIC_FUNCS
#define SU_SIZE_OPT_STATIC static
#else
#define SU_SIZE_OPT_STATIC
#endif

#ifdef  SU_SIZE_OPT_NEED_BOOL_FUNCS
#ifndef SU_SIZE_OPT_ERROR_TYPE
#error  SU_SIZE_OPT_ERROR_TYPE not defined
#endif
#define SU_SIZE_OPT_ERROR_RESULT false
#else
#define SU_SIZE_OPT_ERROR_RESULT 0
#endif

#ifndef SU_SIZE_OPT_NAME
#define SU_SIZE_OPT_MAKE_NAME(s)      s
#else
#define SU_SIZE_OPT_MAKE_NAME__(n, s) n ## _ ## s
#define SU_SIZE_OPT_MAKE_NAME_(n, s ) SU_SIZE_OPT_MAKE_NAME__(n, s)
#define SU_SIZE_OPT_MAKE_NAME(s)      SU_SIZE_OPT_MAKE_NAME_(SU_SIZE_OPT_NAME, s)
#endif

#define SU_SIZE_OPT_PARSE_SIZE_OPTARG    SU_SIZE_OPT_MAKE_NAME(parse_size_optarg)
#define SU_SIZE_OPT_PARSE_SU_SIZE_OPTARG SU_SIZE_OPT_MAKE_NAME(parse_su_size_optarg)
#define SU_SIZE_OPT_ERROR_OPTARG(n)      SU_SIZE_OPT_MAKE_NAME(n ## _opt_arg)

#ifndef SU_SIZE_OPT_ERROR_CASE
#define SU_SIZE_OPT_ERROR_CASE(n)                       \
    case su_size_parse_error_ ## n ## _num:             \
        SU_SIZE_OPT_ERROR_OPTARG(n)(opt_name, opt_arg); \
        return SU_SIZE_OPT_ERROR_RESULT
#endif

#undef  CASE
#define CASE SU_SIZE_OPT_ERROR_CASE

#ifdef SU_SIZE_OPT_NEED_BOOL_FUNCS

#ifdef SU_SIZE_OPT_NEED_PARSE_SIZE

SU_SIZE_OPT_STATIC bool
    SU_SIZE_OPT_PARSE_SIZE_OPTARG(
        const char* opt_name, const char* opt_arg,
        size_t min, size_t max,
        SU_SIZE_OPT_ERROR_TYPE* err,
        size_t* result)
{
    enum su_size_parse_error_t e;

    if (su_size_parse(opt_arg, min, max, result, &e))
        return true;

    switch (e) {
    CASE(invalid);
    CASE(illegal);
    default:
        UNEXPECT_VAR("%d", e);
    }
}

#endif // SU_SIZE_OPT_NEED_PARSE_SIZE

#ifdef SU_SIZE_OPT_NEED_PARSE_SIZE_SU

SU_SIZE_OPT_STATIC bool
    SU_SIZE_OPT_PARSE_SU_SIZE_OPTARG(
        const char* opt_name, const char* opt_arg,
        size_t min, size_t max,
        SU_SIZE_OPT_ERROR_TYPE* err,
        size_t* result)
{
    enum su_size_parse_error_t e;

    if (su_size_parse_su(opt_arg, min, max, result, &e))
        return true;

    switch (e) {
    CASE(invalid);
    CASE(illegal);
    default:
        UNEXPECT_VAR("%d", e);
    }
}

#endif // SU_SIZE_OPT_NEED_PARSE_SIZE_SU

#else // SU_SIZE_OPT_NEED_BOOL_FUNCS

#ifdef SU_SIZE_OPT_NEED_PARSE_SIZE

SU_SIZE_OPT_STATIC size_t
    SU_SIZE_OPT_PARSE_SIZE_OPTARG(
        const char* opt_name, const char* opt_arg,
        size_t min, size_t max)
{
    enum su_size_parse_error_t e;
    size_t v;

    if (su_size_parse(opt_arg, min, max, &v, &e))
        return v;

    switch (e) {
    CASE(invalid);
    CASE(illegal);
    default:
        UNEXPECT_VAR("%d", e);
    }
}

#endif // SU_SIZE_OPT_NEED_PARSE_SIZE

#ifdef SU_SIZE_OPT_NEED_PARSE_SIZE_SU

SU_SIZE_OPT_STATIC size_t
    SU_SIZE_OPT_PARSE_SU_SIZE_OPTARG(
        const char* opt_name, const char* opt_arg,
        size_t min, size_t max)
{
    enum su_size_parse_error_t e;
    size_t v;

    if (su_size_parse_su(opt_arg, min, max, &v, &e))
        return v;

    switch (e) {
    CASE(invalid);
    CASE(illegal);
    default:
        UNEXPECT_VAR("%d", e);
    }
}

#endif // SU_SIZE_OPT_NEED_PARSE_SIZE_SU

#endif // SU_SIZE_OPT_NEED_BOOL_FUNCS

#undef CASE


