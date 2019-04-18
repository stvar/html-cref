# Copyright (C) 2019  Stefan Vargyas
# 
# This file is part of Html-Cref.
# 
# Html-Cref is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Html-Cref is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Html-Cref.  If not, see <http://www.gnu.org/licenses/>.

.PHONY:: default clean allclean all depend

default: all

PROGRAM := html-cref

MODULES := ietrie \
           iwtrie \
           itrie \
           etrie \
           wtrie \
           trie \
           re2c

JSON_TYPE_HOME := ../lib/json-type
JSON_TYPE_LIB  := ${JSON_TYPE_HOME}/lib

SRCS := ${JSON_TYPE_LIB}/su-size.c \
        ${JSON_TYPE_LIB}/pretty-print.c \
        ${JSON_TYPE_LIB}/file-buf.c \
        ${JSON_TYPE_LIB}/dyn-lib.c \
        html-cref-unicode.c \
        html-cref-table.c \
        html-cref.c \
        common.c

MODS := ${MODULES:%=html-cref-%.so}

BIN := ${PROGRAM}

# GCC parameters

GCC := gcc
GCC_STD := gnu99

CFLAGS := -Wall -Wextra -std=${GCC_STD} \
          -g -I. -I${JSON_TYPE_HOME} -I${JSON_TYPE_LIB} \
          -fdata-sections -ffunction-sections -Wl,--gc-sections \
          -fPIC -fvisibility=hidden \
          -DPROGRAM=${PROGRAM} \
          -DJSON_DEBUG \
          -DDEBUG

ifdef OPT
ifneq ($(words ${OPT}),1)
$(error invalid OPT='${OPT}')
endif
ifneq ($(filter-out 0 1 2 3,${OPT}),)
$(error invalid OPT='${OPT}')
endif
CFLAGS += -O$(strip ${OPT})
endif

ifdef TIMINGS
ifneq ($(words ${TIMINGS}),1)
$(error invalid TIMINGS='${TIMINGS}')
endif
ifneq ($(filter-out no yes,${TIMINGS}),)
$(error invalid TIMINGS='${TIMINGS}')
endif
ifeq ($(strip ${TIMINGS}),yes)
SRCS += clocks.c
CFLAGS += -DTIMINGS

${MODS}: clocks.c
endif
endif

ifdef BUILTIN
ifneq ($(words ${BUILTIN}),1)
$(error invalid BUILTIN='${BUILTIN}')
endif
ifneq ($(filter-out ${MODULES},${BUILTIN}),)
$(error invalid BUILTIN='${BUILTIN}')
endif
SRCS += html-cref-${BUILTIN}.c
CFLAGS += -DBUILTIN=${BUILTIN} \
          -DBUILTIN_HEADER=\"html-cref-${BUILTIN}.h\"
endif

# dependency rules

ifeq (.depend, $(wildcard .depend))
include .depend
endif

# building rules

${BIN}:
	${GCC} ${CFLAGS} -ldl ${SRCS} -o $@

${MODS}: %.so: %.c
	${GCC} ${CFLAGS} -DHTML_CREF_MODULE -lrt -shared $^ -o $@ 

# main targets

ifndef BUILTIN
all: ${BIN} ${MODS}
else
all: ${BIN}
endif

clean:
	rm -f *~

allclean: clean
	rm -f ${BIN} ${MODS}

depend:
	${GCC} ${CFLAGS} -c ${SRCS} -MM| \
    sed -r 's/^[^ \t]+\.o:/${BIN}:/' > .depend
	${GCC} ${CFLAGS} -c $(MODS:.so=.c) -MM >> .depend


