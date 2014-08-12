#ifndef SIMIT_SYMBOLTABLE_H
#define SIMIT_SYMBOLTABLE_H

#include <string>
#include <map>
#include <list>

#include "util.h"

namespace simit {
namespace internal {

/** Tracks symbols and their associated IR nodes across a stack of scopes. */
template <typename Value>
class SymbolTable {
 public:
  typedef std::map<std::string, Value> SymbolMap;
  typedef typename std::list<SymbolMap>::const_iterator ScopeIterator;

  SymbolTable()  {   scope(); }
  ~SymbolTable() { unscope(); }

  /** Add a new level of symbol scoping. */
  void scope()   { scopes.push_front(SymbolMap()); }

  /** Remove the top symbol scope. */
  void unscope() { scopes.pop_front(); }

  /** Collapse all scopes into the top scope. */
  void collapse();

  /** Return the first symbol first match or add the symbol to the top scope. */
  Value &operator[](const std::string &name) {
    for (auto &scope : scopes) {
      if (scope.find(name) != scope.end()) {
        return scope[name];
      }
    }
    return scopes.front()[name];
  }

  /** Iterator over symbol scopes. */
  ScopeIterator begin() const { return scopes.begin(); }

  /** Iterator over symbol scopes. */
  ScopeIterator end() const { return scopes.end(); }

  /** Print symbol table to stream. */
  void print(std::ostream &os) const {
    os << "SymbolTable:\n";
    for (auto scope : scopes) {
      for (auto symPair : scope) {
        std::string symString = (symPair.second == NULL)
                                ? "NULL"
                                : simit::util::toString(symPair.second);
        os << simit::util::indent(symPair.first + ":" + symString + ", ", 1);
      }
      os << "\n";
    }
  }

 private:
  std::list<SymbolMap> scopes;
};

}};

#endif
