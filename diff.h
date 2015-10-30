
// diff.h
// Copyright (C) 2003,2015 Matthew Rickard

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#ifndef DIFF_H
#define DIFF_H

#include <iostream>
#include <string>

#define DIFF_NORMAL         0
#define DIFF_REGEXP         1
#define DIFF_IGNORE_INDENT  2
#define DIFF_IGNORE_TAIL    4
#define DIFF_IGNORE_CVS     8
#define DIFF_IGNORE_CASE   16

// returns true if there are differences
bool diff(std::istream &a, std::istream &b, std::ostream &output,
  int flags = DIFF_NORMAL, std::string header = std::string());

#endif
