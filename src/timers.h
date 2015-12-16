#ifndef SIMIT_TIMERS_H
#define SIMIT_TIMERS_H

#include "ir.h"

namespace simit {
namespace ir {

  extern "C" void storeTime(int index, double time);
  
  const char* getTimerMapLine(int i);
  int getTimedLineIndex(std::string line);
  void printTimedLines();
  double getTime(int index);
  double getTimingPercentage(int index);
  unsigned long long int getCounter(int index);
  double getTotalTime();
  void addSourceLines(std::stringstream& ss);
  std::vector<std::string>& getSourceLines();

  Func insertTimers(Func func);
}}

#endif
