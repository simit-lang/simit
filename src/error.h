#ifndef SIMIT_DIAGNOSTICS_H
#define SIMIT_DIAGNOSTICS_H

#include <string>
#include <vector>
#include <iostream>

#include "util.h"

namespace simit {

/// Provides information about errors that occur while loading Simit code.
class ParseError {
public:
  ParseError(int firstLine, int firstColumn, int lastLine, int lastColumn,
        std::string msg);
  virtual ~ParseError();

  int getFirstLine() { return firstLine; }
  int getFirstColumn() { return firstColumn; }
  int getLastLine() { return lastLine; }
  int getLastColumn() { return lastColumn; }
  const std::string &getMessage() { return msg; }

  std::string toString() const;
  friend std::ostream &operator<<(std::ostream &os, const ParseError &obj) {
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

class Diagnostic {
public:
  Diagnostic() {}

  Diagnostic &operator<<(const std::string &str) {
    msg += str;
    return *this;
  }

  std::string getMessage() const { return msg; }

private:
  std::string msg;
};

class Diagnostics {
public:
  Diagnostics() {}
  ~Diagnostics() {}

  Diagnostic &report() {
    diags.push_back(Diagnostic());
    return diags[diags.size()-1];
  }

  bool hasErrors() {
    return diags.size() > 0;
  }

  std::string getMessage() const {
    std::string result;
    auto it = diags.begin();
    if (it != diags.end()) {
      result += it->getMessage();
      ++it;
    }
    while (it != diags.end()) {
      result += "\n" + it->getMessage();
      ++it;
    }
    return result;
  }

  std::vector<Diagnostic>::const_iterator begin() const { return diags.begin();}
  std::vector<Diagnostic>::const_iterator end() const { return diags.end(); }

private:
  std::vector<Diagnostic> diags;
};

std::ostream &operator<<(std::ostream &os, const Diagnostics &f);


namespace internal {

struct ErrorReport {
  std::ostringstream *msg;
  const char *file;
  const char *func;
  int line;

  bool condition;
  const char *conditionString;

  bool user;
  bool warning;

  ErrorReport(const char *file, const char *func, int line, bool condition,
              const char *conditionString, bool user, bool warning)
      : msg(NULL), file(file), func(func), line(line), condition(condition),
        conditionString(conditionString), user(user), warning(warning) {
    if (condition) {
      return;
    }
    msg = new std::ostringstream;

    if (user) {
      if (warning) {
        (*msg) << "Warning";
      } else {
        (*msg) << "Error";
      }
      (*msg) << " in " << func << " in file " << file << ":" << line << "\n";
    } else {
      (*msg) << "Internal ";
      if (warning) {
        (*msg) << "warning";
      } else {
        (*msg) << "error";
      }
      (*msg) << " at " << file << ":" << line << "\n";
      (*msg) << " Report this error to the developers\n";
      if (conditionString) {
        (*msg) << " Condition failed: " << conditionString << "\n";
      }
    }
  }

  template<typename T>
  ErrorReport &operator<<(T x) {
    if (condition) {
      return *this;
    }
    (*msg) << " " << x;
    return *this;
  }

  ~ErrorReport() noexcept(false) {
    if (condition) {
      return;
    }
    explode();
  }

  void explode();
};

#ifndef WITHOUT_INTERNAL_ASSERTS
#define iassert(c) simit::internal::ErrorReport(__FILE__, nullptr, __LINE__, c,     #c,      false, false)
#define ierror     simit::internal::ErrorReport(__FILE__, nullptr, __LINE__, false, nullptr, false, false)

#define unreachable ierror << "reached unreachable location"
#define not_supported_yet ierror << "Not supported yet, but planned for the future"
#else
#define iassert(c)
#define ierror

#define unreachable
#define not_supported_yet
#endif

#define uassert(c) simit::internal::ErrorReport(__FILE__, __FUNCTION__, __LINE__, c,     #c,      true, false)
#define uerror     simit::internal::ErrorReport(__FILE__, __FUNCTION__, __LINE__, false, nullptr, true, false)
#define uwarning   simit::internal::ErrorReport(__FILE__, __FUNCTION__, __LINE__, false, nullptr, true, true)
}

}

#endif
