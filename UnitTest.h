
// UnitTest.h
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

#ifndef UNITTEST_H
#define UNITTEST_H

#include "UnitTestSuite.h"
#include "Scorecard.h"
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <string>

#ifndef NEW_LINE
#if defined _WIN32 && !defined __CYGWIN__
#define NEW_LINE "\r\n"
#else
#define NEW_LINE '\n'
#endif
#endif

#define testEqual(a, b)   (setLine(currentLine), testEqual_f(a, b))
#define testBetween(a, b, c)   (setLine(currentLine), testBetween_f(a, b, c))
#define testDiff(a, b, c) (setLine(currentLine), testDiff_f(a, b, c))
#define testLess(a, b)      (setLine(currentLine), testLess_f(a, b))
#define testApprox(a, b, c)      (setLine(currentLine), testApprox_f(a, b, c))
#define testTrue(a)       (setLine(currentLine), testTrue_f(a))
#define testTrueMsg(a, b) MACROBLOCK(if (!a) { std::cerr << currentLine << ": " << b << NEW_LINE; testTrue_f(false);})
#define testMatch(a, b)   (setLine(currentLine), testMatch_f(a, b))

#define currentLine FileLine(__FILE__, __LINE__)

class FileLine {
public:
  FileLine() { }
  FileLine(const char *file, int line);
  const char *file;
  int line;
  operator std::string();
};

std::ostream &operator <<(std::ostream &o, const FileLine &sl);

class NameAndFile {
public:
  NameAndFile(const char *name, int fd);
  NameAndFile(const char *name, std::istream &);
  NameAndFile(const std::string &name, std::istream &);
  NameAndFile(const char *name, const char *str);
  NameAndFile(const char *name, const std::string &str);
  NameAndFile(const char *name);
  NameAndFile(const NameAndFile &); // beware that removing this unused copy constructor causes
                                    // painful "is implicitly deleted because the default definition
                                    // would be ill-formed" or "private within this context" messages
  const char *filename;
  int fd;
  std::istream *istreamPtr;
  std::ostream *ostreamPtr;
  std::istream &getIStream();
  std::ostream &getOStream();
  std::filebuf *bufPtr;
  std::istringstream iss;
  ~NameAndFile();
  int deleteIstreamPtr;
};

class UnitTest: public UnitTestSuite {
public:
  virtual void PreSetup();
  virtual void Setup();
  virtual void TearDown();
  virtual void run() = 0;
  template<typename T, typename U> void testEqual_f(const T &x, const U &shouldBe);
  void testApprox_f(double x, double shouldBe, double tolerance);
  void testBetween_f(double x, double from, double to);
  template<typename T, typename U> void testLess_f(const T &x, const U &upperLimit);
  void testEqual_f(std::string x, std::string shouldBe); // this is better for strings containing nulls
  void testEqual_f(const char *x, const char *shouldBe); // this may have to remain just to pacify the argument overloading
  void testEqual_f(std::string x, const char *shouldBe); // this too
  void testMatch_f(std::string x, std::string shouldBe); // regular expression matching
  void testDiff_f(NameAndFile f, NameAndFile shouldBe, int flags);
  void testTrue_f(bool result);
  void setLine(const FileLine &fileLine);
  virtual void addSubTests();
  virtual int getLevel();
private:
  FileLine fileLine;
};

extern UnitTest *ut;

template<typename T, typename U> inline void UnitTest::testEqual_f(const T &x, const U &shouldBe) {
  if (x == shouldBe) {
    scorecard->recordPass();
  }
  else {
    std::cout << fileLine << ": " << x << " != " << shouldBe << NEW_LINE;
    scorecard->recordFail();
  }
}

template<typename T, typename U> inline void UnitTest::testLess_f(const T &x, const U &upperLimit) {
  if (x < upperLimit) {
    scorecard->recordPass();
  }
  else {
    std::cout << fileLine << ": " << x << " >= " << upperLimit << NEW_LINE;
    scorecard->recordFail();
  }
}

#endif

