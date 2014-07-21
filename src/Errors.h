#ifndef SIMIT_ERRORS_H
#define SIMIT_ERRORS_H

#include <string>

namespace simit {

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

  operator std::string() const;
  friend std::ostream &operator<<(std::ostream &os, const Error &obj) {
    return os << std::string(obj);
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
