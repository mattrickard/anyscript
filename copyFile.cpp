
// copyFile.cpp
// Copyright (C) 2004, 2015 Matthew Rickard

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

#include "copyFile.h"
#include <iostream>
#include "logMsg.h"
#include <errno.h>
#include "Utils.h"
#include <stdio.h>

int renameFile(const char *from, const char *to) {

  // This renames files the Unix way. Windows fails if the target exists. This function removes the target if that occurs and retries.

  int result = rename(from, to);
#if defined _WIN32 || defined __CYGWIN__
  if (result && (errno == EEXIST || errno == EACCES)) { // it was EEXIST, recently added EACCES for Cygwin
    //logMsg << "Let's remove " << to << " and try again." << std::endl;
    int removeResult = remove(to);
    if (removeResult) {
      logMsg << "Now we're really struggling." << std::endl;
      //lineabort();
      return removeResult;
    }
    result = rename(from, to);
  }
#endif
  return result;
}

// return code: number of bytes, or -1 if something went wrong

int copyFile(std::istream &from, std::ostream &to) {
  char buf[1024];
  int bytes(0);
  do {
    if (from.bad()) {
      lineabort();
      return -1;
    }
    if (to.bad()) {
      lineabort();
      return -1;
    }
    from.read(buf, 1024);
    bytes += from.gcount();
    to.write(buf, from.gcount());
  } while (from.good());
  to.flush();
  return bytes;
}

