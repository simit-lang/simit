#include "Errors.h"

using namespace simit;
using namespace std;

/* Error */
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
  string columnStr = (firstColumn == lastColumn)
                     ? to_string(firstColumn)
                     : to_string(firstColumn) + "-" + to_string(lastColumn);
  return "Error: " + msg + ", at " + lineStr + ":" + columnStr;
}
