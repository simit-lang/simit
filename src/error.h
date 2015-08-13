#ifndef SIMIT_DIAGNOSTICS_H
#define SIMIT_DIAGNOSTICS_H

#include <string>
#include <vector>
#include <iostream>

#include "util/util.h"

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
  enum Kind { User, Internal, Temporary };

  std::ostringstream *msg;
  const char *file;
  const char *func;
  int line;

  bool condition;
  const char *conditionString;

  Kind kind;
  bool warning;

  ErrorReport(const char *file, const char *func, int line, bool condition,
              const char *conditionString, Kind kind, bool warning)
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
        (*msg) << " in " << func << " in file " << file << ":" << line << "\n";
        break;
      case Internal:
        (*msg) << "Internal ";
        if (warning) {
          (*msg) << "warning";
        } else {
          (*msg) << "error";
        }
        (*msg) << " at " << file << ":" << line << " in " << func << "\n";
        if (conditionString) {
          (*msg) << " Condition failed: " << conditionString << "\n";
        }
        break;
      case Temporary:
        (*msg) << "Temporary assumption broken";
        (*msg) << " at " << file << ":" << line << "\n";
        if (conditionString) {
          (*msg) << " Condition failed: " << conditionString << "\n";
        }
        break;
    }
  }

  template<typename T>
  ErrorReport &operator<<(T x) {
    if (condition) {
      return *this;
    }
    (*msg) << x;
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

// internal asserts
#ifdef SIMIT_ASSERTS
  #define iassert(c)                                                           \
    simit::internal::ErrorReport(__FILE__, __FUNCTION__, __LINE__, (c), #c,    \
                               simit::internal::ErrorReport::Internal, false)
  #define ierror                                                               \
    simit::internal::ErrorReport(__FILE__, __FUNCTION__, __LINE__, false, NULL,\
                               simit::internal::ErrorReport::Internal, false)
#else
  struct Dummy {
    template<typename T>
    Dummy &operator<<(T x) {
      return *this;
    }
  };

  #define iassert(c) simit::internal::Dummy()
  #define ierror simit::internal::Dummy()
#endif

#define tassert(c)                                                             \
  simit::internal::ErrorReport(__FILE__, __FUNCTION__, __LINE__, (c), #c,      \
                               simit::internal::ErrorReport::Temporary, false)
#define terror                                                                 \
  simit::internal::ErrorReport(__FILE__, __FUNCTION__, __LINE__, false, NULL,  \
                               simit::internal::ErrorReport::Temporary, false)

#define unreachable                                                            \
  ierror << "reached unreachable location"

#define not_supported_yet                                                      \
  ierror << "Not supported yet, but planned for the future"

// internal assert helpers
#define iassert_scalar(a)                                                      \
  iassert(isScalar(a.type())) << a << ": " << a.type()

#define iassert_types_equal(a,b)                                               \
  iassert(a.type() == b.type()) << a.type() << "!=" << b.type() << "\n"        \
                                << #a << ":" << a << "\n" << #b << ":" << b

#define iassert_boolean_scalar(a)                                              \
  iassert(isScalar(a.type()) && a.type().toTensor()->componentType.isBoolean())\
      << a << "must be a boolean scalar but is a" << a.type()

// User asserts
#define uassert(c)                                                             \
  simit::internal::ErrorReport(__FILE__,__FUNCTION__,__LINE__, (c), #c,        \
                               simit::internal::ErrorReport::User, false)
#define uerror                                                                 \
  simit::internal::ErrorReport(__FILE__,__FUNCTION__,__LINE__, false, nullptr, \
                               simit::internal::ErrorReport::User, false)
#define uwarning                                                               \
  simit::internal::ErrorReport(__FILE__,__FUNCTION__,__LINE__, false, nullptr, \
                               simit::internal::ErrorReport::User, true)
}}

#endif
