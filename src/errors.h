#ifndef SIMIT_ERRORS_H
#define SIMIT_ERRORS_H

#include <string>
#include <vector>
#include <iostream>

namespace simit {

/// Provides information about errors that occur while loading Simit code.
class Error {
 public:
  Error(int firstLine, int firstColumn, int lastLine, int lastColumn,
        std::string msg);
  virtual ~Error();

  int getFirstLine() { return firstLine; }
  int getFirstColumn() { return firstColumn; }
  int getLastLine() { return lastLine; }
  int getLastColumn() { return lastColumn; }
  const std::string &getMessage() { return msg; }

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

}

#endif
