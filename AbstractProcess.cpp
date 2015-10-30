
// AbstractProcess.cpp
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

#include "AbstractProcess.h"
#ifdef unix
#include <unistd.h>
#endif
#include <string.h>
#ifdef unix
#include <sys/resource.h> // required for debian 8 jessie
#include <sys/wait.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h> // required in debian 8 jessie?
#include "Utils.h"
#include <stdlib.h>
#include "logMsg.h"
#include "MacroNames.h"
#include <stdio.h>

void Process::addArgs(const char *cmd) {
  //logMsg << "addArgs() " << ' ' << logValue(cmd) << std::endl;
#ifdef unix
  for (const char *p = cmd; *p; p++) {
    if (args.size() == 0 || isspace(*p)) {
      while (isspace(*p))
        p++;
      if (!*p)
        return;
      args.push_back("");
    }
    while (*p == '"' || *p == '\'') {
      char delimiter = *p;
      p++;
      while (*p && *p != delimiter) {
        *(args.end() - 1) += *p;
        p++;
      }
      if (*p)
        p++;
    }
    if (!*p)
      break;
    if (isspace(*p)) {
      while (isspace(*p))
        p++;
      if (!*p)
        return;
      args.push_back("");
    }
    *(args.end() - 1) += *p;
  }
#else
  addArg(cmd);
#endif
}

Process::Process() {
  construct();
}

void Process::construct() {
  inputPipe = -1;
  outputPipe = -1;
  errorPipe = -1;
  inputFd = 0;
  outputFd = 1;
  errorFd = 2;
  running = false;
  argv = 0;
  maxRunTime = 0;
  maxStack = 0;
  maxData = 0;
}

void Process::addArg(const std::string &arg) {
  //logMsg << "addArg() " << ' ' << logValue(arg) << std::endl;
  args.push_back(arg);
}

Process::Process(const char *cmd) {
  construct();
  addArgs(cmd);
}

void Process::setOutput(const char *filename) {
  outputFd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
  if (outputFd == -1) {
    int saveerrno(errno);
    logMsg << logValue(saveerrno) << ' ' << logValue(ErrnoToMacroName(saveerrno)) << std::endl;
    exit(1);
  }
}

int Process::run() {
  if (!start()) {
    return -1;
  }
  return waitForExit();
}

static bool executable1(const char *filename, std::string &filename2) {
#ifdef unix
  struct stat status;
  bool result;
  if (stat(filename, &status))
    result = false;
  else if (geteuid() == 1)
    result = status.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH);
  else if (status.st_uid == geteuid())
    result = status.st_mode & S_IXUSR;
  else if (status.st_gid == getegid())
    result = status.st_mode & S_IXGRP;
  else
    result = status.st_mode & S_IXOTH;
  //logMsg << logValue(filename) << ": " << logValue(result) << std::endl;
  return result;
#else
  filename2 = filename;
  std::string endbit(toUpper(filename2.substr(filename2.size() - 4)));
  if (endbit != ".EXE") {
    filename2 += ".EXE";
    filename = filename2.c_str();
  }
  //logMsg << logValue(filename) << ' ' << logValue(endbit) << std::endl;
  DWORD dontcare;
  bool result = GetBinaryType(filename, &dontcare);
  int error(GetLastError());
  if (error == ERROR_FILE_NOT_FOUND && !result) {
    //logMsg << logValue(filename) << ' ' << logValue(result) << ' ' << logValue(error) << ' ' << ErrorCodeToMacroName(error) << std::endl;
    return 0;
  }
  if (error == ERROR_PATH_NOT_FOUND && !result) {
    logMsg << logValue(filename) << ' ' << logValue(result) << ' ' << logValue(error) << ' ' << ErrorCodeToMacroName(error) << std::endl;
    return 0;
  }
  if (/*error == ERROR_ALREADY_EXISTS &&*/ result) {
    //logMsg << logValue(filename) << ' ' << logValue(result) /*<< ' ' << logValue(error) << ' ' << ErrorCodeToMacroName(error)*/ << std::endl;
    return 1;
  }
  logMsg << logValue(filename) << ' ' << logValue(result) << ' ' << logValue(error) << ' ' << ErrorCodeToMacroName(error) << std::endl;
  lineabort();
