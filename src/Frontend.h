#ifndef SIMIT_FRONTEND_H
#define SIMIT_FRONTEND_H

#include <list>
#include <map>
#include <string>

#include <iostream>
using namespace std;

namespace simit {

class Program;
class IRNode;

class SymbolTable {
 public:
  typedef std::map<std::string, IRNode*> SymbolMap;

  SymbolTable()  {   scope(); }
  ~SymbolTable() { unscope(); }

  /** Add a new level of symbol scoping. */
  void scope()   { scopes.push_front(SymbolMap()); }

  /** Remove the top symbol scope. */
  void unscope() { scopes.pop_front(); }

  /** Adds a symbol with the name of the irNode pointing at the irNode */
  void addNode(IRNode *irNode);

  /** Return the first symbol first match or add the symbol to the top scope. */
  IRNode *&operator[](const std::string &name);

 private:
  std::list<SymbolMap> scopes;
};


class Frontend {
 public:
  Frontend();
  ~Frontend();

  int parseString(std::string programString, Program *program);
  int parseFile(std::string filename, Program *program);

 private:
  SymbolTable symbolTable;
};

}

#endif
