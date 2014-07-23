#ifndef SIMIT_ERRORS_H
#define SIMIT_ERRORS_H

#include <string>

namespace simit {

/** Provides information about errors that occur while loading Simit code. */
class Error {
 public:
  Error(int firstLine, int firstColumn, int lastLine, int lastColumn,
        std::string msg);
  virtual ~Error();

  int getFirstLine() { return firstLine; }
  int getFirstColumn() { return firstColumn; }
  int getLastLine() { return lastLine; }
  int getLastColumn() { return lastColumn; }
  const std::string &getMsg() { return msg; }

  std::string toString() const;
  friend std::ostream &operator<<(std::ostream &os, const Error &obj) {
    return os << obj.toString();
  }


 private:
  int firstLine;
  int firstColumn;
  int lastLine;
  int lastColumn;
  std::string msg;
  std::string line;  // TODO
};

}

#endif
