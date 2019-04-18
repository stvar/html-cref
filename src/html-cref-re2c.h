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

#ifndef __HTML_CREF_RE2C_H
#define __HTML_CREF_RE2C_H

API size_t html_cref_re2c_get_version(void);

API int html_cref_re2c_lookup(const char*);
API int html_cref_re2c_parse(const char*);

#ifdef TIMINGS
API int html_cref_re2c_lookup2(const char*);
API int html_cref_re2c_parse2(const char*);
#endif

#endif /* __HTML_CREF_RE2C_H */


