
// AbstractProcess.h
// Copyright (C) 2003,2008,2015 Matthew Rickard

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

#ifndef ABSTRACT_PROCESS_H
#define ABSTRACT_PROCESS_H

#include <vector>
#include <map>
#include <iostream>

#ifdef unix
#include <sys/time.h>
#endif
class Process {
public:
  Process();
  Process(const char *cmd);
  void setOutput(const char *filename);
  int run();
  void printArgs(std::ostream &);
  bool start();
  int waitForExit();
  const char **getArgv();
  bool hasArgs();
  std::string getProgram();
  std::string getArgs();
  int inputPipe, outputPipe, errorPipe;
  int inputFd, outputFd, errorFd;
  pid_t child;
  bool running;
  ~Process();
  void addArgs(const char *arg);
  void addArg(const std::string &arg);
  std::vector<std::string> args;
  const char **argv;
  void setenv(const char *env, const char *val);
private:
  int maxRunTime;
  size_t maxStack;
  size_t maxData;
  int runTime;
#if defined _WIN32 && !defined __CYGWIN__
  UINT64 startTime, finishTime;
#else
  struct timeval startTime, finishTime;
#endif
  int saveerrfd;
  void safeclose(int fd);
  void construct();
  std::map<std::string, std::string> envmap;
};

#endif

