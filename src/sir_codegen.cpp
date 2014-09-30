#include "sir_codegen.h"

#include <iostream>

#include "ir.h"
#include "sir.h"
#include "util.h"

using namespace std;

namespace simit {
namespace ir {

SetIRCodeGen::SetIRCodeGen() {

}

SetIRCodeGen::~SetIRCodeGen() {

}

std::unique_ptr<Stmt> SetIRCodeGen::codegen(simit::ir::Function *function){
  blockStack.push(new Block());

  visit(function);

  assert(blockStack.size() == 1);
  std::unique_ptr<Stmt> stmt(new Stmt(blockStack.top()));
  blockStack.pop();
  return stmt;
}

void SetIRCodeGen::handle(ir::Function *f) {

}

typedef std::vector<std::shared_ptr<IndexVar>> Domain;
typedef std::map<const IndexVar *, std::shared_ptr<Expr>> IndexVarMap;

static Stmt indexExprCodegen(const IndexExpr *indexExpr,
                             const Domain &domain,
                             IndexVarMap &indexMap,
                             unsigned currentIndexVar=0,
                             unsigned currentNesting=0) {
  std::unique_ptr<Block> body(new Block());

  const std::shared_ptr<IndexVar> &indexVar = domain[currentIndexVar];
  IndexSet loopDomain = indexVar->getDomain().getFactors()[currentNesting];

  if (currentIndexVar < domain.size()-1) {  // Recursive case
    cout << "hey" << endl;
  }
  else {
    switch (indexExpr->getOperator()) {
      case IndexExpr::NONE:
        break;
      case IndexExpr::NEG:
        break;
      case IndexExpr::ADD:
        break;
      case IndexExpr::SUB:
        break;
      case IndexExpr::MUL:
        break;
      case IndexExpr::DIV:
        break;
    }
  }

  return Stmt(new Foreach(indexVar->getName(), loopDomain,
                          Stmt(body.release())));
}

void SetIRCodeGen::handle(ir::IndexExpr *t) {
  Domain domain = t->getDomain();
  IndexVarMap indexMap;
  blockStack.top()->stmts.push_back(indexExprCodegen(t, domain, indexMap));
}

}} // namespace simit::ir
