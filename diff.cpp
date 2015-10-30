
// diff.cpp
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

#include "diff.h"
#include <list>
#include "Utils.h"
#include <iostream>

class SlaveDiffBuffer {
public:
  SlaveDiffBuffer(std::istream &input, const char *prefix, std::ostream &output);
  bool scan(SlaveDiffBuffer &a);
  void rattleOff();
  bool next();
  void fillBuffer();
  void countBufferedLines();
  void displayLines();
  virtual bool match(SlaveDiffBuffer &b);
  virtual ~SlaveDiffBuffer() { }

  std::list<std::string> strlist;
  std::list<std::string>::iterator ptr;
  std::istream &input;
  const char *const prefix;
  std::ostream &output;
  int spentLines;
  int bufferedLines;
};

class MasterDiffBuffer: public SlaveDiffBuffer {
public:
  MasterDiffBuffer(std::istream &input, const char *prefix, std::ostream &output, int flags);
  bool match(SlaveDiffBuffer &b);

  const int flags;
};

bool diff(SlaveDiffBuffer &a, std::istream &bstream, std::ostream &output, std::string header) {
  SlaveDiffBuffer b(bstream, "> ", output);
  bool result = false;
  for (;;) {
    a.fillBuffer();
    b.fillBuffer();
    if (a.ptr == a.strlist.end())
      if (b.ptr == b.strlist.end())
        return result;
      else
        while (b.next());
    else if (b.ptr == b.strlist.end())
      while (a.next());
    else
      while (a.scan(b) && b.scan(a));
    if (a.ptr == a.strlist.end())
      while (b.next());
    if (b.ptr == b.strlist.end())
      while (a.next());
    a.countBufferedLines();
    b.countBufferedLines();
    if (a.bufferedLines || b.bufferedLines) {
      if (!result && header.length())
    output << header;
      result = true;
      a.displayLines();
      if (a.bufferedLines)
        if (b.bufferedLines)
          output << 'c';
        else
          output << 'd';
      else
        output << 'a';
      b.displayLines();
      output << std::endl;
    }
    a.rattleOff();
    if (a.bufferedLines && b.bufferedLines)
      output << "---\n";
    b.rattleOff();
  }
}

bool diff(std::istream &astream, std::istream &bstream, std::ostream &output, int flags, std::string header) {
  MasterDiffBuffer a(astream, "< ", output, flags);
  return diff(a, bstream, output, header);
}

MasterDiffBuffer::MasterDiffBuffer(std::istream &input, const char *prefix, std::ostream &output, int flags):
    SlaveDiffBuffer(input, prefix, output), flags(flags) {
}

SlaveDiffBuffer::SlaveDiffBuffer(std::istream &input, const char *prefix, std::ostream &output):
    input(input), prefix(prefix), output(output), spentLines(1) {
  ptr = strlist.end();
}

bool SlaveDiffBuffer::match(SlaveDiffBuffer &b) {
  return b.match(*this);
}

std::string collapseId(std::string a) {
  unsigned pos = a.find("$Id: ");
  if (pos != std::string::npos) {
    pos += 3;
    unsigned pos2 = a.find("$", pos + 2);
    a.erase(pos, pos2 - pos);
  }
  return a;
}

bool MasterDiffBuffer::match(SlaveDiffBuffer &b) {
  std::string aa = *ptr;
  std::string bb = *b.ptr;
  if (flags & DIFF_IGNORE_INDENT) {
    aa = stripLeadingWhitespace(aa);
    bb = stripLeadingWhitespace(bb);
  }
  if (flags & DIFF_IGNORE_CVS) {
    aa = collapseId(aa);
    bb = collapseId(bb);
  }
  if (flags & DIFF_IGNORE_CASE) {
    aa = toUpper(aa);
    bb = toUpper(bb);
  }
  if (flags & DIFF_IGNORE_TAIL) {
    aa = stripWhitespace(aa);
    bb = stripWhitespace(bb);
  }
  if (aa == bb)
    return true;
  if (flags & DIFF_REGEXP) {
    return matches(b.ptr->c_str(), ptr->c_str());
  }
  else
    return false;
}

bool SlaveDiffBuffer::scan(SlaveDiffBuffer &x) {
  while (!match(x)) {
    if (!x.next()) {
      x.ptr--;
      return next();
    }
    if (ptr == strlist.begin())
      return true;
    ptr--;
  }
  return false;
}

bool SlaveDiffBuffer::next() {
  ptr++;
  fillBuffer();
  return ptr != strlist.end();
}

void SlaveDiffBuffer::fillBuffer() {
  if (ptr == strlist.end()) {
    std::string s;
    if (getline(input, s)) {
      strlist.push_back(s);
      ptr = strlist.end();
      ptr--;
    }
  }
}

void SlaveDiffBuffer::rattleOff() {
  while (strlist.begin() != ptr) {
    output << prefix << *strlist.begin() << std::endl;
    spentLines++;
    strlist.pop_front();
  }
  if (strlist.begin() != strlist.end()) {
    spentLines++;
    strlist.pop_front();
  }
  ptr = strlist.begin();
}

void SlaveDiffBuffer::countBufferedLines() {
  bufferedLines = 0;
  for (std::list<std::string>::iterator i = strlist.begin(); i != ptr; i++)
    bufferedLines++;
}

void SlaveDiffBuffer::displayLines() {
  if (bufferedLines > 1)
    output << spentLines << ",";
  output << spentLines + bufferedLines - 1;
}

