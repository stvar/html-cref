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

#ifndef __DYN_LIB_H
#define __DYN_LIB_H

#include <stdbool.h>

#include "json-defs.h"

struct dyn_lib_version_t
{
    unsigned short major;
    unsigned short minor;
    unsigned short patch;
};

enum dyn_lib_error_type_t
{
    dyn_lib_error_invalid_lib_name,
    dyn_lib_error_load_lib_failed,
    dyn_lib_error_symbol_not_found,
    dyn_lib_error_wrong_lib_version,
};

struct dyn_lib_error_info_t
{
    enum dyn_lib_error_type_t type;
    union {
        size_t sym; // symbol_not_found
        struct dyn_lib_version_t
               ver; // wrong_lib_version
    };
};

struct dyn_lib_t
{
    const char* lib_name;
    void* sobj;
};

struct dyn_lib_entry_t
{
    const char* name;
    size_t      offset;
};

struct dyn_lib_def_t
{
    const char* prefix;
    const struct dyn_lib_entry_t*
             entries;
    size_t n_entries;
    size_t ver_major;
    size_t ver_minor;
};

// stev: '*err_msg' bellow is allocated dynamically;
// therefore the user is responsible for freeing it!

JSON_API bool dyn_lib_init(struct dyn_lib_t* dl,
    const char* lib, const struct dyn_lib_def_t* def,
    void* obj, struct dyn_lib_error_info_t* err_info,
    char** err_msg);

JSON_API void dyn_lib_done(struct dyn_lib_t* dl);

#define DYN_LIB_ENTRY(s, n)             \
    {                                   \
        .name = #n,                     \
        .offset = offsetof(struct s, n) \
    }

#endif /* __DYN_LIB_H */


