#ifndef SIMIT_SYMBOLTABLE_H
#define SIMIT_SYMBOLTABLE_H

#include <cassert>
#include <string>
#include <map>
#include <list>

#include "util.h"

namespace simit {
namespace internal {

/// Tracks symbols and their associated IR nodes across a stack of scopes.
template <typename Value>
class SymbolTable {
 public:
  typedef std::map<std::string, Value> SymbolMap;
  typedef typename std::list<SymbolMap>::const_iterator ScopeIterator;

  SymbolTable()  {   scope(); }
  ~SymbolTable() { unscope(); }

  /// Add a new level of symbol scoping.
  void scope()   { scopes.push_front(SymbolMap()); }

  /// Remove the top symbol scope.
  void unscope() { scopes.pop_front(); }

  /// Return the first match
  void insert(const std::string &symbol, const Value &value) {
    scopes.front()[symbol] = value;
  }

  // True iff the symbol table contains the given symbol.
  bool contains(const std::string &symbol) {
    for (auto &scope : scopes) {
      if (scope.find(symbol) != scope.end()) {
        return true;
      }
    }
    return false;
  }

  /// Return the first match.  It is an error to call get if the symbol is not
  /// in the symbol table.
  Value &get(const std::string &symbol) {
    for (auto &scope : scopes) {
      if (scope.find(symbol) != scope.end()) {
        return scope[symbol];
      }
    }
    assert(false && "Attempting to get a symbol that is not in symbol table.");
  }

  /// Iterator over symbol scopes.
  ScopeIterator begin() const { return scopes.begin(); }

  /// Iterator over symbol scopes.
  ScopeIterator end() const { return scopes.end(); }

  /// Print symbol table to stream.
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

template <typename T>
std::ostream &operator<<(std::ostream &os, const SymbolTable<T> &symtable) {
  symtable.print(os);
  return os;
}


}};

#endif
