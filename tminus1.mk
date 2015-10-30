
# tminus1.mk
# Copyright (C) 2003,2015 Matthew Rickard

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

ifndef TMINUS1_MK
TMINUS1_MK := 1

CXX := /usr/bin/g++ -fPIC
CC := /usr/bin/gcc -fPIC
LINK.o := $(CXX)

anyLibArgs = $(sort $(foreach x, $(dir $(1)), -L$(x))) $(foreach x, $(notdir $(1)), $(patsubst %.a, %, -l$(patsubst %.$(SO), %, $(patsubst lib%, %, $(x)))))
sharedLibArgs = $(sort $(foreach x, $(dir $(1)), -Wl,-rpath,$(x))) $(call anyLibArgs, $(1))
linkerArgs = $(filter %.o, $(1)) $(call sharedLibArgs, $(filter %.$(SO), $(1))) $(call anyLibArgs, $(filter %.a, $(1)))
ifeq ($(OS),Windows_NT)
  SO := dll
  STACKSIZE := -Wl,--stack,50000000
  ENABLE_AUTO_IMPORT := -Wl,--enable-auto-import,--exclude-libs,ALL
else
  SO := so
endif

# These three are declared as multilines so you can add extra lines such as an echo for debugging

define linkProgram
$(strip $(LINK.o) $(call linkerArgs, $^ $|) $(ENABLE_AUTO_IMPORT) $(LDLIBS) -o $@ $(STACKSIZE))
endef

define linkSharedLib
$(strip $(LINK.o) $(call linkerArgs, $^ $|) $(ENABLE_AUTO_IMPORT) $(LDLIBS) -o $@ -shared)
endef

define linkArchive
rm -f $@; ar rc $@ $(filter %.o, $^)
endef

endif

