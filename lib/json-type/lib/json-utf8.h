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

#ifndef __JSON_UTF8_H
#define __JSON_UTF8_H

#include <stdint.h>

enum json_code_point_type_t
{
    json_basic_plane_code_point,
    json_high_surrogate_code_point,
    json_low_surrogate_code_point
};

struct json_surrogate_pair_t
{
    uint16_t high;
    uint16_t low;
};

enum json_code_point_type_t json_encode_utf8(
    struct json_surrogate_pair_t* surrogates,
    uchar_t* buf, size_t* len);

void json_encode_utf8_surrogate_pair(
    const struct json_surrogate_pair_t* surrogates, uchar_t* buf);

bool json_validate_utf8(
    const uchar_t* buf, size_t len, size_t* err);

void json_print_escaped_utf8(
    const uchar_t* buf, size_t len, bool surrogates, FILE* file);

enum {
    json_utf8_ascii_size = 0x80u,
    json_utf8_esc_bit    = 0x80u,
};

const uchar_t json_utf8_escapes[json_utf8_ascii_size];

#endif/*__JSON_UTF8_H*/