#endif
}

static bool executable2(std::string subPath, const char *filename, std::string &fullname) {
  if (subPath.length() == 0)
    return executable1(filename, fullname);
  subPath.append("/");
  subPath.append(filename);
  return executable1(subPath.c_str(), fullname);
}

#ifdef unix
#define PATH_SEPARATOR ':'
#elif defined _WIN32
#define PATH_SEPARATOR ';'
#endif

static bool searchPath(const char *filename, const char *path, std::string &fullname) {
  if (strchr(filename, '/'))
    return executable1(filename, fullname);
  const char *p1 = path;
  const char *p2 = strchr(path, PATH_SEPARATOR);
  while (p2) {
    std::string subPath(path, p1 - path, p2 - p1);
    if (executable2(subPath, filename, fullname))
      return true;
    p1 = p2 + 1;
    p2 = strchr(p1, PATH_SEPARATOR);
  }
  return false;
}

bool Process::start() {
  if (getenv("SHOWPROCESS")) {
    logMsg << "Process::start() running ";
    printArgs(std::cerr);
    std::cerr << std::endl;
  }
  std::string fullname;
  std::string program(getProgram());
  //logMsg << logValue(program) << std::endl;
  if (hasArgs()) {
    if (!searchPath(program.c_str(), getenv("PATH"), fullname)) {
      // Use stdio instead of iostream because backTrace can call this
      // during pre-main() initialization and if plextestbacktrace isn't
      // available, well, succinct diagnostic beats gratuitous coredump.
      fprintf(stderr, "Process::start(): couldn't find %s in PATH\n", program.c_str());
      return false;
    }
  }
#if defined unix
  if (!(child = fork())) {
    saveerrfd = 2;
    if (inputFd    !=  0) dup2(inputFd,  0);
    if (outputFd   !=  1) dup2(outputFd, 1);
    if (errorFd    !=  2) { saveerrfd = dup(2); dup2(errorFd, 2); fcntl(saveerrfd, F_SETFD, FD_CLOEXEC); } // saveerrfd is a copy used for outputting errors if execvp() fails
    if (inputFd  > 2) close(inputFd);
    if (outputFd > 2) close(outputFd);
    if (errorFd  > 2) close(errorFd);
    if (inputPipe  != -1) close(inputPipe);
    if (outputPipe != -1) close(outputPipe);
    if (errorPipe  != -1) close(errorPipe);
    if (maxRunTime > 0)   alarm(maxRunTime);
    struct rlimit rl;
    if (maxStack > 0) {
      rl.rlim_cur = rl.rlim_max = maxStack;
      setrlimit(RLIMIT_STACK, &rl);
    }
    if (maxData > 0) {
      rl.rlim_cur = rl.rlim_max = maxData;
      setrlimit(RLIMIT_DATA, &rl);
    }

    for (std::map<std::string, std::string>::iterator i = envmap.begin(); i != envmap.end(); i++) {
      //logMsg << "setting " << i->first << " to " << i->second << std::endl;
      ::setenv(i->first.c_str(), i->second.c_str(), 1);
    }

    execvp(program.c_str(), const_cast<char *const *>(getArgv()));
    FILE *saveerr = fdopen(saveerrfd, "w");
    fprintf(saveerr, "Could not exec \'%s\'\n", program.c_str());
    _exit(1);
  }
  else { 
    if (inputPipe  != -1) close(inputFd);
    if (outputPipe != -1) close(outputFd);
    if (errorPipe  != -1) close(errorFd);
  }
  gettimeofday(&startTime, 0);
#else
  //logMsg << logValue(getArgs()) << std::endl;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof si);
  si.cb = sizeof si;
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdInput = (HANDLE)_get_osfhandle(inputFd);
  si.hStdOutput = (HANDLE)_get_osfhandle(outputFd);
  si.hStdError = (HANDLE)_get_osfhandle(errorFd);
  GetSystemTimeAsFileTime((FILETIME *)&startTime);
  BOOL result = CreateProcess(
    fullname.c_str(),

    (char *)getArgs().c_str(),
    0, 0, // process security attributes, thread security attributes
    1, // inherit handles
    0, // priority class (0 = lowest of parent's class and normal)
    0, // environment (0 = same as calling process)
    0, // working directory (0 = same as calling process)
    &si,
    &pi);

  child = (pid_t)pi.hProcess;
  CloseHandle(pi.hThread);
  if (inputPipe  != -1) {
    close(inputFd);
  }
  if (outputPipe != -1) {
    close(outputFd);
  }
  if (errorPipe  != -1) {
    close(errorFd);
  }

  if (!result) {
    int error(GetLastError());
    logMsg << logValue(result) << ' ' << logValue(error) << ' ' << ErrorCodeToMacroName(error) << std::endl;
    logMsg << "Process::start() aborting because of error" << std::endl;
    lineabort();
  }

  //logMsg << "Process::start() pretty much done" << std::endl;
#endif
  running = true;
  return true;
}

