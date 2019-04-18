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

#include <stddef.h>

#include "common.h"
#include "ptr-traits.h"

#include "html-cref-table.h"

// $ html-cref-gen --gen-sub-table --heading > html-cref-table-impl.h

const uchar_t html_cref_table[] =
#include "html-cref-table-impl.h"
;

const size_t html_cref_table_size =
    ARRAY_SIZE(html_cref_table) - 1;


