
// anyscript.cpp
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

#include <iostream>
#include "GetOption.h"
#include "logMsg.h"
#include <sys/stat.h>
#include <stdlib.h>
#include "AbstractProcess.h"
#include "Utils.h"
#include <fstream>
#include "copyFile.h"
#include <string.h>
#include "logMsg.h"

void addToPath(const char *pathvar, const char *folder) {
  int len(strlen(folder));
  const char *oldpath = getenv(pathvar);
  if (oldpath) {
    const char *pos = strstr(pathvar, folder);
    if (pos && (pos[len] == ':' || pos[len] == 0))
      return; // It's already in the path
    setenv(pathvar, (std::string(oldpath) + ":" + folder).c_str(), 1);
  }
  else
    setenv(pathvar, folder, 1);
}

// Open the script file and read the shebang line.
// Returns true if the file is indeed a shebang file.
// Otherwise it leaves the file open and seeked back -- check that with stream.is_open()

bool openScriptFile(const char *name, std::ifstream &stream, std::string &firstline) {
  static int tries = 0;
  tries++;
  if (tries == 2)
    std::cout << "This is a second open. " << logValue(name) << std::endl;
  if (*name == '-')
    return false;
  if (getenv("ANYSCRIPT_SHOW_OPTIONS"))
    std::cout << "Trying to open " << name << std::endl;
  stream.open(name);
  if (!stream.good()) // TODO: maybe this should be is_open
    return false;
  if (getline(stream, firstline) && firstline.size() > 1 && firstline.substr(0, 2) == "#!")
    return true;
  stream.seekg(0);
  return false;
}

void processOptions(OptionGetter &optionGetter, int argc, const char *argv[]) {
  for (;;) {
    char option = optionGetter.getopt(argc, argv);
    if (option == -1) {
      //logMsg << "That's it for options" << std::endl;
      break;
    }
    else if (option == 'L') {
      if (getenv("ANYSCRIPT_SHOW_OPTIONS"))
        std::cout << "Library folder " << optarg << std::endl;
#if defined _WIN32 || defined __CYGWIN__
      addToPath("PATH", optarg);
#else
      addToPath("LD_LIBRARY_PATH", optarg);
#endif
    }
    else if (option == 'I') {
      if (getenv("ANYSCRIPT_SHOW_OPTIONS"))
        std::cout << "Include folder " << optarg << std::endl;
    }
    else if (option == 'l') {
      if (getenv("ANYSCRIPT_SHOW_OPTIONS"))
        std::cout << "Library " << optarg << std::endl;
      addToPath("LD_LIBRARY_PATH", ".");
    }
    else
      logMsg << "Unknown " << logValue(option) << std::endl;
  }
}

bool looksLikeShebangArgs(const char *s) {
  if (getenv("FAST_SHEBANG_CHECK"))
    return *s == '-';
  return *s == '-' && strchr(s, ' ');
}

