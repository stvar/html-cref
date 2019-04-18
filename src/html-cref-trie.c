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
#include <stdbool.h>

#include "common.h"
#include "int-traits.h"

#ifdef TIMINGS
#include "clocks-impl.h"
#endif

#include "html-cref.h"
#include "html-cref-trie.h"

size_t html_cref_trie_get_version(void)
{ return HTML_CREF_VERSION; }

// $ html-cref-gen --gen-func --heading -- -f html_cref_trie_parse > html-cref-trie-impl.h

#include "html-cref-trie-impl.h"

API_ALIAS(trie, parse, lookup)

#ifdef TIMINGS
HTML_CREF_FUNC_DEF(trie, lookup)
HTML_CREF_FUNC_DEF(trie, parse)
#endif


