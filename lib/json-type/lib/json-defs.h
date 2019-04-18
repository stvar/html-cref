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

#ifndef __JSON_DEFS_H
#define __JSON_DEFS_H

#ifndef __GNUC__
#error we need a GCC compiler
#elif __GNUC__ >= 4
#define JSON_API __attribute__ ((visibility("default")))
#else
#define JSON_API
#endif

#ifndef JSON_UNSIGNED_CHAR_DEFINED
#define JSON_UNSIGNED_CHAR_DEFINED
typedef unsigned char uchar_t;
#endif

#endif/*__JSON_DEFS_H*/

