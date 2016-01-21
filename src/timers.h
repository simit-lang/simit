#ifndef SIMIT_TIMERS_H
#define SIMIT_TIMERS_H

#include "ir.h"

namespace simit {
  void printTimes();
namespace ir {
  Func insertTimers(Func func);
  
  // Singleton
  class TimerStorage {
  public:
    static TimerStorage& getInstance() {
      static TimerStorage instance; 
      return instance;
    }

    inline std::vector<std::string>& getSourceLines() {
      return sourceLines;
    }

    inline void addSourceLines(std::stringstream& ss) {
      for (std::string line; getline(ss, line); sourceLines.push_back(line));
    }

    inline void addTimedLine(std::string line) {
      timedLines.push_back(line);
    }

    inline int getTimedLineIndex(std::string line) {
      size_t pos = find(timedLines.begin(), timedLines.end(),
          line.c_str()) - timedLines.begin();
      if (pos >= timedLines.size()){
        pos = -1;
      }
      return pos;
    }

    inline void printTimedLines() {
      for (auto& line : timedLines) {
        std::cout << line << std::endl;
      }
    }

    inline void storeTime(size_t index, double time) {
      while (timerCount.size() < index + 1) {
        timerCount.push_back(0);
        timerSums.push_back(0);
      }
      timerCount[index] += 1;
      timerSums[index] += time;
    }

    inline double getTime(int index) {
      return timerSums[index];
    }

    inline unsigned long long int getCounter(int index) {
      return timerCount[index];
    }

    inline double getTotalTime() {
      double sum = 0;
      for(auto const &time : timerSums) {
        sum += time;
      }
      return sum;
    }

    inline double getTimingPercentage(int index) {
      return getTime(index) * 100.0 / getTotalTime(); 
    }
    
    private:
      std::vector<std::string> sourceLines;
      std::vector<std::string> timedLines;
      std::vector<double> timerSums;
      std::vector<unsigned long long int> timerCount;

      TimerStorage() {};
      TimerStorage(TimerStorage const&)    = delete;
      void operator=(TimerStorage const&)  = delete;
  };

}}

#endif
