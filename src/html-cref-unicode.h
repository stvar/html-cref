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

#ifndef __HTML_CREF_UTF_H
#define __HTML_CREF_UTF_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint32_t code_point_t;

enum { html_cref_unicode_max_code_point = 0x10ffffu };

bool html_cref_unicode_parse_html(const char**, code_point_t*);

typedef unsigned char utf8_t[4];

size_t html_cref_unicode_encode_utf8(code_point_t, utf8_t);

#endif /* HTML_CREF_UNICODE_H */


