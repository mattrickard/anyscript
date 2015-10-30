
// UnitTest.cpp
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

#include "UnitTest.h"
#include "diff.h"
#ifdef __GNUC__
#include <ext/stdio_filebuf.h>
#endif
#include <fstream>
#include <boost/regex.hpp>
#include "logMsg.h"
#include <string.h>
#include "sys/stat.h"
#include "copyFile.h"
#include <stdlib.h>
#include "Utils.h"
#include <iostream>
#include <sstream>

UnitTest *ut(NULL);

void UnitTest::testEqual_f(const char *x, const char *shouldBe) {
  if (strcmp(x, shouldBe))
    testEqual_f(std::string(x), std::string(shouldBe));
  else
    scorecard->recordPass();
}

void UnitTest::testEqual_f(std::string x, const char *shouldBe) {
  testEqual_f(x, std::string(shouldBe));
}

void UnitTest::testEqual_f(std::string x, std::string shouldBe) {
  if (x != shouldBe) {
    if (getenv("QUOTE")) {
      std::string quotedx(quote(x.c_str()));
      std::string quotedShouldBe(quote(shouldBe.c_str()));
      std::cout << fileLine << ":\n\"" << quotedx << "\" != \n\"" << quotedShouldBe << '"' << NEW_LINE;
    }
    else
      std::cout << fileLine << ' ' << x << " != " << shouldBe << NEW_LINE;
    scorecard->recordFail();
  }
  else
    scorecard->recordPass();
}

void UnitTest::testMatch_f(std::string x, std::string shouldBe) {

  boost::regex e(shouldBe);
  boost::smatch match;
  bool result = boost::regex_match(x, match, e);

  if (!result) {
    if (getenv("QUOTE")) {
      std::string quotedx(quote(x.c_str()));
      std::string quotedShouldBe(quote(shouldBe.c_str()));
      std::cout << fileLine << ":\n\"" << quotedx << "\" does not match \n\"" << quotedShouldBe << '"' << NEW_LINE;
    }
    else
      std::cout << fileLine << ":\n\"" << x << "\" does not match \n\"" << shouldBe << '"' << NEW_LINE;
    scorecard->recordFail();
  }
  else {
    scorecard->recordPass();
  }
}

bool exists(const char *filename) {
  struct stat statbuf;
  int result = stat(filename, &statbuf);
  return !result;
}

void UnitTest::testDiff_f(NameAndFile nafOut, NameAndFile nafShouldBe, int flags) {
  if (getenv("NODIFF"))
    return;
  if (!nafOut.getIStream().good()) {
    std::cerr << "Error: " << logValue(nafOut.filename) << " is a bad file" << std::endl;
    testEqual(nafOut.getIStream().good(), 1);
  }
  if (!nafShouldBe.getIStream().good()) {
    logMsg << "Error: " << logValue(nafShouldBe.filename) << " is a bad file" << std::endl;
    if (getenv("REPLACE")) {
      nafOut.getIStream().clear();
      nafOut.getIStream().seekg(0);
      int bytes = copyFile(nafOut.getIStream(), nafShouldBe.getOStream());
      logMsg << "Copied " << bytes << " bytes\n";
      //if (bytes < 1) {
        //logMsg << "That's not a heck of a lot. aborting" << std::endl;
        //lineabort();
      //}
      lineabort(); // then abort anyway
    }
  }
  std::ostringstream oss;
  std::string heading(std::string("Differences between ") + nafShouldBe.filename + " " + nafOut.filename + " (" + std::string(fileLine) + ")\n");

  bool result(!diff(nafShouldBe.getIStream(), nafOut.getIStream(), oss, flags, heading));
  if (!result) {
    std::cout << '\n';
    if (getenv("SHOWALL")) {
      std::cout << "testDiff(): difference found between expected and output. Output is:" << '\n';
      nafOut.getIStream().clear();
      nafOut.getIStream().seekg(0);
      std::string str;
      while (getline(nafOut.getIStream(), str))
        std::cout << str << '\n';
    }
    std::cout << oss.str() << heading;
    if (dynamic_cast<std::ifstream *>(&nafOut.getIStream()) || dynamic_cast<std::fstream *>(&nafOut.getIStream())
     || dynamic_cast<std::ifstream *>(&nafShouldBe.getIStream()) || dynamic_cast<std::fstream *>(&nafShouldBe.getIStream())) {
      if (getenv("WINMERGE")) {
        std::ostringstream oss;
#ifdef linux
        oss << "meld " << nafShouldBe.filename << ' ' << nafOut.filename;
#else
        logMsg << logValue(getenv("PROGRAMFILES")) << std::endl;
        if (exists("c:/Program Files (x86)/Winmerge"))
          oss << "\"c:/Program Files (x86)/Winmerge/winmergeu\" " << nafShouldBe.filename << ' ' << nafOut.filename;
        else
          oss << "\"c:/Program Files/Winmerge/winmergeu\" " << nafShouldBe.filename << ' ' << nafOut.filename;
#endif
        std::cout << "Running " << oss.str() << std::endl;
        system(oss.str().c_str());
      }
      if (getenv("REPLACE")) {
        nafOut.getIStream().clear();
        nafOut.getIStream().seekg(0);
        int bytes = copyFile(nafOut.getIStream(), nafShouldBe.getOStream());
        logMsg << "Copied " << bytes << " bytes\n";
        if (bytes < 1) {
          logMsg << "That's not a heck of a lot. aborting" << std::endl;
          lineabort();
        }
        lineabort(); // then abort anyway
      }
    }
  }
  testTrue_f(result);
}

