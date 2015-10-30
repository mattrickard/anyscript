
// Scorecard.cpp
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

#include "Scorecard.h"
#include "logMsg.h"
#include <errno.h>
#include "UnitTest.h"
#include <fstream>
#include "Utils.h"
#include <algorithm>
#include <string.h>
#include <unistd.h>
#include "copyFile.h"
#include <getopt.h>
#include "MacroNames.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <signal.h>

Scorecard::Scorecard():
  sameOrder(false),
  pullPlug(0),
  timesFileBackedUp(false),
  continueAfterError(false),
  throwingException(0)
{
}

void Scorecard::recordResult(bool result) {
  if (result) {
    recordPass();
  }
  else
    recordFail();
}

void Scorecard::recordPass() {
  passes++;
}

void Scorecard::recordFail() {
  softRecordFail();
  haltOnFail();
}

void Scorecard::haltOnFail() {
  if (fails && !continueAfterError) {
    if (currentTest) {
      std::cerr << currentTest->name << " halted.\n";
      if (!throwingException) {
        throwingException = true;
        throw UnitTestException();
      }
    }
    else {
      std::cout << olib::argv[0] << " aborting" << std::endl;
      lineabort();
    }
  }
}

void Scorecard::softRecordFail() {
  if (currentTest) {
    if (currentTest->status != TEST_FAILED) {
      currentTest->status = TEST_FAILED;
      currentTest->timeSinceLastFail = 0;
      writeTimes();
      writeLog(*currentTest);
      std::cout << currentTest->name << " failed." << std::endl;
    }
  }
  else {
    std::cout << olib::argv[0] << " failed without running any tests." << std::endl;
  }
  fails++;
}

void Scorecard::writeTimes() {
  if (timesFileName.size()) {
    if (!timesFileBackedUp) {
      if (renameFile(timesFileName.c_str(), timesFileBackupName.c_str())) {
        if (errno != ENOENT) {
          std::string errorDesc(ErrnoToMacroName(errno));
          fprintf(stderr, "Scorecard::writeTimes(): cannot rename %s to %s: error %d %s %s\n",
            timesFileName.c_str(), timesFileBackupName.c_str(), errno, ErrnoToMacroName(errno), strerror(errno));
          lineabort();
        }
      }
      timesFileBackedUp = true;
    }
    std::ofstream os(timesFileName.c_str());
    os << "TestName Status LastRuntime(micros) TimeSinceLastFail(micros) FirstRunTime(seconds) CumulativeRuntime(micros) PassedRuns FailedRuns\n";
    os << "-------- ------ ------------------- ------------------------- --------------------- ------------------------- ---------- ----------\n";
    for (std::vector<TestNode>::iterator i = testVector.begin(); i != testVector.end(); i++) {
      os << *i;
    }
    os << std::flush;
    os.close();
  }
}

void Scorecard::writeLog(TestNode &tn) {
  if (logFileName.size()) {
    static std::ofstream os(logFileName.c_str(), std::ios_base::app); // append
    os << tn << std::flush;
  }
}

void Scorecard::readTimes() {
  time(&testRunTime);
  earliestRunTime = 0;
  if (timesFileName.size()) {
    std::ifstream is(timesFileName.c_str());
    std::string line;
    getline(is, line); // headings
    getline(is, line); // dashed lines
    TestNode tn;
    tn.timesFileName = timesFileName;
    while (is >> tn) {
      testMap[tn.name] = int(testVector.size());
      tn.unitTest = 0;
      if (tn.firstRunTime < earliestRunTime || !earliestRunTime)
        earliestRunTime = tn.firstRunTime;
      testVector.push_back(tn);
    }
    is.close();
  }
}

void terminateHandler() {
  fputs("terminateHandler() called\n", stderr);
  lineabort();
}

int Scorecard::examine(UnitTestSuite &uts, int argc, char *argv[]) {
  return examine(uts, argc, (const char **)argv);
}

int Scorecard::examine(UnitTestSuite &uts, int argc, const char *argv[]) {
  std::set_terminate(terminateHandler);
  return processArgs(argc, argv)? examine(uts)? 0: 1: 2;
}

