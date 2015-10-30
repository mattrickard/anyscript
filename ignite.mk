
# ignite.mk
# Copyright (C) 2015 Matthew Rickard

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

ifndef IGNITE_MK
IGNITE_MK := 1

include tminus1.mk

libignite.$(SO): UnitTest.o \
  diff.o \
  copyFile.o \
  Utils.o \
  Scorecard.o \
  UnitTestSuite.o \
  logMsg.o \
  MacroNames.o \
  GetOption.o \
  AbstractProcess.o
	$(linkSharedLib) -lboost_regex

endif

