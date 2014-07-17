#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

namespace util {

#ifdef LOGGING

class Logger {
 public:
  Logger(std::string logfilename) {
    logfile.open(logfilename);
  }

  ~Logger() {
    logfile.close();
  }

  void log(std::string msg) {
    std::string indent(indentLevel*2, ' ');

    if (msg == "") {
      logfile << std::endl;
    }
    else {
      std::stringstream ss(msg);
      std::string to;
      while(std::getline(ss,to,'\n')) {
        logfile << indent << to << std::endl;
      }
    }
  }

  void indent() {
    indentLevel++;
  }

  void dedent() {
    indentLevel--;
  }

 private:
  std::ofstream logfile;
  uint indentLevel;
};
#else
class Logger {
 public:
  Logger(std::string logfilename) {}
  ~Logger() {}
  void log(std::string msg) {}
  void indent() {}
  void dedent() {}
};
#endif


extern Logger logger;

inline void log() {
  logger.log("");
}

inline void log(std::string msg) {
  logger.log(msg);
}

inline void logIndent() {
  logger.indent();
}

inline void logDedent() {
  logger.dedent();
}

}
#endif

