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
      (*msg) << " in " << func << " in file " << file << ":" << line;
      break;
    case Internal:
      (*msg) << "Internal ";
      if (warning) {
        (*msg) << "warning";
      } else {
        (*msg) << "error";
      }
      (*msg) << " at " << file << ":" << line << " in " << func;
      if (conditionString) {
        (*msg)  << endl << " Condition failed: " << conditionString << endl;
      }
      break;
    case Temporary:
      (*msg) << "Temporary assumption broken";
      (*msg) << " at " << file << ":" << line;
      if (conditionString) {
        (*msg) << "\n" << " Condition failed: " << conditionString;
      }
      break;
  }
  (*msg) << " ";
}

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
  throw SimitException();
  exit(1);
}

}}
