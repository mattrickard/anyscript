
// GetOption.h
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

#ifndef GETOPTION_H
#define GETOPTION_H

#include <getopt.h>
#include <vector>
#include <string>

/*

    getopt provides the impressive arguments and optional arguments facilities. It also provides some malarkey:

    * different behaviour on errors
    * re-ordering arguments
    * error messages

    getopt_long additionally provides the very usefuly long options over standard getopt. It also adds more bits of malarkey:

    * the new options are largely independent of the existing options
    * longindex, which weakly duplicates the existing 'flag' mechanism
    * automatically setting variables

    getopt_long_only additionally provides nothing but malarkey.
    
    OptionGetter provides the worthwhile options without the baggage.

*/

class OptionGetter {
  std::vector<option> optionVector;
  std::string storageArea;
  std::string optstring;
public:
  OptionGetter(const char *optionSpec);
  int getopt(int argc, char *argv[]);
  int getopt(int argc, const char *argv[]);
  int optind;
};

class ArgvMaker {
  std::string s;
  std::vector<const char *> v;
public:
  int argc;
  const char **argv;
  ArgvMaker(const char *args, int hasProgram);
  ArgvMaker();
  void process(const char *args, int hasProgram);
};

#endif

