
// Utils.h
// Copyright (C) 2015 Matthew Rickard

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

#ifndef UTILS_H
#define UTILS_H

#include <string>

/*
    Check manifest defines with 'cpp -xc++ -dM /dev/null' or with ../target/identify
    Good ones are: __CYGWIN__ _WIN32 __MVC_VER
*/

#define lineabort() lineabort_(__FILE__, __LINE__)

#if defined(_WIN32) || defined(__CYGWIN__)
#define _NSIG 123 // TODO: Look this one up
#endif

void ReplaceAll(std::string &s, std::string a, std::string b);
std::string toUpper(std::string a);
const char *BaseName(const char *filename, const char *delimiters = "\\/");
std::string DirName(const char *filename);

template<typename T> std::basic_string<T> GetField(std::basic_string<T> &line, const T *delimiter = (T *)0) {
  if (delimiter && std::basic_string<T>(delimiter).size()) {
    size_t delimiterPos = line.find(delimiter);
    if (delimiterPos == std::basic_string<T>::npos) {
      std::basic_string<T> result = line;
      line.clear();
      return result;
    }
    std::basic_string<T> result = line.substr(0, delimiterPos);
    line.erase(0, delimiterPos + std::basic_string<T>(delimiter).size());
    return result;
  }
  return line;
}

std::string stripWhitespace(std::string a);
std::string stripLeadingWhitespace(std::string a);

#define TIMER_RESET 1

class Timer {
public:
  long long startns;
  Timer();
  long getMicroseconds(int flags = 0);
};

namespace olib {
  extern int argc;
  extern char *const *argv;
};

std::string quote(const char *s);
bool matches(const char *s, const char *pattern);
void showMemoryUsage();
void lineabort_(const char *file, int line) __attribute__((noreturn));

#endif

