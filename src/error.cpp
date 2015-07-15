#include "error.h"

using namespace std;

namespace simit {

// class Error
ParseError::ParseError(int firstLine, int firstColumn, int lastLine, int lastColumn,
             std::string msg)
    : firstLine(firstLine), firstColumn(firstColumn),
      lastLine(lastLine), lastColumn(lastColumn),
      msg(msg) {}

ParseError::~ParseError() {

}

std::string ParseError::toString() const {
  return "Error: " + msg + " at " + to_string(firstLine) + ":" +
         to_string(firstColumn);
}

// class Diagnostics
std::ostream &operator<<(std::ostream &os, const Diagnostics &f) {
  return os << f.getMessage();
}

namespace internal {

// Force the classes to exist, even if exceptions are off
void ErrorReport::explode() {
  // TODO: Add an option to error out on warnings too
  if (warning) {
    std::cerr << msg->str();
    delete msg;
    return;
  }

  std::cerr << msg->str() << "\n";
  delete msg;
  throw;
  exit(1);
}
} // namespace internal
} // namespace simit