NameAndFile::NameAndFile(const char *filename): filename(filename), fd(-1), istreamPtr(0), ostreamPtr(0), bufPtr(0), deleteIstreamPtr(0) {
}

NameAndFile::NameAndFile(const char *filename, int fd): filename(filename), fd(fd), istreamPtr(0), ostreamPtr(0), bufPtr(0), deleteIstreamPtr(0) {
}

NameAndFile::NameAndFile(const std::string &filename, std::istream &stream): filename(filename.c_str()), fd(-1), istreamPtr(&stream), ostreamPtr(0), bufPtr(0), deleteIstreamPtr(0) {
}

NameAndFile::NameAndFile(const char *filename, std::istream &stream): filename(filename), fd(-1), istreamPtr(&stream), ostreamPtr(0), bufPtr(0), deleteIstreamPtr(0) {
}

NameAndFile::NameAndFile(const char *filename, char const *str): filename(filename), fd(-1), /*istreamPtr(0),*/ ostreamPtr(0), bufPtr(0), iss(str), deleteIstreamPtr(0) {
  istreamPtr = &iss;
}

NameAndFile::NameAndFile(const char *filename, const std::string &str): filename(filename), fd(-1), istreamPtr(0), ostreamPtr(0), bufPtr(0), iss(str), deleteIstreamPtr(0) {
  istreamPtr = &iss;
}

NameAndFile::~NameAndFile() {
  if (deleteIstreamPtr)
    delete istreamPtr;
  delete ostreamPtr;
  delete bufPtr;
}

std::istream &NameAndFile::getIStream() {
  if (!istreamPtr) {
    if (fd == -1) {
      istreamPtr = new std::fstream(filename, std::ios::in);
    }
    else {
      bufPtr = new __gnu_cxx::stdio_filebuf<char>(fd, std::ios::in);
      istreamPtr = new std::istream(bufPtr);
    }
    deleteIstreamPtr = 1;
  }
  return *istreamPtr;
}

std::ostream &NameAndFile::getOStream() {
  if (!ostreamPtr) {
    if (fd == -1) {
      ostreamPtr = new std::fstream(filename, std::ios::out);
    }
    else {
      logMsg << "Must convert " << logValue(fd) << " into an ostreamPtr" << std::endl;
      bufPtr = new __gnu_cxx::stdio_filebuf<char>(fd, std::ios::out | std::ios::trunc);
      ostreamPtr = new std::ostream(bufPtr);
      logMsg << ostreamPtr->good() << std::endl;
      logMsg << ostreamPtr->bad() << std::endl;
    }
  }
  return *ostreamPtr;
}

void UnitTest::testTrue_f(bool result) {
  if (!result)
    std::cout << fileLine << ": false" << NEW_LINE;
  scorecard->recordResult(result);
}

int UnitTest::getLevel() {
  return 5;
}

void UnitTest::addSubTests() {
}

void UnitTest::setLine(const FileLine &fileLine) {
  this->fileLine = fileLine;
}

void UnitTest::Setup() {
}

void UnitTest::PreSetup() {
}

void UnitTest::TearDown() {
}

void UnitTest::testApprox_f(double x, double shouldBe, double tolerance) {
  if (fabs(x - shouldBe) < tolerance) {
    scorecard->recordPass();
  }
  else {
    std::cout << fileLine << ": " << x << " !~= " << shouldBe << NEW_LINE;
    scorecard->recordFail();
  }
}

void UnitTest::testBetween_f(double x, double from, double to) {
  if (x >= from && x <= to) {
    scorecard->recordPass();
  }
  else {
    std::cout << fileLine << ": " << x << " is not between " << from << " and " << to << NEW_LINE;
    scorecard->recordFail();
  }
}

std::ostream &operator <<(std::ostream &o, const FileLine &sl) {
  o << sl.file << ' ' << sl.line;
  return o;
}

FileLine::operator std::string() {
  std::ostringstream o;
  o << *this;
  return o.str();
}

FileLine::FileLine(const char *file, int line): file(file), line(line) {
}
