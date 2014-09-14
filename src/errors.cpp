#include "errors.h"

using namespace std;

namespace simit {

// class Error
Error::Error(int firstLine, int firstColumn, int lastLine, int lastColumn,
             std::string msg)
    : firstLine(firstLine), firstColumn(firstColumn),
      lastLine(lastLine), lastColumn(lastColumn),
      msg(msg) {}

Error::~Error() {

}

std::string Error::toString() const {
  string lineStr   = (firstLine == lastLine)
                     ? to_string(firstLine)
                     : to_string(firstLine) + "-" + to_string(lastLine);
  string columnStr = (firstColumn == lastColumn-1)
                     ? to_string(firstColumn)
                     : to_string(firstColumn) + "-" + to_string(lastColumn-1);
  return "Error: " + msg + ", at " + lineStr + ":" + columnStr;
}

// class Diagnostics
std::ostream &operator<<(std::ostream &os, const Diagnostics &f) {
  return os << f.getMessage();
}

} // namespace simit
