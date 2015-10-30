
// Scorecard.h
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

#ifndef SCORECARD_H
#define SCORECARD_H

#include <vector>
#include <map>
#include <set>
#include "Utils.h"
#include <string>
#include <iostream>

class UnitTestSuite;
class UnitTest;

enum TestStatus { TEST_SKIP = -4, TEST_RUNNING = -3, TEST_FAILED = -2, TEST_NEVER_RUN = -1, TEST_PASSED = 0 };

class TestNode {
public:
  TestNode();
  std::string name, timesFileName;
  UnitTest *unitTest;
  long long runTime, timeSinceLastFail;
  time_t firstRunTime, lastRunTime;
  long long cumulativeRunTime;
  int passedRuns, failedRuns;
  TestStatus status;
};

std::ostream &operator << (std::ostream &os, const TestNode &tn);
std::istream &operator >> (std::istream &os, TestNode &tn);

class Scorecard {
public:
  Scorecard();
  virtual ~Scorecard();
  int examine(UnitTestSuite &uts, int argc, const char *argv[]);
  int examine(UnitTestSuite &uts, int argc, char *argv[]);
  bool examine(UnitTestSuite &uts);
  int passes;
  int fails;
  TestNode *currentTest;
  bool processArgs(int argc, const char *argv[]);
  bool sameOrder;
  bool pullPlug;
  Timer timer;

protected:
  virtual void recordPass();
  virtual void recordFail();
  virtual void displayCurrentTest(std::string testName);
  void haltOnFail();
  void softRecordFail();

private:
  void recordResult(bool result);
  void readTimes();
  void searchTestSuite(UnitTestSuite &uts);
  void writeTimes();
  void writeLog(TestNode &tn);

  std::vector<TestNode> testVector;
  std::map<std::string, int> testMap;
  std::set<std::string> suiteSet;
  std::string timesFileName, timesFileBackupName, logFileName;
  bool timesFileBackedUp;
  bool continueAfterError;
  std::set<std::string> requestedTestNameSet;
  std::vector<const char *> requestedTestNameVector;
  bool skip(const char *name);
  bool testFound;
  UnitTestSuite *unitTestSuite;

  friend struct ScorecardMemoryDeclarer;
  friend struct UnitTest;
  friend void scorecardSignalHandler(int sig);
  bool throwingException;
  bool actuallyTest(std::vector<TestNode>::iterator i);
  time_t earliestRunTime;
  time_t testRunTime;
};

class UnitTestException {
public:
  UnitTestException();
};

#endif

