#include <iostream>

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
  std::stringstream oss;
  oss << "Error: " << msg << ", at " << to_string(firstLine) << ":" 
      << to_string(firstColumn);
  if (firstLine != lastLine || firstColumn != lastColumn) {
    oss << "-";
    if (firstLine != lastLine) {
      oss << to_string(lastLine) << ":";
    }
    oss << lastColumn;
  }
  return oss.str();
}

// class Diagnostics
std::ostream &operator<<(std::ostream &os, const Diagnostics &f) {
  return os << f.getMessage();
}

namespace internal {

ErrorReport::ErrorReport(const char *file, const char *func, int line,
                         bool condition, const char *conditionString,
                         Kind kind, bool warning)
    : msg(NULL), file(file), func(func), line(line), condition(condition),
      conditionString(conditionString), kind(kind), warning(warning) {
  if (condition) {
    return;
  }
  msg = new std::ostringstream;

  switch (kind) {
    case User:
      if (warning) {
        (*msg) << "Warning";
      } else {
        (*msg) << "Error";
      }
      (*msg) << " in " << func << " in file " << file << ":" << line << endl;
      break;
    case Internal:
      (*msg) << "Compiler bug";
      if (warning) {
        (*msg) << "(warning)";
      }
      (*msg) << " at " << file << ":" << line << " in " << func;
      (*msg) << endl << "Please report it to http://issues.simit-lang.org";

      if (conditionString) {
        (*msg)  << endl << " Condition failed: " << conditionString;
      }
      (*msg) << endl;
      break;
    case Temporary:
      (*msg) << "Temporary assumption broken";
      (*msg) << " at " << file << ":" << line << endl;
      (*msg) << " Not supported yet, but planned for the future";
      if (conditionString) {
        (*msg) << endl << " Condition failed: " << conditionString;
      }
      (*msg) << endl;
      break;
  }
  (*msg) << " ";
}

// Force the classes to exist, even if exceptions are off
void ErrorReport::explode() {
  std::cerr << msg->str() << endl;
  delete msg;
//  assert(false);  // Uncomment to trigger an abort debuggers can break at

  if (warning) {
    return;
  }

  throw SimitException();
  exit(1);
}

}}