int main(int argc, const char *argv[]) {
  OptionGetter optionGetter("-L: -l: -I:");

  // argv[1] is of limited length (typically 127 characters) so read it ourselves instead.
  // Further reading: Sven Mascheck http://www.in-ulm.de/~mascheck/various/shebang/
  // Oh also: it is either argv[1] or argv[2].

  if (getenv("ANYSCRIPT_SHOW_OPTIONS")) {
    std::cout << logValue(argc) << std::endl;
    for (int i = 0; i < argc; i++)
      std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
  }

  std::ifstream scriptfile;
  std::string shebangline;

  ArgvMaker argvMaker;

  //logMsg << logValue(argc) << std::endl;

  if (argc > 1 && argv[1][0] != '-' && openScriptFile(argv[1], scriptfile, shebangline)) {
    //logMsg << "checkpoint A\n";
    if (getenv("ANYSCRIPT_SHOW_OPTIONS"))
      std::cout << logValue(shebangline) << std::endl;
    argvMaker.process(shebangline.c_str(), 1);
    processOptions(optionGetter, argvMaker.argc, argvMaker.argv);
    optionGetter.optind = 1;
  }
  else if (argc > 2 && looksLikeShebangArgs(argv[1]) && argv[2][0] != '-' && openScriptFile(argv[2], scriptfile, shebangline)) {
    //logMsg << "checkpoint B\n";
    if (getenv("ANYSCRIPT_SHOW_OPTIONS"))
      std::cout << logValue(shebangline) << std::endl;
    argvMaker.process(shebangline.c_str(), 1);
    processOptions(optionGetter, argvMaker.argc, argvMaker.argv);
    optionGetter.optind = 2;
  }
  else {
    //logMsg << "checkpoint C\n";
    processOptions(optionGetter, argc, argv);
    if (scriptfile.is_open()) {
      if (getenv("ANYSCRIPT_SHOW_OPTIONS"))
        std::cout << "There's an open file here ... " << logValue(optionGetter.optind) << std::endl;
      if (argv[1][0] != '-' && optionGetter.optind == 1 || argv[1][0] == '-' && optionGetter.optind == 2) {
        if (getenv("ANYSCRIPT_SHOW_OPTIONS")) {
          std::cout << "That's all right then" << std::endl;
          goto thatsalrightthen;
        }
      }
      // how do you get to here? You need to modify looksLikeShebangArgs so it does not test for space. Then, AnyscriptTest3f can hit it.
      if (getenv("ANYSCRIPT_SHOW_OPTIONS"))
        std::cout << "Closing it." << std::endl;
      scriptfile.close();
    }
    openScriptFile(argv[optionGetter.optind], scriptfile, shebangline);
    if (!scriptfile.is_open()) {
      std::cerr << "anyscript: Couldn't open argv[1] " << argv[1] << " or argv[2] " << argv[2] << std::endl;
      lineabort();
    }
    argvMaker.argc = optionGetter.optind;
    argvMaker.argv = argv;
  }

  thatsalrightthen:

  //if (getenv("ANYSCRIPT_SHOW_OPTIONS")) {
    //std::cout << logValue(argc) << std::endl;
    //for (int i = 0; i < argc; i++)
      //std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
  //}

  std::string script = argv[optionGetter.optind];
  std::string exe = script + "-x";

  struct stat scriptstat, exestat;

  if (stat(script.c_str(), &scriptstat))
    lineabort();

  bool exeexists = !stat(exe.c_str(), &exestat);

  if (getenv("RECOMPILE") || scriptstat.st_mtim.tv_sec > exestat.st_mtim.tv_sec || !exeexists) {
    std::cerr << "(compiling ...) " << std::flush;
    std::string cpp = script + "-x.cpp";
    std::ofstream cppfile(cpp.c_str());
    cppfile << "#line 2 \"" << script << "\"\n";
    copyFile(scriptfile, cppfile);
    cppfile.close();
    Process compiler;
    compiler.addArgs("/usr/bin/g++ -L. -o");
    compiler.addArg(exe);
    compiler.addArg(cpp);
    //logMsg << logValue(argvMaker.argc) << std::endl;
    for (int i = 1; i < argvMaker.argc; i++) {
      //logMsg << "Adding compiler argument " << argvMaker.argv[i] << std::endl;
      compiler.addArg(argvMaker.argv[i]);
    }
    int result = compiler.run();
    if (result)
      return result;
    std::cerr << std::endl;
  }

  scriptfile.close();

  Process exeprocess;
  if (getenv("DEBUG"))
    exeprocess.addArgs("gdb --args");
  if (exe.find('/') == std::string::npos)
    exe.insert(0, "./");
  exeprocess.addArg(exe);
  for (int i = optionGetter.optind + 1; i < argc; i++)
    exeprocess.addArg(argv[i]);

  return exeprocess.run();
}