bool Scorecard::processArgs(int argc, const char *argv[]) {
  olib::argc = argc;
  olib::argv = (char **)argv;
  std::string usage
    = std::string("Usage: [ADDRESS=0x<HexAddress>] ")
    + argv[0] + " [-h|--help] [-c|--continue] [-s|--sameorder] [TestName]\n";

  struct option longOptions[] = {
    { "continue",  no_argument, 0, 'c' },
    { "sameorder", no_argument, 0, 's' },
    { "help",      no_argument, 0, 'h' },
    { 0,           0,           0, 0   }
  };
  opterr = 0;
  int option;
  while ((option = getopt_long(argc, olib::argv, "csh", longOptions, 0)) != -1) {
    switch (option) {
    case 'c': continueAfterError = true;  break;
    case 's': sameOrder          = true;  break;
    case 'h': std::cerr << usage;         exit(0);
    default:
      if (optopt == 0)
        std::cerr << argv[0] << ": \"" << argv[optind - 1] << "\": invalid option\n" << usage;
      else
        std::cerr << argv[0] << ": \"" << static_cast<char>(optopt) << "\": invalid option\n" << usage;
      return false;
    }
  }
  while (optind < argc) {
    requestedTestNameVector.push_back(argv[optind]);
    requestedTestNameSet.insert(argv[optind]);
    optind++;
  }

  char *dirGuff  = strdup(olib::argv[0]);
  char *baseGuff = strdup(olib::argv[0]);
  std::string dirPart = DirName(dirGuff);
  const char *basePart = BaseName(baseGuff);
  if (dirPart.size()) {
    timesFileName = std::string(dirPart) + "/" + basePart + "-times";
    logFileName = std::string(dirPart) + "/" + basePart + "-log";
  }
  else {
    timesFileName = std::string("") + basePart + "-times";
    logFileName = std::string("") + basePart + "-log";
  }
  ReplaceAll(timesFileName, ".exe", "");
  ReplaceAll(logFileName, ".exe", "");
  free(dirGuff);
  free(baseGuff);
  timesFileBackupName = timesFileName + "-backup";
  return true;
}

/*

Run order:
Level
Running tests, by last run time
Failed tests, by last run time
Never-run tests, unsorted
Passed tests, by time since last fail
Not found tests, unsorted

*/

class SortTestsIntoRunOrder {
public:
  bool operator () (const TestNode &a, const TestNode &b) {
    // Return true if a precedes b
    int aLevel = a.unitTest? a.unitTest->getLevel(): 2000000000;
    int bLevel = b.unitTest? b.unitTest->getLevel(): 2000000000;
    if (aLevel != bLevel)
      return aLevel < bLevel;
    if (a.runTime != b.runTime)
      if (getenv("FASTEST_FIRST"))
        return a.runTime < b.runTime;
    if (a.status != b.status)
      return a.status < b.status;
    if (a.status == TEST_RUNNING || a.status == TEST_FAILED)
      if (a.runTime != b.runTime)
        return a.runTime < b.runTime;
    if (a.status == TEST_NEVER_RUN)
      return false;
    if (a.lastRunTime != b.lastRunTime)
      return a.lastRunTime < b.lastRunTime;
    return a.timeSinceLastFail < b.timeSinceLastFail;
  }
};

bool Scorecard::skip(const char *name) {
  if (requestedTestNameVector.size())
    return 0;
  static std::set<std::string> skipSet;
  static int loaded(0);
  if (!loaded) {
    loaded = 1;
    const char *envvar(getenv("SKIP"));
    if (envvar) {
      std::string str(envvar);
      for (;;) {
        if (!str.size())
          break;
        std::string s = GetField(str, ",");
        if (s.size()) {
          skipSet.insert(s);
        }
      }
    }
  }
  return skipSet.count(name);
}

void Scorecard::searchTestSuite(UnitTestSuite &uts) {
  if (skip(uts.getname().c_str()))
    return;
  UnitTest *utp = dynamic_cast<UnitTest *>(&uts);
  if (utp) {
    if (!requestedTestNameVector.size() || requestedTestNameSet.count(uts.modifiedName())) {
      testFound = true;
      if (testMap.count(uts.modifiedName()) == 0) {
        testMap[uts.modifiedName()] = int(testVector.size());
        TestNode tn;
        tn.name = uts.modifiedName();
        tn.status = TEST_NEVER_RUN;
        testVector.push_back(tn);
      }
      testVector[testMap[uts.modifiedName()]].unitTest = utp;
    }
  }
  else
    suiteSet.insert(uts.modifiedName());
  std::vector<UnitTestSuite *> tv;
  uts.scorecard = this;
  uts.subTests = &tv;
  uts.addSubTests();
  std::vector<UnitTestSuite *>::iterator i;
  for (i = tv.begin(); i < tv.end(); i++) {
    if (dynamic_cast<UnitTest *>(*i))
      if (testMap.count((*i)->modifiedName()) && testVector[testMap[(*i)->modifiedName()]].unitTest)
        delete *i;
      else {
        searchTestSuite(**i);
        if (requestedTestNameVector.size() && !requestedTestNameSet.count((*i)->modifiedName()))
          delete *i;
      }
    else {
      if (!suiteSet.count((*i)->modifiedName()))
        searchTestSuite(**i);
      delete *i;
    }
  }
}

