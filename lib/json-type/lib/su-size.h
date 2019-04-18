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

#ifndef __SU_SIZE_H
#define __SU_SIZE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include "int-traits.h"

#define KB(x) (SZ(x) * SZ(1024))
#define MB(x) (KB(x) * SZ(1024))

struct su_size_t
{
    size_t      sz;
    const char* su;
};

struct su_size_t su_size(size_t sz);

enum su_size_parse_error_t
{
    su_size_parse_error_invalid_num,
    su_size_parse_error_illegal_num,
};

bool su_size_parse(
    const char* str, size_t min, size_t max,
    size_t* result, enum su_size_parse_error_t* err);

bool su_size_parse_su(
    const char* str, size_t min, size_t max,
    size_t* result, enum su_size_parse_error_t* err);

enum su_size_parse_list_error_t
{
    su_size_parse_list_error_invalid_num,
    su_size_parse_list_error_illegal_num,
    su_size_parse_list_error_invalid_list,
    su_size_parse_list_error_list_overflow,
};

bool su_size_parse_list(
    const char* str, size_t min, size_t max,
    char sep, size_t* list, size_t n_list,
    enum su_size_parse_list_error_t* err,
    const char** ptr, size_t* n_elems);

size_t su_size_to_string(
    char* buf, size_t len, size_t val, bool null);

size_t su_size_to_string_su(
    char* buf, size_t len, size_t val, bool null);

size_t su_size_list_to_string(
    char* buf, size_t len, const size_t* list,
    size_t n_list, const char* sep, bool null);

enum su_size_param_type_t {
    su_size_param_plain,
    su_size_param_su,
};

struct su_size_param_t
{
    enum su_size_param_type_t
                type;
    const char* name;
    const char* desc;
    size_t      min;
    size_t      max;
    size_t      def;
};

void su_size_print_sizes(
    const struct su_size_param_t* params,
    size_t n_params,
    FILE* file);

#endif /* __SU_SIZE_H */


