#ifndef SIMIT_FRONTEND_H
#define SIMIT_FRONTEND_H

#include <list>
#include <map>
#include <string>
#include <fstream>
#include <memory>

namespace simit {
class Error;

namespace internal {
class Function;
class IRNode;
class Test;

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

  /** Collapse all scopes into the top scope. */
  void collapse();

  /** Adds a symbol with the name of the irNode pointing at the irNode. */
  void addNode(const std::shared_ptr<IRNode> &irNode);

  /** Return the first symbol first match or add the symbol to the top scope. */
  std::shared_ptr<IRNode> &operator[](const std::string &name);

  friend std::ostream &operator<<(std::ostream &os, const SymbolTable &table);

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
  /** Parses, typechecks and turns a given Simit-formated stream into Simit IR.
    */
  int parseStream(std::istream         &programStream,
                  std::list<Function*> *functions,
                  std::list<Error>     *errors,
                  std::list<Test>      *tests);

  /** Parses, typechecks and turns a given Simit-formated string into Simit IR.
    */
  int parseString(const std::string    &programString,
                  std::list<Function*> *functions,
                  std::list<Error>     *errors,
                  std::list<Test>      *tests);

  /** Parses, typechecks and turns a given Simit-formated file into Simit IR. */
  int parseFile(const std::string    &filename,
                std::list<Function*> *functions,
                std::list<Error>     *errors,
                std::list<Test>      *tests);

 private:

  // TODO: The symbol table should be a part of the program, not the frontend
  SymbolTable symtable;
};

}} // namespace simit::internal

#endif

