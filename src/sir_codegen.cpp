#include "sir_codegen.h"

#include <iostream>

#include "ir.h"
#include "sir.h"
#include "scopedmap.h"

using namespace std;

namespace simit {
namespace ir {

typedef internal::ScopedMap<std::string, Expr> SymbolTable;

SetIRCodeGen::SetIRCodeGen()
: symtable(new SymbolTable), scopeStack(new std::stack<Stmt>) {
}

SetIRCodeGen::~SetIRCodeGen() {
  delete symtable;
  delete scopeStack;
}

std::unique_ptr<Stmt> SetIRCodeGen::codegen(simit::ir::Function *function){
  scopeStack->push(Stmt());

  visit(function);

  assert(scopeStack->size() == 1);

  auto stmt = scopeStack->top();
  if (!stmt.defined()) {
    stmt = Pass::make();
  }

  scopeStack->pop();
  return std::unique_ptr<Stmt>(new Stmt(stmt));
}

typedef std::vector<std::shared_ptr<IndexVar>> Domain;
typedef std::map<const IndexVar *, Expr> IndexMap;

static Expr emitLoad(const Expression *tensor, const Domain &indexVariables,
                     SymbolTable *symtable, IndexMap &indexMap){
  Expr index;
  switch (indexVariables.size()) {
    case 0:
      index = IntLiteral::make(0);
      break;
    case 1:
      index = indexMap[indexVariables[0].get()];
      break;
    default:
      assert(tensor->getType().isTensor());
      for (auto &iv : indexVariables) {
        assert(iv->getDomain().getFactors().size() == 1 &&
               "Loads from blocked tensors not currently supported");
        assert(iv->getDomain().getFactors()[0].getKind() != IndexSet::Set &&
               "Loads from set tensors not currently supported");
      }

      for (size_t i=0; i < indexVariables.size(); ++i) {
        const IndexVar *iv = indexVariables[i].get();

        Expr var = indexMap[iv];
        for (size_t j=i+1; j < indexVariables.size(); ++j) {
          const IndexVar *jv = indexVariables[j].get();
          int dimsize = jv->getDomain().getFactors()[0].getSize();
          var = Mul::make(var, IntLiteral::make(dimsize));
        }

        index = (!index.defined()) ? var : Add::make(index, var);
      }
  }

  Expr target = symtable->get(tensor->getName());
  return Load::make(target, index);
}

static Expr emitLoad(const IndexedTensor &indexedTensor,
                     SymbolTable *symtable, IndexMap &indexMap){
  return emitLoad(indexedTensor.getTensor().get(),
                  indexedTensor.getIndexVariables(), symtable, indexMap);
}

static Stmt emitStore(const Expression *tensor, const Domain &indexVariables,
                      const Expr &val,
                      SymbolTable *symtable, IndexMap &indexMap){
  Expr target = Variable::make(tensor->getName());
  symtable->insert(tensor->getName(), target);

  const TensorType *type = tensor->getType().toTensor();
  switch (type->order()) {
    case 0: {
      Expr index = IntLiteral::make(0);
      return Store::make(target, index, val);
    }
    case 1: {
      Expr index = indexMap[indexVariables[0].get()];
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
                                 IndexVar::Operator reductionOperator,
                                 SymbolTable *symtable, IndexMap &indexMap) {
  Expr val;
  switch (indexExpr->getOperator()) {
    case IndexExpr::NONE: {
      assert(indexExpr->getOperands().size() == 1);
      val = emitLoad(indexExpr->getOperands()[0], symtable, indexMap);
      break;
    }
    case IndexExpr::NEG: {
      Expr a = emitLoad(indexExpr->getOperands()[0], symtable, indexMap);
      val = Neg::make(a);
      break;
    }
    case IndexExpr::ADD: {
      Expr a = emitLoad(indexExpr->getOperands()[0], symtable, indexMap);
      Expr b = emitLoad(indexExpr->getOperands()[1], symtable, indexMap);
      val = Add::make(a, b);
      break;
    }
    case IndexExpr::SUB: {
      Expr a = emitLoad(indexExpr->getOperands()[0], symtable, indexMap);
      Expr b = emitLoad(indexExpr->getOperands()[1], symtable, indexMap);
      val = Sub::make(a, b);
      break;
    }
    case IndexExpr::MUL: {
      Expr a = emitLoad(indexExpr->getOperands()[0], symtable, indexMap);
      Expr b = emitLoad(indexExpr->getOperands()[1], symtable, indexMap);
      val = Mul::make(a, b);
      break;
    }
    case IndexExpr::DIV: {
      Expr a = emitLoad(indexExpr->getOperands()[0], symtable, indexMap);
      Expr b = emitLoad(indexExpr->getOperands()[1], symtable, indexMap);
      val = Div::make(a, b);
      break;
    }
  }
  assert(val.defined());

  switch (reductionOperator) {
    case IndexVar::FREE:  // Do nothing
      break;
    case IndexVar::SUM: {
      Expr oldVal = emitLoad(indexExpr, indexExpr->getIndexVariables(),
                             symtable, indexMap);
      val = Add::make(oldVal, val);
      break;
    }
    case IndexVar::PRODUCT:
      Expr oldVal = emitLoad(indexExpr, indexExpr->getIndexVariables(),
                             symtable, indexMap);
      val = Mul::make(oldVal, val);
    break;
  }

  return emitStore(indexExpr, indexExpr->getIndexVariables(), val,
                   symtable, indexMap);
}

/// Lowers a Tensor IR Index Expression into a Set IR statement.
static Stmt emitIndexExpr(const IndexExpr *indexExpr,
                          SymbolTable *symtable,
                          IndexMap &indexMap,
                          unsigned currentIndexVar=0,
                          unsigned currentNesting=0) {
  auto domain = indexExpr->getDomain();
  auto indexVar = domain[currentIndexVar];
  indexMap[indexVar.get()] = Variable::make(indexVar->getName());

  auto loopDomain = indexVar->getDomain();
  Stmt body = (currentIndexVar < domain.size()-1)
              ? emitIndexExpr(indexExpr, symtable, indexMap,
                              currentIndexVar+1, currentNesting)
              : emitIndexComputation(indexExpr, indexVar->getOperator(),
                                     symtable, indexMap);

  return Foreach::make(indexVar->getName(), loopDomain, body);
}

static void addToCurrentScope(const Stmt &stmt, std::stack<Stmt> *scopeStack) {
  Stmt block = scopeStack->top();
  block = (!block.defined()) ? stmt : Block::make(block, stmt);
  scopeStack->pop();
  scopeStack->push(block);
}

void SetIRCodeGen::handle(ir::Function *f) {
  for (auto &argument : f->getArguments()) {
    symtable->insert(argument->getName(), Variable::make(argument->getName()));
  }
  for (auto &result : f->getResults()) {
    symtable->insert(result->getName(), Variable::make(result->getName()));
  }
}

void SetIRCodeGen::handle(FieldRead *t) {
  symtable->insert(t->getName(), Variable::make(t->getName()));
}

void SetIRCodeGen::handle(ir::IndexExpr *t) {
  IndexMap indexMap;
  Stmt indexExprStmt;
  if (t->getDomain().size() == 0) {
    indexExprStmt = emitIndexComputation(t, IndexVar::FREE, symtable, indexMap);
  }
  else {
    indexExprStmt = emitIndexExpr(t, symtable, indexMap);
  }
  addToCurrentScope(indexExprStmt, scopeStack);
}

}} // namespace simit::ir
