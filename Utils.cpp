
// Utils.cpp
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

#include "Utils.h"
#include <iomanip>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <wctype.h>
#include <string>
#include <boost/regex.hpp>
#include <map>
#include <fstream>
#include <iostream>

void ReplaceAll(std::string &s, std::string a, std::string b)
{
  std::string::size_type i = s.find(a);
  while (i != std::string::npos)
  {
    s.replace(i, a.size(), b);
    i = s.find(a, i + b.size());
  }
}

const char *strrcspn(const char *s, const char *cs)
{
  // Find the _last_ occurrence of any of the characters in cs which
  // appear in the string s.  Modelled loosely after strchr, strrchr
  // and strcspn
  for (const char *p = s + strlen(s) - 1; p >= s; p--)
    if (strchr(cs, *p))
      return p;
  return 0;
}

const char *BaseName(const char *filename, const char *delimiters)
{
  // Return the base name of the filename i.e. the filename with any
  // preceding directory names removed
  const char *lastDelimiter = strrcspn(filename, delimiters);
  if (lastDelimiter)
    return lastDelimiter + 1;
  return filename;
}

std::string DirName(const char *filename)
{
  // Return the base name of the filename i.e. the filename with any
  // preceding directory names removed
  const char *lastDelimiter = strrcspn(filename, "\\/");
  if (lastDelimiter)
    return std::string(filename, lastDelimiter - filename);
  return "";
}

std::string toLower(std::string a) {
  for (std::string::iterator i = a.begin(); i != a.end(); i++)
    *i = tolower(*i);
  return a;
}

std::string toUpper(std::string a) {
  for (std::string::iterator i = a.begin(); i != a.end(); i++)
    *i = toupper(*i);
  return a;
}

std::string stripWhitespace(std::string a) {
  while (a.size() && iswspace(a[a.size() - 1]))
    a.erase(a.size() - 1);
  return a;
}

std::string stripLeadingWhitespace(std::string s) {
  unsigned a;
  for (a = 0; a < s.length() && isspace(s[a]); a++);
  return s.substr(a, s.length() - a);
}

void (*cleanup)() = 0;

void lineabort_(const char *file, int line) {
  if (cleanup)
    cleanup();
  // abort() causes a segfault during static initialisation in Cygwin.
  // When this happens, gdb cannot produce a back trace (even when it
  // runs the program, and the *.exe.stackdump file is as always quite
  // unusable)

  fprintf(stderr, "lineabort() called at %s line %d\n", file, line);
  fflush(stderr); // this is required in minw32 for some reason

  #ifdef __MINGW32__
    *(int *)0 = 123;  // This ends up saying 'Segmentation fault'. But, gdb can see it
    //raise(SIGABRT); // No error message but gdb can't see it
    //abort()         // VERY annoying error message "This application has requested the Runtime terminate it in an unusual way ... Please contact the application's support team ..."
  #endif

  //logMsg << "checkpoint" << std::endl;
  abort();
}

long long getNanosecondsNow() {
#ifdef unix
  timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return 1000000000LL * t.tv_sec + t.tv_nsec;
#elif defined _WIN32
  long long t;
  GetSystemTimeAsFileTime((FILETIME *)&t);
  return 100 * t;
#else
  bork bork bork
#endif
}

Timer::Timer() {
  startns = getNanosecondsNow();
}

long Timer::getMicroseconds(int flags) {
  long long endns = getNanosecondsNow();
  long microseconds((endns - startns) / 1000);
  if (flags & TIMER_RESET)
    startns = endns;
  return microseconds;
}

namespace olib {
  int argc;
  char *const *argv;
};

static char nibble(unsigned char x) {
  if (x < 10)
    return '0' + x;
  else
    return 'a' + x - 10;
}

std::string hexPair(unsigned char x) {
  char a[3];
  a[0] = nibble(x >> 4);
  a[1] = nibble(x & 15);
  a[2] = 0;
  return std::string(a);
}

std::string quote(const char *s) {
  //logMsg << "quote() started" << std::endl;
  //logMsg << logValue(s) << std::endl;
  //logMsg << "quote() here goes" << std::endl;
  std::string result;
  while (*s) {
    if (*s >= 32 && *s <= 126)
      result.append(1, *s);
    else if (*s == '\a') result += ("\\a");
    else if (*s == '\b') result += ("\\b");
    else if (*s == '\f') result += ("\\f");
    else if (*s == '\n') result += ("\\n");
    else if (*s == '\r') result += ("\\r");
    else if (*s == '\t') result += ("\\t");
    else if (*s == '\v') result += ("\\v");
    else
      result.append("\\0x").append(hexPair(*s));
    s++;
  }
  //logMsg << "quote() finished. " << logValue(result) << std::endl;
  return result;
}

static std::map<std::string, boost::regex *> reMapx;

bool matches(const char *s, const char *pattern) {
  if (!reMapx.count(pattern))
    reMapx[pattern] = new boost::regex(pattern);
  boost::regex *re(reMapx[pattern]);
  boost::cmatch cmatch;
  bool result = regex_match(s, cmatch, *re);
  return result;
}

void showMemoryUsage() {
#if defined(linux) || defined(__CYGWIN__) // cygwin is new!
  std::ifstream is("/proc/self/status");
  std::string s;
  int vmdata, vmstk;
  while (is >> s) {
    if (s == "VmData:")
      is >> vmdata;
    else if(s == "VmStk:")
      is >> vmstk;
  }
  vmdata *= 1024;
  vmstk *= 1024;
  std::cout << "data " << vmdata << " bytes, stack " << vmstk << " bytes\n";
#endif
}