void Process::safeclose(int fd) {
  //logMsg << "BTW " << logValue(EBADF) << ' ' << logValue(ErrnoToMacroName(EBADF)) << ". Do write ErrnoToMacroNameTest sometime." << std::endl;
  if (fd != -1) {
    int result = close(fd);
    if (result) {
      int someerrno(errno);
      logMsg << "close() " << logValue(result) << ' ' << ". Couldn't close file descriptor " << fd << ". " << logValue(someerrno) << ' ' << ErrnoToMacroName(someerrno) << ".\n";
      logMsg << "Args were " << getArgs() << '\n';
      logMsg << "Usually this is because an stdio_filebuf was attached to it and closed the file automatically. Drop the fd from the process\n";
      logMsg << "Aborting.\n";
      lineabort();
    }
  }
}

Process::~Process() {
  // Use a version of close that has loud, easy-to-trace error messages if something goes wrong. Microsoft CRT in particular produces quiet, hard-to-trace, wild-goose-chase error messages.
  safeclose(inputPipe);
  safeclose(outputPipe);
  safeclose(errorPipe);
}

void Process::printArgs(std::ostream &os) {
  const char **argv(getArgv());
  for (const char **arg = argv; *arg; arg++) {
    if (arg != argv)
      os << ' ';
    os << *arg;
  }
}

const char **Process::getArgv() {
  if (!argv) {
    argv = new const char *[args.size() + 1];
    int argc = 0;
    for (std::vector<std::string>::const_iterator i = args.begin(); i < args.end(); i++) {
      argv[argc++] = i->c_str();
    }
    argv[argc] = 0;
  }
  return argv;
}

bool Process::hasArgs() {
  return args.size();
}

std::string Process::getProgram() {
  std::string result = getArgv()[0];
  size_t space = result.find(' ');
  if (space != std::string::npos) {
    result.erase(space);
  }
  return result;
}

int Process::waitForExit() {
#if defined unix
  int status;
  pid_t result = waitpid(child, &status, 0);
  gettimeofday(&finishTime, 0);
  running = false;
  runTime = (finishTime.tv_sec - startTime.tv_sec);
  if (finishTime.tv_usec > startTime.tv_usec)
    runTime++;
  if (result == -1)
    return -1;
  return status;
#else
  DWORD result = WaitForSingleObject((HANDLE)child, INFINITE);
  GetSystemTimeAsFileTime((FILETIME *)&finishTime);
  running = false;
  runTime = (finishTime - startTime) / 10000000;
  if (result != WAIT_OBJECT_0) {
    logMsg << "That's not what I was expecting. " << logValue(result) << std::endl;
  }
  unsigned long status;
  BOOL result2 = GetExitCodeProcess((HANDLE)child, &status);
  if (!result2) {
    logMsg << "That's not what I was expecting. " << logValue(result2) << std::endl;
  }
  CloseHandle((HANDLE)child);
  return status;
#endif
}

std::string Process::getArgs() {
  std::string result;
  const char **argv = getArgv();
  while (*argv) {
    if (result.size())
      result.push_back(' ');
    result.append(*argv);
    argv++;
  }
  return result;
}

void Process::setenv(const char *env, const char *val) {
  envmap[env] = val;
}

