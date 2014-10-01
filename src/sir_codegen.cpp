#include "sir_codegen.h"

#include <iostream>

#include "ir.h"
#include "sir.h"
#include "scopedmap.h"
//#include "util.h"

#include "sir_printer.h"

using namespace std;

namespace simit {
namespace ir {

typedef internal::ScopedMap<const IRNode*, Expr> SymbolTable;

SetIRCodeGen::SetIRCodeGen()
: symtable(new SymbolTable), scopeStack(new std::stack<Stmt>) {
}

SetIRCodeGen::~SetIRCodeGen() {
}

std::unique_ptr<Stmt> SetIRCodeGen::codegen(simit::ir::Function *function){
  scopeStack->push(Stmt());

  visit(function);

  assert(scopeStack->size() == 1);
  std::unique_ptr<Stmt> stmt(new Stmt(scopeStack->top()));
  scopeStack->pop();
  return stmt;
}

void SetIRCodeGen::handle(ir::Function *f) {
  // TODO add all the arguments to the symbol table as Variables
  for (auto &argument : f->getArguments()) {
    symtable->insert(argument.get(), Variable::make(argument->getName()));
  }
  for (auto &result : f->getResults()) {
    symtable->insert(result.get(), Variable::make(result->getName()));
  }
}

typedef std::vector<std::shared_ptr<IndexVar>> Domain;
typedef std::map<const IndexVar *, Expr> IndexMap;

static Expr emitLoad(const IndexedTensor &indexedTensor,
                     SymbolTable *symtable, IndexMap &indexMap){
  Expr index;
  TensorType *type = tensorTypePtr(indexedTensor.getTensor()->getType());
  switch (type->getOrder()) {
    case 0:
      index = IntLiteral::make(0);
      break;
    case 1:
      index = indexMap[indexedTensor.getIndexVariables()[0].get()];
      break;
    default:
      assert(false && "Matrix and higher order tensor loads not supported");
  }

  Expr target = symtable->get(indexedTensor.getTensor().get());
  return Load::make(target, index);
}

static Stmt emitStore(const IndexExpr *indexExpr, const Expr &val,
                      SymbolTable *symtable, IndexMap &indexMap){
  Expr target = Variable::make(indexExpr->getName());
  symtable->insert(indexExpr, target);

  TensorType *type = tensorTypePtr(indexExpr->getType());
  switch (type->getOrder()) {
    case 0: {
      Expr index = IntLiteral::make(0);
      return Store::make(target, index, val);
    }
    case 1: {
      Expr index = indexMap[indexExpr->getIndexVariables()[0].get()];
      return Store::make(target, index, val);
    }
    case 2:
      NOT_SUPPORTED_YET;
      break;
    default:
      assert(false && "Higher order tensor stores than matrices not supported");
  }
}

static Stmt emitIndexComputation(const IndexExpr *indexExpr,
                                 SymbolTable *symtable, IndexMap &indexMap) {
  switch (indexExpr->getOperator()) {
    case IndexExpr::NONE: {
      assert(indexExpr->getOperands().size() == 1);
      Expr val = emitLoad(indexExpr->getOperands()[0], symtable, indexMap);
      return emitStore(indexExpr, val, symtable, indexMap);
    }
    case IndexExpr::NEG: {
      Expr a = emitLoad(indexExpr->getOperands()[0], symtable, indexMap);
      Expr val = Neg::make(a);
      return emitStore(indexExpr, val, symtable, indexMap);
    }
    case IndexExpr::ADD: {
      Expr a = emitLoad(indexExpr->getOperands()[0], symtable, indexMap);
      Expr b = emitLoad(indexExpr->getOperands()[1], symtable, indexMap);
      Expr val = Add::make(a, b);;
      return emitStore(indexExpr, val, symtable, indexMap);
    }
    case IndexExpr::SUB: {
      Expr a = emitLoad(indexExpr->getOperands()[0], symtable, indexMap);
      Expr b = emitLoad(indexExpr->getOperands()[1], symtable, indexMap);
      Expr val = Sub::make(a, b);;
      return emitStore(indexExpr, val, symtable, indexMap);
    }
    case IndexExpr::MUL: {
      Expr a = emitLoad(indexExpr->getOperands()[0], symtable, indexMap);
      Expr b = emitLoad(indexExpr->getOperands()[1], symtable, indexMap);
      Expr val = Mul::make(a, b);;
      return emitStore(indexExpr, val, symtable, indexMap);
    }
    case IndexExpr::DIV: {
      Expr a = emitLoad(indexExpr->getOperands()[0], symtable, indexMap);
      Expr b = emitLoad(indexExpr->getOperands()[1], symtable, indexMap);
      Expr val = Div::make(a, b);;
      return emitStore(indexExpr, val, symtable, indexMap);
    }
  }
}

/// Lowers a Tensor IR Index Expression into a Set IR statement.
static Stmt emitIndexExpr(const IndexExpr *indexExpr,
                          SymbolTable *symtable,
                          IndexMap &indexMap,
                          unsigned currentIndexVar=0,
                          unsigned currentNesting=0) {
  auto domain = indexExpr->getDomain();
  auto indexVar = domain[currentIndexVar];
  auto loopDomain = indexVar->getDomain().getFactors()[currentNesting];

  indexMap[indexVar.get()] = Variable::make(indexVar->getName());

  cout << "currentIndexVar: " << currentIndexVar << endl;

  Stmt body = (currentIndexVar < domain.size()-1)
              ? emitIndexExpr(indexExpr, symtable, indexMap,
                              currentIndexVar+1, currentNesting)
              : emitIndexComputation(indexExpr, symtable, indexMap);

  return Foreach::make(indexVar->getName(), loopDomain, body);
}

static void addToCurrentScope(const Stmt &stmt, std::stack<Stmt> *scopeStack) {
  Stmt block = scopeStack->top();
  block = (!block.defined()) ? stmt : Block::make(block, stmt);
  scopeStack->pop();
  scopeStack->push(block);
}

void SetIRCodeGen::handle(ir::IndexExpr *t) {
  IndexMap indexMap;
  Stmt indexExprStmt;
  if (tensorTypePtr(t->getType())->getOrder() == 0) {
    indexExprStmt = emitIndexComputation(t, symtable, indexMap);
  }
  else {
    indexExprStmt = emitIndexExpr(t, symtable, indexMap);
  }
  addToCurrentScope(indexExprStmt, scopeStack);
}

}} // namespace simit::ir