static Scorecard *signaledScorecard = 0;

static sighandler_t previousHandler[_NSIG];

// "man 2 signal" says you basically shouldn't use signal, because there
// is no portable way of specifying repeated signal handling. The
// behaviour desired here is to do a bit of cleanup (write the test
// times), restore the old signal handler (clearing it) and re-raise the
// signal, so the calling process can see how and why this process
// exited. Hence safeSignal(), which has the simple API of signal() but
// specifically allows nested signalling.

sighandler_t safeSignal(int signum, sighandler_t handler) {
#ifdef __MINGW32__
  // unfortunately MinGW doesn't do sigaction
  return signal(signum, handler);
#else
  struct sigaction act, oldact;
  act.sa_handler = handler;
  act.sa_flags = SA_NODEFER; // this is the flag that allows nested signalling
  sigemptyset(&act.sa_mask);
  int result = sigaction(signum, &act, &oldact);
  if (result) {
    logMsg << logValue(result) << std::endl;
    lineabort();
  }
  return oldact.sa_handler;
#endif
}

void scorecardSignalHandler(int sig) {
  static bool triedThis(0);
  if (triedThis) {
    logMsg << olib::argv[0] << " caught signal " << SignalToMacroName(sig) << ". Must have totally mangled the signal handlers. _exit()ing this time" << std::endl;
    _exit(0);
  }
  triedThis = 1;
  safeSignal(sig, previousHandler[sig]);
  logMsg << olib::argv[0] << " caught signal " << SignalToMacroName(sig) << ". restored the old signal handler (" << previousHandler[sig] << ") and writing times" << std::endl;

  if (signaledScorecard)
    signaledScorecard->writeTimes();

  logMsg << olib::argv[0] << " wrote times and re-raising the signal. This should exit the process" << std::endl;
  raise(sig); // this shouldn't return
  logMsg << olib::argv[0] << " Hmm. raise() returned without exiting the process. Must have totally mangled the signal handlers. _exit()ing this time" << std::endl;
  _exit(0);
}

bool Scorecard::actuallyTest(std::vector<TestNode>::iterator i) {
  ut = i->unitTest;
  if (ut && (i->status != TEST_SKIP && !skip(i->name.c_str()))) {
    currentTest = &*i;
    i->status = TEST_RUNNING;
    writeTimes();
    writeLog(*i);
    displayCurrentTest(i->name);
    timer.getMicroseconds(TIMER_RESET);
    try {
      ut->Setup();
      ut->run();
      ut->TearDown();
    }
    catch (UnitTestException &e) {
      if (pullPlug) {
#if defined _WIN32 && !defined __CYGWIN__
        TerminateProcess(GetCurrentProcess(), 1);
#endif
#ifdef unix
        _exit(1);
#endif
      }
      for (; i != testVector.end(); i++)
        if (i->unitTest != unitTestSuite)
          delete i->unitTest;
      return false;
    }
    i->lastRunTime = testRunTime;
    if (!i->firstRunTime)
      i->firstRunTime = i->lastRunTime;
    i->runTime = timer.getMicroseconds();
    if (i->status == TEST_FAILED) {
      i->timeSinceLastFail = 0;
      i->failedRuns++;
    }
    else {
      i->status = TEST_PASSED;
      i->timeSinceLastFail += i->runTime;
      i->cumulativeRunTime += i->runTime;
      i->passedRuns++;
      writeLog(*i);
    }
  }
  if (pullPlug) {
    writeTimes();
#if defined _WIN32 && !defined __CYGWIN__
    TerminateProcess(GetCurrentProcess(), 0);
#elif defined unix
    _exit(0);
#endif
  }
  return true;
}

