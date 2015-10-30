
// UnitTestSuite.cpp
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

#include "UnitTestSuite.h"
#include <string>
#include  <typeinfo>
#include <string.h>

std::string UnitTestSuite::getname() const {
  const char *name = typeid(*this).name();
  while (isdigit(*name))
    name++;
  if (!strncmp(name, "class ", 6))
    name += 6;
  else if (!strncmp(name, "struct ", 7))
    name += 7;
  return name;
}

UnitTestSuite::~UnitTestSuite() {
}

UnitTestSuite::UnitTestSuite(): scorecard(0), name("UnitTestSuite") {
}

std::string UnitTestSuite::modifiedName() const {
  std::string s(getname());
  for (unsigned i = 0; i < s.size(); i++)
    if (s[i] == ' ')
      s[i] = '_';
  return s;
}

