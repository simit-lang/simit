#ifndef SIMIT_FRONTEND_H
#define SIMIT_FRONTEND_H

#include <list>
#include <map>
#include <string>
#include <memory>
//#include <Program.h>

struct YYLTYPE;

namespace simit {

class IRNode;
class Test;
class Error;

/** Tracks symbols and their associated IR nodes across a stack of scopes. */
class SymbolTable {
 public:
  typedef std::map<std::string, std::shared_ptr<IRNode>> SymbolMap;
  typedef std::list<SymbolMap>::const_iterator ScopeIterator;

  SymbolTable()  {   scope(); }
  ~SymbolTable() { unscope(); }

  /** Add a new level of symbol scoping. */
  void scope()   { scopes.push_front(SymbolMap()); }

  /** Remove the top symbol scope. */
  void unscope() { scopes.pop_front(); }

  /** Adds a symbol with the name of the irNode pointing at the irNode. */
  void addNode(const std::shared_ptr<IRNode> &irNode);

  /** Return the first symbol first match or add the symbol to the top scope. */
  std::shared_ptr<IRNode> &operator[](const std::string &name);

  std::string toString() const;
  friend std::ostream &operator<<(std::ostream &os, const SymbolTable &table) {
    return os << table.toString();
  }

  /** Iterator over symbol scopes. */
  ScopeIterator begin() const { return scopes.begin(); }

  /** Iterator over symbol scopes. */
  ScopeIterator end() const { return scopes.end(); }

 private:
  std::list<SymbolMap> scopes;
};


/** Provides methods to convert Simit-formated strings and files to Simit IR.
  *
  * Strings and files can be parsed using the \ref parseString and
  * \ref parseFile methods and the resulting IR can be retrieved using the
  * \ref getIR method. If the parse methods return an error value information
  * about the errors can be retrieved using the getErrors method.  */
class Frontend {
 public:
  /** Parses, typechecks and turns a given Simit-formated string into Simit IR.
    * The resulting IR can be retrieved through the \ref getIR method and
    * errors through the \ref getErrors method. */
  int parseString(std::string programString);

  /** Parses, typechecks and turns a given Simit-formated file into Simit IR.
    * The resulting IR can be retrieved through the \ref getIR method and
    * errors through the \ref getErrors method. */
  int parseFile(std::string filename);

  std::list<simit::Error> &getErrors() { return errors; }
  std::list<simit::Test> &getTests() { return tests; }

 private:
  SymbolTable symbolTable;
  std::list<Error> errors;
  std::list<simit::Test> tests;
};

}

#endif

