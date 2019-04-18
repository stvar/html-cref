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

#ifndef __CLOCKS_H
#define __CLOCKS_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

struct clocks_time_t
{
    size_t secs;
    size_t nsecs;
};

struct clocks_t
{
    struct clocks_time_t real;
    struct clocks_time_t process;
    struct clocks_time_t thread;
};

void clocks_init(
    struct clocks_t* clocks);

void clocks_assign(
    struct clocks_t* clocks,
    const struct clocks_t* clocks2);

void clocks_add(
    struct clocks_t* clocks,
    const struct clocks_t* clocks2);

void clocks_print(
    const struct clocks_t* clocks,
    const char* name, size_t width,
    bool nseconds, FILE* file);

#endif // __CLOCKS_H


