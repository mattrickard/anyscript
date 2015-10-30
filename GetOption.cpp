
// GetOption.cpp
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

#include "GetOption.h"
#include <getopt.h>
#include "Utils.h"

/*

    getopt provides the impressive arguments and optional arguments facilities. It also provides some malarkey:

    * different behaviour on errors
    * re-ordering arguments
    * error messages

    getopt_long additionally provides the very usefuly long options over standard getopt. It also adds two more bits of malarkey:

    * the new options are largely independent of the existing options
    * longindex, which weakly duplicates the existing 'flag' mechanism
    * automatically setting variables

    getopt_long_only additionally provides nothing but malarkey:
    
    * a long-only syntax which is just more malarkey.

    getoptions provides the worthwhile options without the baggage.

 */

OptionGetter::OptionGetter(const char *optionSpec): storageArea(optionSpec), optstring("+") {
  char *p = &storageArea[0];
  for (;;) {
    while (*p == ' ') {
      *p = 0;
      p++;
    }
    if (*p == '-') {
      *p = 0;
      p++;
      option newOption = { 0, 0, 0, 0 };
      newOption.val = *p;
      optstring.push_back(*p);
      p++;
      if (*p == '-') {
        p++;
        if (*p != '-')
          lineabort();
        p++;
        newOption.name = p;
        while (isalnum(*p))
          p++;
      }
      while (*p == ':') {
        newOption.has_arg++;
        p++;
        optstring.push_back(':');
      }
      optionVector.push_back(newOption);
    }
    if (!*p)
      break;
  }
  option nullOption = { 0, 0, 0, 0 };
  optionVector.push_back(nullOption);

  opterr = 0;
  ::optind = 1;
}

int OptionGetter::getopt(int argc, const char *argv[]) {
  return getopt(argc, (char **)argv);
}

int OptionGetter::getopt(int argc, char *argv[]) {
  //for (int i = 0; i < argc; i++)
    //logMsg << logValue(i) << ' ' << logValue(argv[i]) << std::endl;
  // supply longindex, and return all supplied options as normal even if they are erroneous.
  int result = getopt_long(argc, argv, optstring.c_str(), &optionVector[0], 0);
  optind = ::optind; // copy optind. Some libraries (e.g. MinGW32) balls up its linkage
  if (result == '?')
    result = optopt;
  return result;
}

ArgvMaker::ArgvMaker() {
}

void ArgvMaker::process(const char *args, int hasProgram) {
  s = args;
  if (!hasProgram)
    v.push_back("pseudoprogram"); // there has to be something
  const char *field = &s[0];
  for (int i = 0; i < s.size(); i++)
    if (s[i] == ' ') {
      s[i] = 0;
      v.push_back(field);
      field = &s[i + 1];
    }
  v.push_back(field);
  argc = v.size();
  argv = &v[0];
}

ArgvMaker::ArgvMaker(const char *args, int hasProgram) {
  process(args, hasProgram);
}