bool Scorecard::examine(UnitTestSuite &uts) {
  passes = 0;
  fails = 0;
  unitTestSuite = &uts;
  readTimes();
  currentTest = 0;
  testFound = false;
  searchTestSuite(uts);
  if (!sameOrder && !requestedTestNameVector.size())
    stable_sort(testVector.begin(), testVector.end(), SortTestsIntoRunOrder());
#ifdef unix
  previousHandler[SIGHUP] = safeSignal(SIGHUP, scorecardSignalHandler);
  previousHandler[SIGQUIT] = safeSignal(SIGQUIT, scorecardSignalHandler);
  previousHandler[SIGPIPE] = safeSignal(SIGPIPE, scorecardSignalHandler);
#endif
  previousHandler[SIGINT] = safeSignal(SIGINT, scorecardSignalHandler);
  previousHandler[SIGTERM] = safeSignal(SIGTERM, scorecardSignalHandler);
  signaledScorecard = this;
  if (requestedTestNameVector.size()) {
    for (std::vector<const char *>::iterator i = requestedTestNameVector.begin(); i != requestedTestNameVector.end(); i++) {
      if (testMap.count(*i)) {
        TestNode &tn(testVector[testMap[*i]]);
        if (tn.unitTest && (tn.status != TEST_SKIP && !skip(tn.name.c_str())))
          (testVector.begin() + testMap[*i])->unitTest->PreSetup();
      }
    }
    for (std::vector<const char *>::iterator i = requestedTestNameVector.begin(); i != requestedTestNameVector.end(); i++) {
      if (testMap.count(*i) && testVector[testMap[*i]].unitTest) {
        if (!actuallyTest(testVector.begin() + testMap[*i]))
          return false;
      }
      else {
        std::cerr << olib::argv[0] << ": " << *i << ": test not found." << std::endl;
        return false;
      }
    }
  }
  else {
    for (std::vector<TestNode>::iterator i = testVector.begin(); i != testVector.end(); i++)
      if (i->unitTest && (i->status != TEST_SKIP && !skip(i->name.c_str())))
        i->unitTest->PreSetup();
    for (std::vector<TestNode>::iterator i = testVector.begin(); i != testVector.end(); i++) {
      if (!actuallyTest(i))
        return false;
    }
  }
  for (std::vector<TestNode>::iterator i = testVector.begin(); i != testVector.end(); i++) {
    UnitTest *ut = i->unitTest;
    if (ut && (i->status != TEST_SKIP && !skip(i->name.c_str()))) {
      if (ut != unitTestSuite)
        delete ut;
    }
  }
  std::cout << NEW_LINE << passes << " passes, " << fails << " fails" << "\r\n" << std::flush;
  writeTimes();
  if (pullPlug) {
#if defined _WIN32 && !defined __CYGWIN__
      TerminateProcess(GetCurrentProcess(), 0);
#elif defined unix
      _exit(0);
#endif
  }
  signaledScorecard = 0;
  if (fails)
    return false;
  showMemoryUsage();
  return true;
}

void Scorecard::displayCurrentTest(std::string testName) {
  std::cout << testName << ' ' << std::flush;
}

UnitTestException::UnitTestException() {
}

Scorecard::~Scorecard() {
}

TestNode::TestNode(): runTime(0), timeSinceLastFail(0), firstRunTime(0), lastRunTime(0), cumulativeRunTime(0),
  passedRuns(0), failedRuns(0), status(TEST_NEVER_RUN) { }

std::ostream &operator << (std::ostream &os, const TestNode &tn) {
  os << tn.name << ' ';
  switch (tn.status) {
  case TEST_RUNNING:   os << "running";                      break;
  case TEST_FAILED:    os << "failed";                       break;
  case TEST_NEVER_RUN: os << "never_run";                    break;
  case TEST_PASSED:    os << "passed";                       break;
  case TEST_SKIP:      os << "skip";                         break;
  default:             os << "unknown(" << tn.status << ")"; break;
  }
  if (!tn.unitTest)
    os << "(not_found)";
  os << ' ' << tn.runTime
     << ' ' << tn.timeSinceLastFail
     << ' ' << tn.firstRunTime
     << ' ' << tn.lastRunTime
     << ' ' << tn.cumulativeRunTime
     << ' ' << tn.passedRuns
     << ' ' << tn.failedRuns
     << '\n';
  return os;
}

std::istream &operator >> (std::istream &is, TestNode &tn) {
  std::string line, status;
  if (getline(is, line)) {
    std::istringstream linestream(line);
    //logMsg << logValue(line) << std::endl;
    linestream >> tn.name
               >> status
         >> tn.runTime
         >> tn.timeSinceLastFail
         >> tn.firstRunTime
         >> tn.lastRunTime
         >> tn.cumulativeRunTime
         >> tn.passedRuns
         >> tn.failedRuns;
    if (status == "running") tn.status = TEST_RUNNING;
    else if (status == "running(not_found)") tn.status = TEST_RUNNING;
    else if (status == "failed") tn.status = TEST_FAILED;
    else if (status == "failed(not_found)") tn.status = TEST_FAILED;
    else if (status == "passed") tn.status = TEST_PASSED;
    else if (status == "skip") tn.status = TEST_SKIP;
    else if (status == "skip(not_found)") tn.status = TEST_SKIP;
    else if (status == "passed(not_found)") tn.status = TEST_PASSED;
    else if (status == "failed(not_found)") tn.status = TEST_FAILED;
    else if (status == "never_run" || status == "never_run(not_found)") tn.status = TEST_NEVER_RUN;
    else {
      std::cerr << tn.timesFileName << ": unknown status '" << status << "'" << std::endl;
      std::cerr << tn.timesFileName << " may be an old version or corrupt." << std::endl;
      lineabort();
    }
  }
  return is;
}

